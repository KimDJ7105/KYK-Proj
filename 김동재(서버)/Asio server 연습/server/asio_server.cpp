//------------------------
// 0. 뷰리스트 구현 <- 완료
// 1. 주기적으로 이동하는 NPC 생성
//    1-1. NPC class 구현 - 완료
//    1-2. NPC 화면에 그리기 - 완료
//    1-3. NPC 랜덤 움직임 - 완료
//        > 부족하다 생각되는 부분 : timer_thread에서 직접 MoveNpc 함수를 호출한다.
//        > worker_thread에게 신호를 줘서 worker_thread에서 MoveNpc를 실행하도록 하는게 좋을까?
// 2. Hello, bye 구현
//		  > npc 이동, player 이동마다 시야 리스트 내에서 겹치는지 확인한다. 
//        > 겹친다면 즉시 hello를 출력하고 3초뒤에 bye를 출력한다. - 완료
// 3. 플레이어와 겹치면 멈춰서 3초간 hello라고 하고 3초 뒤에 bye 라고 하며 다시 움직인다.
// 4. 개선점
//        > npc ai의 처리를 timer_thread에서 직접 처리하지 않고 다른 쓰레드를 통해 처리한다.
//------------------------


#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <array>
#include <mutex>
#include <chrono>
#include <queue>
#include <atomic>
#include <thread>
#include <unordered_set>
#include <concurrent_unordered_map.h>
#include <concurrent_priority_queue.h>
#include <concurrent_queue.h>

#include "protocol.h"
#include <boost/asio.hpp>
#include "include/lua.hpp"

#pragma comment(lib, "lua54.lib")

using namespace std;
using namespace chrono;
using boost::asio::ip::tcp;

atomic_int g_user_ID;

const auto X_START_POS = 4;
const auto Y_START_POS = 4;

class session;
class NPC;
class TIMER_EVENT;
class EVENT;

//리눅스에서는 tbb:concurrent_~~
concurrency::concurrent_unordered_map<int, shared_ptr<session>> players;
concurrency::concurrent_unordered_map<int, shared_ptr<NPC>> npcs;
concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
concurrency::concurrent_queue<EVENT> event_queue;

void Init_Server();
int GetNewClientID();
int API_get_x_p(lua_State* L);
int API_get_y_p(lua_State* L);
int API_get_x_n(lua_State* L);
int API_get_y_n(lua_State* L);
int API_SendMessage(lua_State* L);
void wakeupNPC(int n_id);
bool can_see_npc(int from, int to);
void MoveNpc(int npc_id);

enum EVENT_TYPE { EV_RANDOM_MOVE, EV_SAY_HELLO, EV_SAY_BYE };
struct TIMER_EVENT {
	int obj_id;
	chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event_id;
	int target_id;
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};

enum NPC_STATE {ST_IDLE, ST_HELLO};
class NPC
	: public std::enable_shared_from_this<NPC>
{
private:
	tcp::socket socket_;
	unordered_set<int> view_list;
	mutex vl;
	int last_move_time;

	void do_write(unsigned char* packet, std::size_t length)
	{
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(packet, length),
			[this, self, packet, length](boost::system::error_code ec, std::size_t bytes_transferred)
			{
				if (!ec)
				{
					if (length != bytes_transferred) {
						cout << "Incomplete Send occured on session[" << id << "]. This session should be closed.\n";
					}
					delete packet; //삭제 해줘야한다. 
				}
			});
	}

public:
	atomic_bool is_active;
	int id;
	int pos_x;
	int pos_y;
	lua_State* L;
	mutex ll;
	NPC_STATE state;

	NPC(tcp::socket s, int _id, int _x, int _y)
		:socket_(std::move(s)), id(_id), pos_x(_x), pos_y(_y)
	{
		state = ST_IDLE;
		is_active = false;
		last_move_time = 0;
	}

	void Send_Packet(void* packet)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		do_write(buff, packet_size);
	}

	void send_chat_packet(int p_id, const char* mess)
	{
		sc_packet_chat packet;
		packet.id = p_id;
		packet.size = sizeof(sc_packet_chat);
		packet.type = SC_CHAT;
		strcpy_s(packet.message, mess);

		Send_Packet(&packet);
	}
};

