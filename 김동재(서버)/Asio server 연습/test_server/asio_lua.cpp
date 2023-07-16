//--------------------
// 1. wakeup npc와 workertherd 수정
// 2. 특히 workerthread는 완전히 갈아엎어야함
// 3. asio의 동작 구조 더 공부하기
//--------------------

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <concurrent_priority_queue.h>

#include "protocol.h"
#include "include/lua.hpp"

#pragma comment(lib, "lua54.lib")

using boost::asio::ip::tcp;

constexpr int VIEW_RANGE = 5;

enum EVENT_TYPE { EV_RANDOM_MOVE };

struct TIMER_EVENT {
    int obj_id;
    std::chrono::system_clock::time_point wakeup_time;
    EVENT_TYPE event_id;    int target_id;
    constexpr bool operator<(const TIMER_EVENT& L) const {
        return (wakeup_time > L.wakeup_time);
    }
};

concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_NPC_MOVE, OP_AI_HELLO };

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };

class SESSION {
public:
    std::mutex _s_lock;
    S_STATE _state;
    std::atomic_bool _is_active;
    int _id;
    tcp::socket _socket;
    short x, y;
    char _name[NAME_SIZE];
    int _prev_remain;
    std::unordered_set<int> _view_list;
    std::mutex _vl;
    int last_move_time;
    lua_State* _L;
    std::mutex _ll;
    char recv_data[BUF_SIZE];

public:
    SESSION() : _socket(nullptr) {
        std::cout << "Session Creation Error.\n";
    }

    SESSION(tcp::socket socket, int id) : _is_active(false), _socket(std::move(socket)) {
        _id = id;
        x = y = 0;
        _name[0] = 0;
        _state = ST_FREE;
        _prev_remain = 0;
    }

    ~SESSION() {}

    void do_recv() {
        _socket.async_read_some(boost::asio::buffer(recv_data, BUF_SIZE),
            [&](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (!ec) {
                    int remain_data = bytes_transferred + _prev_remain;
                    char* p = recv_data;
                    while (remain_data > 0) {
                        int packet_size = p[0];
                        if (packet_size <= remain_data) {
                            process_packet(_id, p);
                            p = p + packet_size;
                            remain_data = remain_data - packet_size;
                        }
                        else {
                            break;
                        }
                    }
                    _prev_remain = remain_data;
                    if (remain_data > 0) {
                        memcpy(recv_data, p, remain_data);
                    }
                    do_recv();
                }
                else {
                    disconnect(_id);
                }
            });
    }

    void do_send(void* packet) {
        char* data = reinterpret_cast<char*>(packet);
        boost::asio::async_write(_socket,boost::asio::buffer(data,data[0]),
            [&](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (ec) {
                    disconnect(_id);
                }
            });
    }

    void send_login_info_packet() {
        SC_LOGIN_INFO_PACKET p;
        p.id = _id;
        p.size = sizeof(SC_LOGIN_INFO_PACKET);
        p.type = SC_LOGIN_INFO;
        p.x = x;
        p.y = y;
        do_send(&p);
    }

    void send_move_packet(int c_id);
    void send_add_player_packet(int c_id);
    void send_chat_packet(int c_id, const char* mess);
    void send_remove_player_packet(int c_id) {
        std::lock_guard<std::mutex> ll(_vl);
        if (_view_list.count(c_id)) {
            _view_list.erase(c_id);
        }
        else {
            return;
        }
        SC_REMOVE_OBJECT_PACKET p;
        p.id = c_id;
        p.size = sizeof(p);
        p.type = SC_REMOVE_OBJECT;
        do_send(&p);
    }
};

std::unordered_map<int, SESSION> clients;

bool is_pc(int object_id) {
    return object_id < MAX_USER;
}

bool is_npc(int object_id) {
    return !is_pc(object_id);
}

