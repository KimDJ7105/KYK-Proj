//------------------------
// 0. �丮��Ʈ ���� <- �Ϸ�
// 1. �ֱ������� �̵��ϴ� NPC ����
//    1-1. NPC class ���� - �Ϸ�
//    1-2. NPC ȭ�鿡 �׸��� - �Ϸ�
//    1-3. NPC ���� ������
// 2. Hello, bye ����
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
#include <unordered_set>
#include <concurrent_unordered_map.h>

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

concurrency::concurrent_unordered_map<int, shared_ptr<session>> players;
//������������ tbb:concurrent_~~

concurrency::concurrent_unordered_map<int, shared_ptr<NPC>> npcs;

void Init_Server();
int GetNewClientID();
int API_get_x(lua_State* L);
int API_get_y(lua_State* L);
int API_SendMessage(lua_State* L);

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
					delete packet; //���� ������Ѵ�. 
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

	NPC(tcp::socket s, int _id, int _x, int _y)
		:socket_(std::move(s)), id(_id), pos_x(_x), pos_y(_y)
	{
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

	int pos_x;
	int pos_y;

	unordered_set<int> view_list;
	mutex vl;

	bool can_see(int from, int to)
	{
		if (abs(players[from]->pos_x - players[to]->pos_x) > VIEW_RANGE) return false;
		return abs(players[from]->pos_y - players[to]->pos_y) <= VIEW_RANGE;
	}
	
	bool can_see_npc(int from, int to)
	{
		if (abs(players[from]->pos_x - npcs[to - MAX_USER]->pos_x) > VIEW_RANGE) return false;
		return abs(players[from]->pos_y - npcs[to - MAX_USER]->pos_y) <= VIEW_RANGE;
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

		sc_packet_pos sp_pos; //�̵� ��Ŷ �غ�

		sp_pos.id = id;
		sp_pos.size = sizeof(sc_packet_pos);
		sp_pos.type = SC_POS;
		sp_pos.x = P->pos_x;
		sp_pos.y = P->pos_y;

		sc_packet_put_player sp_put; //�÷��̾� �߰� ��Ŷ �غ�
		sp_put.id = id;
		sp_put.size = sizeof(sc_packet_put_player);
		sp_put.type = SC_PUT_PLAYER;
		sp_put.x = P->pos_x;
		sp_put.y = P->pos_y;

		P->Send_Packet(&sp_pos); //������ �̵����� ����

		//----�þ�ó��----
		P->vl.lock(); //�̵��� �÷��̾��� ���� �丮��Ʈ ����
		unordered_set<int> old_vlist = P->view_list;
		P->vl.unlock();

		unordered_set<int> near_list; //�̵��� �� �þ� ���� ������Ʈ ����Ʈ
		for (auto& [key, p] : players) {
			if (p == nullptr) continue; //�α׾ƿ��� ��� �ѱ�
			if (p->my_id_ == id) continue; //�ڱ� �ڽ��� ���� ����
			if (can_see(id, p->my_id_)) //���� �� �� �ִ� ��� �߰�
				near_list.insert(p->my_id_);
		}

		for (auto& [key, npc] : npcs) {
			if (npc == nullptr) continue;
			if (can_see_npc(id,npc->id))
				near_list.insert(npc->id);
		}
		//-----���ο� �þ� ����Ʈ ���� �Ϸ�

		for (auto& p_id : near_list) { //near_list ���� ó��
			if (p_id >= MAX_USER) continue;
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
			if (old_vlist.count(p_id) == 0) { //���ο� �÷��̾ �þ߿� ����
				sc_packet_put_player sp_put_o;
				sp_put_o.id = p_id;
				sp_put_o.size = sizeof(sc_packet_put_player);
				sp_put_o.type = SC_PUT_PLAYER;
				sp_put_o.x = players[p_id]->pos_x;
				sp_put_o.y = players[p_id]->pos_y;
				
				P->Send_Packet(&sp_put_o);
			}
		}

		for (auto& n_id : near_list) { //near_list npc ó��
			if (n_id < MAX_USER) continue;
			auto& np = npcs[n_id - MAX_USER];
			if (!np->is_active) {
				//npc�� active ��Ű�� Ÿ�̸ӿ� ���� �����̰� �ؾ���
				sc_packet_put_player put_npc; //�÷��̾� �߰� ��Ŷ �غ�
				put_npc.id = np->id;
				put_npc.size = sizeof(sc_packet_put_player);
				put_npc.type = SC_PUT_PLAYER;
				put_npc.x = np->pos_x;
				put_npc.y = np->pos_y;

				P->Send_Packet(&put_npc);
			}
		}

		for (auto& pl : old_vlist) { //�÷��̾� ����ó��
			if (pl >= MAX_USER) continue;
			if (pl == P->my_id_) continue;
			if (0 == near_list.count(pl)) { //�� �þ߿��� �ٸ� �÷��̾ ������� ���� ����(���� �þ߰� ���� ���)
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

		for (auto& nl : old_vlist) { // NPC ���� ó��
			if (nl < MAX_USER) continue;
			if (0 == near_list.count(nl)) {
				sc_packet_remove_player remove_npc;
				remove_npc.id = nl;
				remove_npc.size = sizeof(sc_packet_remove_player);
				remove_npc.type = SC_REMOVE_PLAYER;
				P->Send_Packet(&remove_npc);
			}
		}
		P->view_list = near_list;
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
					// ��Ŷ ����
					memcpy(packet_ + prev_data_size_, buf, need_to_build);
					Process_Packet(packet_, my_id_);
					curr_packet_size_ = 0;
					prev_data_size_ = 0;
					data_to_process -= need_to_build;
					buf += need_to_build;
				}
				else {
					// �ʳ��� ���
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
				delete packet; //���� ������Ѵ�. 
			}
		});
	}

public:
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

		// ���� ������ ��� �÷��̾�� �˸�
		for (auto& pl : players) {
			if (pl.second == nullptr) continue;
			if (can_see(pl.second->my_id_, my_id_)) {
				pl.second->Send_Packet(&p);
				pl.second->vl.lock();
				pl.second->view_list.insert(my_id_);
				pl.second->vl.unlock();
			}
		}
		// ������ ������ �ִ� �ٸ� �÷��̾� ������ ����
		// ������ ������ �ִ� NPC�� ������ ����
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
			npcs[i] = std::make_shared<NPC>(std::move(socket_), id, x, y);

			auto L = npcs[i]->L = luaL_newstate();

			luaL_openlibs(L);
			luaL_loadfile(L, "npc.lua");
			lua_pcall(L, 0, 0, 0);

			lua_getglobal(L, "set_uid");
			lua_pushnumber(L, i);
			lua_pcall(L, 1, 0, 0);
			// lua_pop(L, 1);// eliminate set_uid from stack after call

			lua_register(L, "API_SendMessage", API_SendMessage);
			lua_register(L, "API_get_x", API_get_x);
			lua_register(L, "API_get_y", API_get_y);
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

void worker_thread(boost::asio::io_context *service)
{
	service->run();
}

int main()
{
	boost::asio::io_context io_service;
	vector <thread > worker_threads;
	server s(io_service, MY_SERVER_PORT);

	Init_Server();

	for (auto i = 0; i < 4; i++) worker_threads.emplace_back(worker_thread, &io_service);
	for (auto &th : worker_threads) th.join();
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

int API_get_x(lua_State* L)
{
	int user_id =
		(int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = npcs[user_id]->pos_x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
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

	npcs[user_id]->send_chat_packet(my_id, mess);
	return 0;
}