class session
	: public std::enable_shared_from_this<session>
{
private:
	tcp::socket socket_;
	int my_id_;
	enum { max_length = 1024 };
	unsigned char data_[max_length];
	unsigned char packet_[max_length];
	int curr_packet_size_;
	int prev_data_size_;

	bool can_see(int from, int to)
	{
		if (abs(players[from]->pos_x - players[to]->pos_x) > VIEW_RANGE) return false;
		return abs(players[from]->pos_y - players[to]->pos_y) <= VIEW_RANGE;
	}

	void Send_Packet(void *packet, unsigned id)
	{
		int packet_size = reinterpret_cast<unsigned char *>(packet)[0];
		unsigned char *buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		players[id]->do_write(buff, packet_size);
	}

	void Process_Packet(unsigned char *packet, int id)
	{
		auto P= players[id];
		int y = P->pos_y;
		int x = P->pos_x;
		switch (packet[1]) {
			case CS_UP: y--; if (y < 0) y = 0; break;
			case CS_DOWN: y++; if (y >= BOARD_HEIGHT) y = BOARD_HEIGHT - 1; break;
			case CS_LEFT: x--; if (x < 0) x = 0; break;
			case CS_RIGHT: x++; if (x >= BOARD_WIDTH) x = BOARD_WIDTH - 1; break;
			default: cout << "Invalid Packet From Client [" << id << "]\n"; system("pause"); exit(-1);
		}
		P->pos_x = x;
		P->pos_y = y;

		sc_packet_pos sp_pos; //이동 패킷 준비

		sp_pos.id = id;
		sp_pos.size = sizeof(sc_packet_pos);
		sp_pos.type = SC_POS;
		sp_pos.x = P->pos_x;
		sp_pos.y = P->pos_y;

		sc_packet_put_player sp_put; //플레이어 추가 패킷 준비
		sp_put.id = id;
		sp_put.size = sizeof(sc_packet_put_player);
		sp_put.type = SC_PUT_PLAYER;
		sp_put.x = P->pos_x;
		sp_put.y = P->pos_y;

		P->Send_Packet(&sp_pos); //본인이 이동함을 전달

		//----시야처리----
		P->vl.lock(); //이동한 플레이어의 기존 뷰리스트 저장
		unordered_set<int> old_vlist = P->view_list;
		P->vl.unlock();

		unordered_set<int> near_list; //이동한 뒤 시야 내의 오브젝트 리스트
		for (auto& [key, p] : players) {
			if (p == nullptr) continue; //로그아웃한 경우 넘김
			if (p->my_id_ == id) continue; //자기 자신은 넣지 않음
			if (can_see(id, p->my_id_)) //서로 볼 수 있는 경우 추가
				near_list.insert(p->my_id_);
		}

		for (auto& [key, npc] : npcs) {
			if (npc == nullptr) continue;
			if (can_see_npc(id, npc->id)) {
				near_list.insert(npc->id);
				if (npc->state == ST_IDLE) {
					TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_SAY_HELLO, id };
					timer_queue.push(ev);
				}
			}
		}
		//-----새로운 시야 리스트 생성 완료

		for (auto& p_id : near_list) { //near_list 유저 처리
			if (p_id < MAX_USER) {
				if (players[p_id] == nullptr) continue;
				auto& cpl = players[p_id];

				cpl->vl.lock();
				if (players[p_id]->view_list.count(id)) {
					cpl->vl.unlock();
					players[p_id]->Send_Packet(&sp_pos);
				}
				else {
					players[p_id]->view_list.insert(id);
					cpl->vl.unlock();
					players[p_id]->Send_Packet(&sp_put);

				}
				if (old_vlist.count(p_id) == 0) { //새로운 플레이어가 시야에 등장
					sc_packet_put_player sp_put_o;
					sp_put_o.id = p_id;
					sp_put_o.size = sizeof(sc_packet_put_player);
					sp_put_o.type = SC_PUT_PLAYER;
					sp_put_o.x = players[p_id]->pos_x;
					sp_put_o.y = players[p_id]->pos_y;

					P->Send_Packet(&sp_put_o);
				}
			}
			else { //near_list npc 처리
				if (npcs[p_id] == nullptr) continue;
				auto& np = npcs[p_id];
				if (!np->is_active) {
					//npc를 active 시키고 타이머에 따라 움직이게 해야함
					sc_packet_put_player put_npc; //플레이어 추가 패킷 준비
					put_npc.id = np->id;
					put_npc.size = sizeof(sc_packet_put_player);
					put_npc.type = SC_PUT_PLAYER;
					put_npc.x = np->pos_x;
					put_npc.y = np->pos_y;

					P->Send_Packet(&put_npc);
					wakeupNPC(p_id);
				}
			}
		}

		for (auto& pl : old_vlist) { //플레이어 삭제처리
			if (pl < MAX_USER) {
				if (players[pl] == nullptr) continue;
				if (pl == P->my_id_) continue;
				if (0 == near_list.count(pl)) { //내 시야에서 다른 플레이어가 사라지면 서로 삭제(서로 시야가 같은 경우)
					sc_packet_remove_player sp_re1;
					sp_re1.id = pl;
					sp_re1.size = sizeof(sc_packet_remove_player);
					sp_re1.type = SC_REMOVE_PLAYER;
					P->Send_Packet(&sp_re1);

					sc_packet_remove_player sp_re2;
					sp_re2.id = P->my_id_;
					sp_re2.size = sizeof(sc_packet_remove_player);
					sp_re2.type = SC_REMOVE_PLAYER;
					players[pl]->Send_Packet(&sp_re2);
					players[pl]->vl.lock();
					players[pl]->view_list.erase(P->my_id_);
					players[pl]->vl.unlock();
				}
			}
			else {
				if (npcs[pl] == nullptr) continue;
				if (0 == near_list.count(pl)) {
					sc_packet_remove_player remove_npc;
					remove_npc.id = pl;
					remove_npc.size = sizeof(sc_packet_remove_player);
					remove_npc.type = SC_REMOVE_PLAYER;
					P->Send_Packet(&remove_npc);
				}
			}
		}

		P->vl.lock();
		P->view_list = near_list;
		P->vl.unlock();
	}

	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_),
			[this, self](boost::system::error_code ec, std::size_t length)
		{
			if (ec)
			{
				if (ec.value() == boost::asio::error::operation_aborted) return;
				cout << "Receive Error on Session[" << my_id_ << "] EC[" << ec << "]\n";
				players[my_id_] = nullptr;
				//players.unsafe_erase(my_id_);
				return;
			}

			int data_to_process = static_cast<int>(length);
			unsigned char * buf = data_;
			while (0 < data_to_process) {
				if (0 == curr_packet_size_) {
					curr_packet_size_ = buf[0];
					if (buf[0] > 200) {
						cout << "Invalid Packet Size [" << buf[0] << "] Terminating Server!\n";
						exit(-1);
					}
				}
				int need_to_build = curr_packet_size_ - prev_data_size_;
				if (need_to_build <= data_to_process) {
					// 패킷 조립
					memcpy(packet_ + prev_data_size_, buf, need_to_build);
					Process_Packet(packet_, my_id_);
					curr_packet_size_ = 0;
					prev_data_size_ = 0;
					data_to_process -= need_to_build;
					buf += need_to_build;
				}
				else {
					// 훗날을 기약
					memcpy(packet_ + prev_data_size_, buf, data_to_process);
					prev_data_size_ += data_to_process;
					data_to_process = 0;
					buf += data_to_process;
				}
			}
			do_read();
		});
	}

	void do_write(unsigned char *packet, std::size_t length)
	{
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(packet, length),
			[this, self, packet, length](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				if (length != bytes_transferred) {
					cout << "Incomplete Send occured on session[" << my_id_ << "]. This session should be closed.\n";
				}
				delete packet; //삭제 해줘야한다. 
			}
		});
	}