bool can_see(int from, int to) {
    if (std::abs(clients[from].x - clients[to].x) > VIEW_RANGE) return false;
    return std::abs(clients[from].y - clients[to].y) <= VIEW_RANGE;
}

void SESSION::send_move_packet(int c_id) {
    SC_MOVE_OBJECT_PACKET p;
    p.id = c_id;
    p.size = sizeof(SC_MOVE_OBJECT_PACKET);
    p.type = SC_MOVE_OBJECT;
    p.x = clients[c_id].x;
    p.y = clients[c_id].y;
    p.move_time = clients[c_id].last_move_time;
    do_send(&p);
}

void SESSION::send_add_player_packet(int c_id) {
    SC_ADD_OBJECT_PACKET add_packet;
    add_packet.id = c_id;
    strcpy_s(add_packet.name, clients[c_id]._name);
    add_packet.size = sizeof(add_packet);
    add_packet.type = SC_ADD_OBJECT;
    add_packet.x = clients[c_id].x;
    add_packet.y = clients[c_id].y;
    _vl.lock();
    _view_list.insert(c_id);
    _vl.unlock();
    do_send(&add_packet);
}

void SESSION::send_chat_packet(int p_id, const char* mess) {
    SC_CHAT_PACKET packet;
    packet.id = p_id;
    packet.size = sizeof(packet);
    packet.type = SC_CHAT;
    strcpy_s(packet.mess, mess);
    do_send(&packet);
}

int get_new_client_id() {
    for (int i = 0; i < MAX_USER; ++i) {
        std::lock_guard<std::mutex> ll{ clients[i]._s_lock };
        if (clients[i]._state == ST_FREE)
            return i;
    }
    return -1;
}

void WakeUpNPC(int npc_id, int waker) {
    OVER_EXP* exover = new OVER_EXP;
    exover->_comp_type = OP_AI_HELLO;
    exover->_ai_target_obj = waker;
    PostQueuedCompletionStatus(h_iocp, 1, npc_id, &exover->_over);

    if (clients[npc_id]._is_active)
        return;
    bool old_state = false;
    if (!clients[npc_id]._is_active.compare_exchange_strong(old_state, true))
        return;
    TIMER_EVENT ev{ npc_id, std::chrono::system_clock::now(), EV_RANDOM_MOVE, 0 };
    timer_queue.push(ev);
}

void process_packet(int c_id, char* packet) {
    switch (packet[1]) {
    case CS_LOGIN: {
        CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
        strcpy_s(clients[c_id]._name, p->name);
        {
            std::lock_guard<std::mutex> ll{ clients[c_id]._s_lock };
            clients[c_id].x = rand() % W_WIDTH;
            clients[c_id].y = rand() % W_HEIGHT;
            clients[c_id]._state = ST_INGAME;
        }
        clients[c_id].send_login_info_packet();
        for (auto& pl : clients) {
            {
                std::lock_guard<std::mutex> ll(pl.second._s_lock);
                if (pl.second._state != ST_INGAME)
                    continue;
            }
            if (pl.second._id == c_id)
                continue;
            if (!can_see(c_id, pl.second._id))
                continue;
            if (is_pc(pl.second._id))
                pl.second.send_add_player_packet(c_id);
            else
                WakeUpNPC(pl.second._id, c_id);
            clients[c_id].send_add_player_packet(pl.second._id);
        }
        break;
    }
    case CS_MOVE: {
        CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
        clients[c_id].last_move_time = p->move_time;
        short x = clients[c_id].x;
        short y = clients[c_id].y;
        switch (p->direction) {
        case 0:
            if (y > 0)
                y--;
            break;
        case 1:
            if (y < W_HEIGHT - 1)
                y++;
            break;
        case 2:
            if (x > 0)
                x--;
            break;
        case 3:
            if (x < W_WIDTH - 1)
                x++;
            break;
        }
        clients[c_id].x = x;
        clients[c_id].y = y;

        std::unordered_set<int> near_list;
        clients[c_id]._vl.lock();
        std::unordered_set<int> old_vlist = clients[c_id]._view_list;
        clients[c_id]._vl.unlock();
        for (auto& cl : clients) {
            if (cl.second._state != ST_INGAME)
                continue;
            if (cl.second._id == c_id)
                continue;
            if (can_see(c_id, cl.second._id))
                near_list.insert(cl.second._id);
        }

        clients[c_id].send_move_packet(c_id);

        for (auto& pl : near_list) {
            auto& cpl = clients[pl];
            if (is_pc(pl)) {
                cpl._vl.lock();
                if (clients[pl]._view_list.count(c_id)) {
                    cpl._vl.unlock();
                    clients[pl].send_move_packet(c_id);
                }
                else {
                    cpl._vl.unlock();
                    clients[pl].send_add_player_packet(c_id);
                }
            }
            else {
                WakeUpNPC(pl, c_id);
            }

            if (old_vlist.count(pl) == 0)
                clients[c_id].send_add_player_packet(pl);
        }

        for (auto& pl : old_vlist) {
            if (near_list.count(pl) == 0) {
                clients[c_id].send_remove_player_packet(pl);
                if (is_pc(pl))
                    clients[pl].send_remove_player_packet(c_id);
            }
        }
        break;
    }
    }
}

void disconnect(int c_id) {
    clients[c_id]._vl.lock();
    std::unordered_set<int> vl = clients[c_id]._view_list;
    clients[c_id]._vl.unlock();
    for (auto& p_id : vl) {
        if (is_npc(p_id))
            continue;
        auto& pl = clients[p_id];
        {
            std::lock_guard<std::mutex> ll(pl._s_lock);
            if (pl._state != ST_INGAME)
                continue;
        }
        if (pl._id == c_id)
            continue;
        pl.send_remove_player_packet(c_id);
    }

    std::lock_guard<std::mutex> ll(clients[c_id]._s_lock);
    clients[c_id]._state = ST_FREE;
}

void do_npc_random_move(int npc_id) {
    SESSION& npc = clients[npc_id];
    std::unordered_set<int> old_vl;
    for (auto& obj : clients) {
        if (obj.second._state != ST_INGAME)
            continue;
        if (is_npc(obj.second._id))
            continue;
        if (can_see(npc._id, obj.second._id))
            old_vl.insert(obj.second._id);
    }

    int x = npc.x;
    int y = npc.y;
    switch (rand() % 4) {
    case 0:
        if (x < (W_WIDTH - 1))
            x++;
        break;
    case 1:
        if (x > 0)
            x--;
        break;
    case 2:
        if (y < (W_HEIGHT - 1))
            y++;
        break;
    case 3:
        if (y > 0)
            y--;
        break;
    }
    npc.x = x;
    npc.y = y;

    std::unordered_set<int> new_vl;
    for (auto& obj : clients) {
        if (obj.second._state != ST_INGAME)
            continue;
        if (is_npc(obj.second._id))
            continue;
        if (can_see(npc._id, obj.second._id))
            new_vl.insert(obj.second._id);
    }

    for (auto pl : new_vl) {
        if (old_vl.count(pl) == 0) {
            clients[pl]._vl.lock();
            if (clients[pl]._view_list.count(npc._id)) {
                clients[pl]._vl.unlock();
                clients[pl].send_remove_player_packet(npc._id);
            }
            else {
                clients[pl]._vl.unlock();
            }
        }
    }

    for (auto pl : old_vl) {
        if (new_vl.count(pl) == 0) {
            clients[pl]._vl.lock();
            if (clients[pl]._view_list.count(npc._id)) {
                clients[pl]._vl.unlock();
                clients[pl].send_remove_player_packet(npc._id);
            }
            else {
                clients[pl]._vl.unlock();
            }
        }
    }
}