public:
	int pos_x;
	int pos_y;

	unordered_set<int> view_list;
	mutex vl;

	session(tcp::socket socket, int new_id)
		: socket_(std::move(socket)), my_id_(new_id)
	{
		pos_x = 0;
		pos_y = 0;
		curr_packet_size_ = 0;
		prev_data_size_ = 0;
	}

	void start()
	{
		do_read();

		sc_packet_login_info pl;
		pl.id = my_id_;
		pl.size = sizeof(sc_packet_login_info);
		pl.type = SC_LOGIN_INFO;
		pl.x = pos_x;
		pl.y = pos_y;
		Send_Packet(&pl);

		sc_packet_put_player p;
		p.id = my_id_;
		p.size = sizeof(sc_packet_put_player);
		p.type = SC_PUT_PLAYER;
		p.x = pos_x;
		p.y = pos_y;

		// 나의 접속을 모든 플레이어에게 알림
		for (auto& pl : players) {
			if (pl.second == nullptr) continue;
			if (can_see(pl.second->my_id_, my_id_)) {
				pl.second->Send_Packet(&p);
				pl.second->vl.lock();
				pl.second->view_list.insert(my_id_);
				pl.second->vl.unlock();
			}
		}
		// 나에게 접속해 있는 다른 플레이어 정보를 전송
		// 나에게 주위에 있는 NPC의 정보를 전송
		for (auto &pl : players) {
			if (pl.second == nullptr) continue;
			if (pl.second->my_id_ == my_id_) continue;
			if (can_see(pl.second->my_id_, my_id_)) {
				p.id = pl.second->my_id_;
				p.x = pl.second->pos_x;
				p.y = pl.second->pos_y;
				Send_Packet(&p);
				view_list.insert(pl.second->my_id_);
			}
		}
	}

	void Send_Packet(void* packet)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		do_write(buff, packet_size);
	}
};

class server
{
private:
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	void do_accept()
	{
		acceptor_.async_accept(socket_,
			[this](boost::system::error_code ec)
		{
				if (!ec)
				{
					int p_id = GetNewClientID();
					players[p_id] = std::make_shared<session>(std::move(socket_), p_id);
					players[p_id]->start();
					do_accept();
				}
		});
	}

	void InitializeNPC()
	{
		std::cout << "NPC initialize begin.\n";
		for (int i = 0; i < NUM_OF_NPC; ++i) {
			int x = rand() % BOARD_WIDTH;
			int y = rand() % BOARD_HEIGHT;
			int id = i + MAX_USER;
			npcs[id] = std::make_shared<NPC>(std::move(socket_), id, x, y);
			
			auto L = npcs[id]->L = luaL_newstate();

			luaL_openlibs(L);
			luaL_loadfile(L, "npc.lua");
			lua_pcall(L, 0, 0, 0);

			lua_getglobal(L, "set_uid");
			lua_pushnumber(L, id);
			lua_pcall(L, 1, 0, 0);
			// lua_pop(L, 1);// eliminate set_uid from stack after call

			lua_register(L, "API_SendMessage", API_SendMessage);
			lua_register(L, "API_get_x_p", API_get_x_p);
			lua_register(L, "API_get_y_p", API_get_y_p);
			lua_register(L, "API_get_x_n", API_get_x_n);
			lua_register(L, "API_get_y_n", API_get_y_n);
		}
		std::cout << "Done\n";
	}

public:
	server(boost::asio::io_context& io_service, int port)
		: acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
		socket_(io_service)
	{
		do_accept();
		InitializeNPC();
	}
};

enum EVENT_CASE {EC_RANDOM_MOVE, EC_SAY_HELLO,EC_SAY_BYE};
struct EVENT
{
	int obj_id;
	EVENT_CASE event_id;
	int target_id;
};

void worker_thread(boost::asio::io_context *service)
{
	service->run();
}

void timer_thread()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == timer_queue.try_pop(ev)) {
			if (ev.wakeup_time > current_time) {
				timer_queue.push(ev);		// 최적화 필요
				// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(1ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.event_id) {
			case EV_RANDOM_MOVE:
				if (npcs[ev.obj_id]->state != ST_IDLE) break;
				/*EVENT move_event;
				move_event.event_id = EC_RANDOM_MOVE;
				move_event.obj_id = ev.obj_id;
				move_event.target_id = ev.target_id;

				event_queue.push(move_event);*/

				MoveNpc(ev.obj_id);
				break;
			case EV_SAY_BYE:
				/*EVENT bye_event;
				bye_event.event_id = EC_SAY_BYE;
				bye_event.obj_id = ev.obj_id;
				bye_event.target_id = ev.target_id;

				event_queue.push(bye_event);*/
				npcs[ev.obj_id]->state = ST_IDLE;
				sc_packet_chat packet;
				packet.id = ev.obj_id;
				packet.size = sizeof(sc_packet_chat);
				packet.type = SC_CHAT;
				strcpy_s(packet.message, "BYE");

				players[ev.target_id]->Send_Packet(&packet);

				MoveNpc(ev.obj_id);
				break;
			case EV_SAY_HELLO :
				/*EVENT hello_event;
				hello_event.event_id = EC_SAY_HELLO;
				hello_event.obj_id = ev.obj_id;
				hello_event.target_id = ev.target_id;

				event_queue.push(hello_event);*/
				auto L = npcs[ev.obj_id]->L;
				lua_getglobal(L, "event_player_move");
				lua_pushnumber(L, ev.target_id);
				lua_pcall(L, 1, 0, 0);
				lua_pop(L, 1);
				break;
			}
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}
}

int main()
{
	boost::asio::io_context io_service;
	vector <thread > worker_threads;
	server s(io_service, MY_SERVER_PORT);

	thread timer( timer_thread );

	Init_Server();

	for (auto i = 0; i < 4; i++) worker_threads.emplace_back(worker_thread, &io_service);
	for (auto &th : worker_threads) th.join();
	timer.join();
}

void Init_Server()
{
	_wsetlocale(LC_ALL, L"korean");
}