void worker_thread(HANDLE h_iocp) {
    while (true) {
        DWORD num_bytes;
        ULONG_PTR key;
        WSAOVERLAPPED* over = nullptr;
        BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
        OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
        if (!ret) {
            if (ex_over->_comp_type == OP_ACCEPT)
                std::cout << "Accept Error";
            else {
                std::cout << "GQCS Error on client[" << key << "]\n";
                disconnect(static_cast<int>(key));
                if (ex_over->_comp_type == OP_SEND)
                    delete ex_over;
                continue;
            }
        }

        if ((0 == num_bytes) && ((ex_over->_comp_type == OP_RECV) || (ex_over->_comp_type == OP_SEND))) {
            disconnect(static_cast<int>(key));
            if (ex_over->_comp_type == OP_SEND)
                delete ex_over;
            continue;
        }

        switch (ex_over->_comp_type) {
        case OP_ACCEPT: {
            int client_id = get_new_client_id();
            if (client_id != -1) {
                {
                    std::lock_guard<std::mutex> ll(clients[client_id]._s_lock);
                    clients[client_id]._state = ST_ALLOC;
                }
                clients[client_id].x = 0;
                clients[client_id].y = 0;
                clients[client_id]._id = client_id;
                clients[client_id]._name[0] = 0;
                clients[client_id]._prev_remain = 0;
                clients[client_id]._socket = g_c_socket;
                CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), h_iocp, client_id, 0);
                clients[client_id].do_recv();
                g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
            }
            else {
                std::cout << "Max user exceeded.\n";
            }
            ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
            int addr_size = sizeof(SOCKADDR_IN);
            AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
            break;
        }
        case OP_RECV: {
            int remain_data = num_bytes + clients[key]._prev_remain;
            char* p = ex_over->_send_buf;
            while (remain_data > 0) {
                int packet_size = p[0];
                if (packet_size <= remain_data) {
                    process_packet(static_cast<int>(key), p);
                    p = p + packet_size;
                    remain_data = remain_data - packet_size;
                }
                else {
                    break;
                }
            }
            clients[key]._prev_remain = remain_data;
            if (remain_data > 0) {
                memcpy(ex_over->_send_buf, p, remain_data);
            }
            clients[key].do_recv();
            break;
        }
        case OP_SEND:
            delete ex_over;
            break;
        case OP_NPC_MOVE: {
            bool keep_alive = false;
            for (int j = 0; j < MAX_USER; ++j) {
                if (clients[j]._state != ST_INGAME)
                    continue;
                if (can_see(static_cast<int>(key), j)) {
                    keep_alive = true;
                    break;
                }
            }
            if (keep_alive) {
                do_npc_random_move(static_cast<int>(key));
                TIMER_EVENT ev{ static_cast<int>(key), std::chrono::system_clock::now() + std::chrono::milliseconds(500), EV_RANDOM_MOVE, 0 };
                timer_queue.push(ev);
            }
            else {
                clients[static_cast<int>(key)]._is_active = false;
            }
            break;
        }
        case OP_AI_HELLO:
            clients[key].send_chat_packet(ex_over->_ai_target_obj, "Hello");
            break;
        }
    }
}

void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor);

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor my_acceptor{ io_context, tcp::endpoint(tcp::v4(), PORT_NUM) };
        my_acceptor.async_accept([&my_acceptor](boost::system::error_code ec, tcp::socket socket) { //비동기 accept
            accept_callback(ec, socket, my_acceptor); }); //비동기이기 때문에 콜백함수를 넣어줘야한다.
        //람다를 사용해서 콜백 함수에 my_acceptor를 넘겨준다. 콜백 함수 내부에서도 accept를 해야하기 때문이다.
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

void accept_callback(boost::system::error_code ec, tcp::socket& socket, tcp::acceptor& my_acceptor)
{
    int new_id = get_new_client_id();
    if (new_id != -1) {
        clients.try_emplace(new_id, std::move(socket), new_id);
    }
    
    
}