int GetNewClientID()
{
	if (g_user_ID >= MAX_USER) {
		cout << "MAX USER FULL\n";
		exit(-1);
	}
	return g_user_ID++;
}

int API_get_x_p(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = players[user_id]->pos_x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y_p(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = players[user_id]->pos_y;
	lua_pushnumber(L, y);
	return 1;
}

int API_get_x_n(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);

	int x = npcs[user_id]->pos_x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y_n(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = npcs[user_id]->pos_y;
	lua_pushnumber(L, y);
	return 1;
}

int API_SendMessage(lua_State* L)
{
	int my_id = (int)lua_tointeger(L, -3);
	int user_id = (int)lua_tointeger(L, -2);
	char* mess = (char*)lua_tostring(L, -1);

	lua_pop(L, 4);

	sc_packet_chat packet;
	packet.id = my_id;
	packet.size = sizeof(sc_packet_chat);
	packet.type = SC_CHAT;
	strcpy_s(packet.message, mess);

	players[user_id]->Send_Packet(&packet);
	
	npcs[my_id]->state = ST_HELLO;

	TIMER_EVENT ev{ my_id, chrono::system_clock::now() + 3s, EV_SAY_BYE, user_id };
	timer_queue.push(ev);

	return 0;
}

void wakeupNPC(int n_id)
{
	if (n_id < MAX_USER) return;
	if (npcs[n_id]->is_active) return;
	bool old_state = false;

	if (false == atomic_compare_exchange_strong(&npcs[n_id]->is_active, &old_state, true))
		return;
	//타이머 이벤트 생성하고 우선순위 큐에 넣기
	TIMER_EVENT ev{ n_id, chrono::system_clock::now(), EV_RANDOM_MOVE, 0 };
	timer_queue.push(ev);
}

bool can_see_npc(int from, int to)
{
	if (abs(players[from]->pos_x - npcs[to]->pos_x) > VIEW_RANGE) return false;
	return abs(players[from]->pos_y - npcs[to]->pos_y) <= VIEW_RANGE;
}

void MoveNpc(int npc_id)
{

	unordered_set<int> old_vl;
	for (auto& [key, p] : players) {
		if (p == nullptr) continue;
		if (can_see_npc(key, npc_id))
			old_vl.insert(key);
	}

	int x = npcs[npc_id]->pos_x;
	int y = npcs[npc_id]->pos_y;
	switch (rand() % 4) {
	case 0: if (x < (BOARD_WIDTH - 1)) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < (BOARD_HEIGHT - 1)) y++; break;
	case 3:if (y > 0) y--; break;
	}
	npcs[npc_id]->pos_x = x;
	npcs[npc_id]->pos_y = y;

	unordered_set<int> new_vl;
	for (auto& [key, p] : players) {
		if (p == nullptr) continue;
		if (can_see_npc(key, npc_id)) {
			new_vl.insert(key);
			TIMER_EVENT ev{ npc_id, chrono::system_clock::now() + 1s, EV_SAY_HELLO, key };
			timer_queue.push(ev);
		}
	}

	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			// 플레이어의 시야에 등장

			sc_packet_put_player npc_put; //플레이어 추가 패킷 준비
			npc_put.id = npc_id;
			npc_put.size = sizeof(sc_packet_put_player);
			npc_put.type = SC_PUT_PLAYER;
			npc_put.x = npcs[npc_id]->pos_x;
			npc_put.y = npcs[npc_id]->pos_y;
			players[pl]->Send_Packet(&npc_put);
		}
		else {
			// 플레이어가 계속 보고 있음.
			sc_packet_pos npc_pos; //이동 패킷 준비

			npc_pos.id = npc_id;
			npc_pos.size = sizeof(sc_packet_pos);
			npc_pos.type = SC_POS;
			npc_pos.x = npcs[npc_id]->pos_x;
			npc_pos.y = npcs[npc_id]->pos_y;

			players[pl]->Send_Packet(&npc_pos);
		}
	}
	//삭제
	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			players[pl]->vl.lock();
			if (0 != players[pl]->view_list.count(npc_id)) {
				players[pl]->vl.unlock();
				sc_packet_remove_player npc_remove;

				npc_remove.id = npc_id;
				npc_remove.size = sizeof(sc_packet_remove_player);
				npc_remove.type = SC_REMOVE_PLAYER;

				players[pl]->Send_Packet(&npc_remove);
			}
			else {
				players[pl]->vl.unlock();
			}
		}

	}

	if (new_vl.size() == 0) {
		npcs[npc_id]->is_active = false;
	}

	else {
		TIMER_EVENT ev{ npc_id, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
		timer_queue.push(ev);
	}
}