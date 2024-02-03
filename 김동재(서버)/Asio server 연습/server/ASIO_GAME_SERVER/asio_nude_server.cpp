//------------------------
//
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
#include <boost/asio/steady_timer.hpp>
#include <boost/bind.hpp>
#include "include/lua.hpp"

#pragma comment(lib, "lua54.lib")

using namespace std;
using namespace chrono;
using boost::asio::ip::tcp;

atomic_int g_user_ID;
atomic_int g_item_ID = MAX_USER + NUM_OF_NPC + 1;
atomic_int g_item_EA;

const auto X_START_POS = 4;
const auto Y_START_POS = 4;

class session;
class NPC;
class TIMER_EVENT;
class EVENT;
class Item;

//리눅스에서는 tbb:concurrent_~~
concurrency::concurrent_unordered_map<int, shared_ptr<session>> players;
concurrency::concurrent_unordered_map<int, shared_ptr<NPC>> npcs;
concurrency::concurrent_unordered_map<int, shared_ptr<Item>> items;
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
bool can_see_item(int px, int py, int ix, int iy);
void MoveNpc(int npc_id);
void event_excuter(const boost::system::error_code& ec, boost::asio::steady_timer* timer);

enum EVENT_TYPE { EV_RANDOM_MOVE, EV_SAY_HELLO, EV_SAY_BYE, EV_SPAWN_ITEM, EV_START_RASER, EV_SET_RASER, EV_SET_DANGER,EV_SET_WHITE};
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

class Item {
private:
	int id;

public:
	int type;
	int pos_x, pos_y;

	Item(int _id) {
		id = _id;
		type = rand() % 2;

		pos_x = rand() % BOARD_WIDTH;
		pos_y = rand() % BOARD_HEIGHT;

		while (col[pos_x][pos_y] == TILE_BLACK) {
			pos_x = rand() % BOARD_WIDTH;
			pos_y = rand() % BOARD_HEIGHT;
		}
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
	unsigned int move_time;

	int hp;
	int bullet;

	bool can_see(int from, int to)
	{
		shared_ptr<session> p1 = players[from];
		shared_ptr<session> p2 = players[to];
		if (nullptr == p1) return false;
		if (nullptr == p2) return false;

		if (abs(p1->pos_x - p2->pos_x) > VIEW_RANGE) return false;
		return abs(p1->pos_y - p2->pos_y) <= VIEW_RANGE;
	}

	void Send_Packet(void *packet, unsigned id)
	{
		int packet_size = reinterpret_cast<unsigned char *>(packet)[0];
		unsigned char *buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);

		shared_ptr<session> player = players[id];

		if (nullptr == player) return;

		player->do_write(buff, packet_size);
	}

	void serch_target(int shoter_id) 
	{
		shared_ptr<session> P = players[shoter_id];

		P->vl.lock();
		unordered_set<int> vlist = P->view_list;
		P->vl.unlock();

		for (auto& key : vlist) {
			if (key < MAX_USER) {
				shared_ptr<session> target = players[key];
				if (target == NULL) continue;
				switch (P->view_dir) {
				case VIEW_UP:
					if (target->pos_x == P->pos_x && target->pos_y < P->pos_y) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->my_id_;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						target->Send_Packet(&hit_p);
						target->hp -= 1;

						if (target->hp <= 0) {
							//체력이 0된 플레이어 로그 아웃 시키기
							sc_packet_game_over gameover_packet;
							gameover_packet.size = sizeof(sc_packet_game_over);
							gameover_packet.type = SC_GAME_OVER;

							target->Send_Packet(&gameover_packet);
						}

						std::cout << "Player" << P->my_id_ << " shot Player" << target->my_id_ << " remain bullet : " << P->bullet << std::endl;
						std::cout << "Player" << target->my_id_ << " remain HP : " << target->hp << std::endl;
						return;
					}
					break;
				case VIEW_DOWN:
					if (target->pos_x == P->pos_x && target->pos_y > P->pos_y) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->my_id_;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						target->Send_Packet(&hit_p);
						target->hp -= 1;

						if (target->hp <= 0) {
							//체력이 0된 플레이어 로그 아웃 시키기
							sc_packet_game_over gameover_packet;
							gameover_packet.size = sizeof(sc_packet_game_over);
							gameover_packet.type = SC_GAME_OVER;

							target->Send_Packet(&gameover_packet);
						}
						std::cout << "Player" << P->my_id_ << " shot Player" << target->my_id_ << " remain bullet : " << P->bullet << std::endl;
						std::cout << "Player" << target->my_id_ << " remain HP : " << target->hp << std::endl;
						return;
					}
					break;
				case VIEW_LEFT:
					if (target->pos_y == P->pos_y && target->pos_x < P->pos_x) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->my_id_;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						target->Send_Packet(&hit_p);
						target->hp -= 1;

						if (target->hp <= 0) {
							//체력이 0된 플레이어 로그 아웃 시키기
							sc_packet_game_over gameover_packet;
							gameover_packet.size = sizeof(sc_packet_game_over);
							gameover_packet.type = SC_GAME_OVER;

							target->Send_Packet(&gameover_packet);
						}
						std::cout << "Player" << P->my_id_ << " shot Player" << target->my_id_ << " remain bullet : " << P->bullet << std::endl;
						std::cout << "Player" << target->my_id_ << " remain HP : " << target->hp << std::endl;
						return;
					}
					break;
				case VIEW_RIGHT:
					if (target->pos_y == P->pos_y && target->pos_x > P->pos_x) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->my_id_;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						target->Send_Packet(&hit_p);
						ChangeHP(target->my_id_, 1);
						
						std::cout << "Player" << P->my_id_ << " shot Player" << target->my_id_ << " remain bullet : " << P->bullet << std::endl;
						return;
					}
					break;
				}
			}

			else {
				shared_ptr<NPC> target = npcs[key];
				if (target == NULL) continue;
				switch (P->view_dir) {
				case VIEW_UP:
					if (target->pos_x == P->pos_x && target->pos_y < P->pos_y) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->id;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						return;
					}
					break;
				case VIEW_DOWN:
					if (target->pos_x == P->pos_x && target->pos_y > P->pos_y) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->id;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						return;
					}
					break;
				case VIEW_LEFT:
					if (target->pos_y == P->pos_y && target->pos_x < P->pos_x) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->id;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						return;
					}
					break;
				case VIEW_RIGHT:
					if (target->pos_y == P->pos_y && target->pos_x > P->pos_x) {
						sc_packet_target_hit hit_p;
						hit_p.size = sizeof(hit_p);
						hit_p.type = SC_TARGET_HIT;
						hit_p.shoter_id = P->my_id_;
						hit_p.target_id = target->id;
						hit_p.shoter_x = P->pos_x;
						hit_p.shoter_y = P->pos_y;
						hit_p.target_x = target->pos_x;
						hit_p.target_y = target->pos_y;

						P->Send_Packet(&hit_p);
						return;
					}
					break;
				}
			}
		}
	}

	void Process_Packet(unsigned char *packet, int id)
	{
		shared_ptr<session> P= players[id];

		if (nullptr == P) return;

		int y = P->pos_y;
		int x = P->pos_x;
		switch (packet[1]) {
			case CS_UP: 
				y--;
				if (y < 0) y = 0;
				if (col[x][y] == TILE_BLACK) y++;
				view_dir = VIEW_UP;
				memcpy(&move_time, &packet[2], sizeof(move_time));
				break;
			case CS_DOWN:
				y++;
				if (y >= BOARD_HEIGHT) y = BOARD_HEIGHT - 1;
				if (col[x][y] == TILE_BLACK) y--;
				view_dir = VIEW_DOWN;
				memcpy(&move_time, &packet[2], sizeof(move_time));
				break;
			case CS_LEFT:
				x--; 
				if (x < 0) x = 0;
				if (col[x][y] == TILE_BLACK) x++;
				view_dir = VIEW_LEFT;
				memcpy(&move_time, &packet[2], sizeof(move_time));
				break;
			case CS_RIGHT:
				x++; 
				if (x >= BOARD_WIDTH) x = BOARD_WIDTH - 1; 
				if (col[x][y] == TILE_BLACK) x--;
				view_dir = VIEW_RIGHT;
				memcpy(&move_time, &packet[2], sizeof(move_time));
				break;
			case CS_SPACE: 
				if (P->bullet > 0) {
					P->bullet -= 1;
					serch_target(P->my_id_);
				}
				break;
			case CS_LOGOUT:
				for (auto& [key, player] : players) {
					const shared_ptr<session> p = player;
					if (p == nullptr) continue; //로그아웃한 경우 넘김
					if (p->my_id_ == id) continue; //자기 자신은 넣지 않음
					if (!can_see(id, p->my_id_)) continue;

					sc_packet_remove_player packet;

					packet.id = my_id_;
					packet.size = sizeof(p);
					packet.type = SC_REMOVE_PLAYER;

					p->Send_Packet(&packet);
				}
				players[my_id_] = nullptr;
				return;
				break;
			default: cout << "Invalid Packet From Client [" << id << "]\n"; system("pause"); exit(-1);
		}
		P->pos_x = x;
		P->pos_y = y;

		if (col[P->pos_x][P->pos_y] == TILE_RED) {
			ChangeHP(P->my_id_, 10);
		}


		sc_packet_pos sp_pos; //이동 패킷 준비

		sp_pos.id = id;
		sp_pos.size = sizeof(sc_packet_pos);
		sp_pos.type = SC_POS;
		sp_pos.x = P->pos_x;
		sp_pos.y = P->pos_y;
		sp_pos.move_time = move_time;
		sp_pos.view_dir = P->view_dir;

		sc_packet_put_player sp_put; //플레이어 추가 패킷 준비
		sp_put.id = id;
		sp_put.size = sizeof(sc_packet_put_player);
		sp_put.type = SC_PUT_PLAYER;
		sp_put.x = P->pos_x;
		sp_put.y = P->pos_y;
		sp_put.view_dir = P->view_dir;

		P->Send_Packet(&sp_pos); //본인이 이동함을 전달

		//----시야처리----
		P->vl.lock(); //이동한 플레이어의 기존 뷰리스트 저장
		unordered_set<int> old_vlist = P->view_list;
		P->vl.unlock();

		unordered_set<int> near_list; //이동한 뒤 시야 내의 오브젝트 리스트
		for (auto& [key, player] : players) {
			const shared_ptr<session> p = player;
			if (p == nullptr) continue; //로그아웃한 경우 넘김
			if (p->my_id_ == id) continue; //자기 자신은 넣지 않음
			if (can_see(id, p->my_id_)) //서로 볼 수 있는 경우 추가
				near_list.insert(p->my_id_);
		}

		for (auto& [key, npc] : npcs) {
			shared_ptr<NPC> n = npc;
			if (n == nullptr) continue;

			if (can_see_npc(id, n->id)) {
				near_list.insert(n->id);
				if (n->state == ST_IDLE) {
					TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_SAY_HELLO, id };
					timer_queue.push(ev);
				}
			}
		}

		for (auto& [key, item] : items) {
			shared_ptr<Item> it = item;
			if (it == nullptr) continue;
			if (!can_see_item(P->pos_x, P->pos_y, it->pos_x, it->pos_y)) continue;
			near_list.insert(key);
		}
		//-----새로운 시야 리스트 생성 완료

		for (auto& p_id : near_list) { //near_list 유저 처리
			if (p_id < MAX_USER) {
				shared_ptr<session> cpl = players[p_id];

				if (cpl == nullptr) continue;

				cpl->vl.lock();
				if (cpl->view_list.count(id)) {
					cpl->vl.unlock();
					cpl->Send_Packet(&sp_pos);
				}
				else {
					cpl->view_list.insert(id);
					cpl->vl.unlock();
					cpl->Send_Packet(&sp_put);

				}
				if (old_vlist.count(p_id) == 0) { //새로운 플레이어가 시야에 등장
					shared_ptr<session> pl = players[p_id];
					if (nullptr == pl) continue;

					sc_packet_put_player sp_put_o;
					sp_put_o.id = p_id;
					sp_put_o.size = sizeof(sc_packet_put_player);
					sp_put_o.type = SC_PUT_PLAYER;
					sp_put_o.x = pl->pos_x;
					sp_put_o.y = pl->pos_y;
					sp_put_o.view_dir = pl->view_dir;

					P->Send_Packet(&sp_put_o);
				}
			}
			else if(p_id <= MAX_USER + NUM_OF_NPC) { //near_list npc 처리
				shared_ptr<NPC>np = npcs[p_id];
				if (np == nullptr) continue;

				if (!np->is_active) {
					//npc를 active 시키고 타이머에 따라 움직이게 해야함
					wakeupNPC(p_id);
				}

				if (old_vlist.count(p_id) == 0) {
					sc_packet_put_object put_npc; //npc 추가 패킷 준비
					put_npc.id = np->id;
					put_npc.size = sizeof(sc_packet_put_object);
					put_npc.type = SC_PUT_OBJECT;
					put_npc.x = np->pos_x;
					put_npc.y = np->pos_y;
					put_npc.view_dir = 0;

					P->Send_Packet(&put_npc);
				}
			}

			else {
				//item의 경우
				shared_ptr<Item> item = items[p_id];
				if (item == nullptr) continue;

				if (P->pos_x == item->pos_x && P->pos_y == item->pos_y) {
					switch (item->type) {
					case ITEM_BULLET :
						P->bullet += 5;
						if (P->bullet > 30) P->bullet = 30;

						std::cout << "Player" << P->my_id_ << " use Item" << p_id << "remain bullet : " << P->bullet << std::endl;
						break;
					case ITEM_HEAL :
						P->hp += 5;
						if (P->hp > 10) P->hp = 10;

						std::cout << "Player" << P->my_id_ << " use Item" << p_id << "remain hp : " << P->hp << std::endl;

						break;
					}

					//아이템을 삭제해야한다.
					for (auto& [key, player] : players) {
						shared_ptr<session> pl = player;
						if (pl == nullptr) continue;
						if (!can_see_item(pl->pos_x, pl->pos_y, item->pos_x, item->pos_y)) continue;

						sc_packet_remove_item remove_it;
						remove_it.size = sizeof(sc_packet_remove_item);
						remove_it.type = SC_REMOVE_ITEM;
						remove_it.id = p_id;

						pl->Send_Packet(&remove_it);
					}

					items[p_id].reset();
					item.reset();

					g_item_EA -= 1;

					continue;
				}

				if (old_vlist.count(p_id) == 0) {
					sc_packet_put_item pitem;
					pitem.size = sizeof(sc_packet_put_item);
					pitem.type = SC_PUT_ITEM;
					pitem.id = p_id;
					pitem.item_type = item->type;
					pitem.item_x = item->pos_x;
					pitem.item_y = item->pos_y;

					P->Send_Packet(&pitem);
				}
			}
		}

		for (auto& pl : old_vlist) { //플레이어 삭제처리
			if (0 != near_list.count(pl)) continue;
			if (pl < MAX_USER) {
				shared_ptr<session> p = players[pl];
				if (p == nullptr) continue;
				if (pl == P->my_id_) continue;
				//내 시야에서 다른 플레이어가 사라지면 서로 삭제(서로 시야가 같은 경우)
	
				sc_packet_remove_player sp_re1;
				sp_re1.id = pl;
				sp_re1.size = sizeof(sc_packet_remove_player);
				sp_re1.type = SC_REMOVE_PLAYER;
				P->Send_Packet(&sp_re1);

				sc_packet_remove_player sp_re2;
				sp_re2.id = P->my_id_;
				sp_re2.size = sizeof(sc_packet_remove_player);
				sp_re2.type = SC_REMOVE_PLAYER;
				p->Send_Packet(&sp_re2);
				p->vl.lock();
				p->view_list.erase(P->my_id_);
				p->vl.unlock();
				
			}
			else if (pl <= NUM_OF_NPC + MAX_USER) {
				shared_ptr<NPC> np = npcs[pl];

				if (np == nullptr) continue;
				
				sc_packet_remove_player remove_npc;
				remove_npc.id = pl;
				remove_npc.size = sizeof(sc_packet_remove_player);
				remove_npc.type = SC_REMOVE_PLAYER;
				P->Send_Packet(&remove_npc);
			
			}

			else {
				shared_ptr<Item> item = items[pl];
				if (item == nullptr) continue;
				
				sc_packet_remove_item remove_it;
				remove_it.size = sizeof(sc_packet_remove_item);
				remove_it.type = SC_REMOVE_ITEM;
				remove_it.id = pl;

				P->Send_Packet(&remove_it);
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
	int view_dir;

	unordered_set<int> view_list;
	mutex vl;

	session(tcp::socket socket, int new_id)
		: socket_(std::move(socket)), my_id_(new_id)
	{
		/*pos_x = rand() % BOARD_WIDTH;
		pos_y = rand() % BOARD_HEIGHT;*/
		view_dir = VIEW_LEFT;
		pos_x = 0;
		pos_y = 0;
		hp = 10;
		bullet = 30;
		curr_packet_size_ = 0;
		prev_data_size_ = 0;
	}

	~session() {
		cout << my_id_ << "is disconnected\n";
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
		for (auto& [key,pl] : players) {
			shared_ptr<session> player = pl;

			if (player == nullptr) continue;
			if (can_see(player->my_id_, my_id_)) {
				player->Send_Packet(&p);
				player->vl.lock();
				player->view_list.insert(my_id_);
				player->vl.unlock();
			}
		}
		// 나에게 접속해 있는 다른 플레이어 정보를 전송
		// 나에게 주위에 있는 NPC의 정보를 전송
		for (auto &pl : players) {
			shared_ptr<session> player = pl.second;

			if (player == nullptr) continue;
			if (player->my_id_ == my_id_) continue;
			if (can_see(player->my_id_, my_id_)) {
				p.id = player->my_id_;
				p.x = player->pos_x;
				p.y = player->pos_y;
				Send_Packet(&p);
				view_list.insert(player->my_id_);
			}
		}

		for (auto& [key, npc] : npcs) {
			if (can_see_npc(my_id_, key)) {
				shared_ptr<NPC> n = npc;

				p.id = key;
				p.x = n->pos_x;
				p.y = n->pos_y;
				Send_Packet(&p);
				view_list.insert(key);

				wakeupNPC(key);
			}
		}

		for (auto& [key, item] : items) {
			shared_ptr<Item> itm = item;
			if (itm == nullptr) continue;
			if (can_see_item(pos_x, pos_y, itm->pos_x, itm->pos_y)) {
				sc_packet_put_item pitem;
				pitem.size = sizeof(sc_packet_put_item);
				pitem.type = SC_PUT_ITEM;
				pitem.id = key;
				pitem.item_type = item->type;
				pitem.item_x = item->pos_x;
				pitem.item_y = item->pos_y;

				Send_Packet(&pitem);
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

	void ChangeHP(int id, int dmg)
	{
		shared_ptr<session> P = players[my_id_];

		P->hp -= dmg;

		std::cout << "Player" << P->my_id_ << " remain HP : " << P->hp << std::endl;

		if (P->hp <= 0) {
			//체력이 0된 플레이어 로그 아웃 시키기
			sc_packet_game_over gameover_packet;
			gameover_packet.size = sizeof(sc_packet_game_over);
			gameover_packet.type = SC_GAME_OVER;

			P->Send_Packet(&gameover_packet);
		}
	}

	int GetID() { return my_id_; }
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
					//players[p_id] = std::make_shared<session>(std::move(socket_), p_id);
					//shared_ptr<session> pl = players[p_id];
					auto pl = std::make_shared<session>(std::move(socket_), p_id);
					if (nullptr == pl) {
						exit(1);
					}
					players[p_id] = pl;

					pl->start();
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

			while (col[x][y] == TILE_BLACK) {
				x = rand() % BOARD_WIDTH;
				y = rand() % BOARD_HEIGHT;
			}

			int id = i + MAX_USER;
			npcs[id] = std::make_shared<NPC>(std::move(socket_), id, x, y);
			
			shared_ptr<NPC> n = npcs[id];

			auto L = n->L = luaL_newstate();

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

int main()
{
	for (int i = 0; i < BOARD_WIDTH; ++i) {
		for (int j = 0; j < BOARD_HEIGHT; ++j) {
			col[i][j] = TILE_BLACK;
		}
	}

	//방 생성==============================
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 3; ++j) {
			int start_x = i * 11;
			int start_y = j * 11;
			for (int x = 0; x < 8; ++x)
				for (int y = 0; y < 8; ++y)
					col[start_x + x][start_y + y] = TILE_WHITE;
		}
	}
	//=====================================
	//복도 생성============================
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 30; ++j) {
			col[i * 11 + 3][j] = TILE_WHITE;
			col[i * 11 + 4][j] = TILE_WHITE;
		}
	}

	for (int j = 0; j < 3; ++j) {
		for (int i = 0; i < 52; ++i) {
			col[i][j * 11 + 3] = TILE_WHITE;
			col[i][j * 11 + 4] = TILE_WHITE;
		}
	}
	//=====================================
	boost::asio::io_context io_service;
	vector <thread > worker_threads;
	server s(io_service, MY_SERVER_PORT);

	boost::asio::steady_timer event_timer(io_service,boost::asio::chrono::microseconds(1000));
	event_timer.async_wait(boost::bind(event_excuter, boost::asio::placeholders::error, &event_timer));

	Init_Server();

	for (auto i = 0; i < 8; i++) worker_threads.emplace_back(worker_thread, &io_service);
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

	shared_ptr<session> pl = players[user_id];
	if (nullptr == pl) return 1;
	pl->Send_Packet(&packet);
	
	shared_ptr<NPC> npc = npcs[my_id];
	npc->state = ST_HELLO;

	TIMER_EVENT ev{ my_id, chrono::system_clock::now() + 3s, EV_SAY_BYE, user_id };
	timer_queue.push(ev);

	return 0;
}

void wakeupNPC(int n_id)
{
	if (n_id < MAX_USER) return;

	shared_ptr<NPC> npc = npcs[n_id];
	if (nullptr == npc) return;

	if (npc->is_active) return;
	bool old_state = false;

	if (false == atomic_compare_exchange_strong(&npc->is_active, &old_state, true))
		return;
	//타이머 이벤트 생성하고 우선순위 큐에 넣기
	TIMER_EVENT ev{ n_id, chrono::system_clock::now(), EV_RANDOM_MOVE, 0 };
	timer_queue.push(ev);
}

bool can_see_npc(int from, int to)
{
	shared_ptr<session> pl = players[from];
	if (nullptr == pl) return false;

	shared_ptr<NPC> np = npcs[to];
	if (nullptr == np) return false;

	if (abs(pl->pos_x - np->pos_x) > VIEW_RANGE) return false;
	return abs(pl->pos_y - np->pos_y) <= VIEW_RANGE;
}

bool can_see_item(int px, int py, int ix, int iy)
{
	if (abs(px - ix) > VIEW_RANGE) return false;
	return abs(py - iy) <= VIEW_RANGE;
}

void MoveNpc(int npc_id)
{

	unordered_set<int> old_vl;
	for (auto& [key, p] : players) {
		shared_ptr<session> pl = p;
		if (pl == nullptr) continue;
		if (can_see_npc(key, npc_id))
			old_vl.insert(key);
	}

	shared_ptr<NPC> npc = npcs[npc_id];
	if (nullptr == npc) return;

	int x = npc->pos_x;
	int y = npc->pos_y;
	switch (rand() % 4) {
	case 0: if (x < (BOARD_WIDTH - 1) && (TILE_WHITE == col[x+1][y])) x++; break;
	case 1: if (x > 0 && (TILE_WHITE == col[x-1][y])) x--; break;
	case 2: if (y < (BOARD_HEIGHT - 1) && (TILE_WHITE == col[x][y+1])) y++; break;
	case 3: if (y > 0 && (TILE_WHITE == col[x][y-1])) y--; break;
	}
	npc->pos_x = x;
	npc->pos_y = y;

	unordered_set<int> new_vl;
	for (auto& [key, p] : players) {
		shared_ptr<session> pl = p;
		if (pl == nullptr) continue;
		if (can_see_npc(key, npc_id)) {
			new_vl.insert(key);
			TIMER_EVENT ev{ npc_id, chrono::system_clock::now() + 1s, EV_SAY_HELLO, key };
			timer_queue.push(ev);
		}
	}

	for (auto pl : new_vl) {
		if (0 == old_vl.count(pl)) {
			// 플레이어의 시야에 등장

			sc_packet_put_object npc_put; //플레이어 추가 패킷 준비
			npc_put.id = npc_id;
			npc_put.size = sizeof(sc_packet_put_object);
			npc_put.type = SC_PUT_OBJECT;
			npc_put.x = npc->pos_x;
			npc_put.y = npc->pos_y;

			shared_ptr<session> player = players[pl];
			if (nullptr == player) continue;

			player->Send_Packet(&npc_put);
		}
		else {
			// 플레이어가 계속 보고 있음.
			sc_packet_pos npc_pos; //이동 패킷 준비

			npc_pos.id = npc_id;
			npc_pos.size = sizeof(sc_packet_pos);
			npc_pos.type = SC_POS;
			npc_pos.x = npc->pos_x;
			npc_pos.y = npc->pos_y;
			npc_pos.view_dir = -1;

			shared_ptr<session> player = players[pl];
			if (nullptr == player) continue;

			player->Send_Packet(&npc_pos);

		}
	}
	//삭제
	for (auto pl : old_vl) {
		if (0 == new_vl.count(pl)) {
			shared_ptr<session> player = players[pl];
			
			if (player == nullptr) continue;

			player->vl.lock();
			if (0 != player->view_list.count(npc_id)) {
				player->vl.unlock();
				sc_packet_remove_player npc_remove;

				npc_remove.id = npc_id;
				npc_remove.size = sizeof(sc_packet_remove_player);
				npc_remove.type = SC_REMOVE_PLAYER;

				player->Send_Packet(&npc_remove);
			}
			else {
				player->vl.unlock();
			}
		}

	}

	if (new_vl.size() == 0) {
		npc->is_active = false;
	}

	else {
		TIMER_EVENT ev{ npc_id, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
		timer_queue.push(ev);
	}
}

void event_excuter(const boost::system::error_code& ec, boost::asio::steady_timer* timer)
{
	if (!ec) {
		//루프로 들어가기 이전에 필요한 이벤트 삽입
		//========================================
		TIMER_EVENT item_event_init;
		item_event_init.event_id = EV_SPAWN_ITEM;
		item_event_init.obj_id = 0;
		item_event_init.target_id = 0;
		item_event_init.wakeup_time = chrono::system_clock::now() + 5s;

		timer_queue.push(item_event_init);

		TIMER_EVENT start_raser;
		start_raser.event_id = EV_START_RASER;
		start_raser.obj_id = 0;
		start_raser.target_id = rand() % 15;
		start_raser.wakeup_time = chrono::system_clock::now() + 10s;

		timer_queue.push(start_raser);
		//========================================
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
			}
			switch (ev.event_id) {
			case EV_RANDOM_MOVE: {
				shared_ptr<NPC> n = npcs[ev.obj_id];
				if (nullptr == n) break;

				if (n->state != ST_IDLE) {
					TIMER_EVENT n_ev{ ev.obj_id, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
					timer_queue.push(n_ev);
					break;
				}

				MoveNpc(ev.obj_id);
			}
				break;
			case EV_SAY_BYE: {
				const shared_ptr<session> target_player = players[ev.target_id];
				if (nullptr == target_player) break;

				shared_ptr<NPC> np = npcs[ev.obj_id];

				if (nullptr == np) break;

				np->state = ST_IDLE;
				sc_packet_chat packet;
				packet.id = ev.obj_id;
				packet.size = sizeof(sc_packet_chat);
				packet.type = SC_CHAT;
				strcpy_s(packet.message, "BYE");

				target_player->Send_Packet(&packet);
			}
				break;
			case EV_SAY_HELLO :{
				const shared_ptr<session> target_player = players[ev.target_id];
				if (nullptr == target_player) break;
				const shared_ptr<NPC> npc = npcs[ev.obj_id];
				if (nullptr == npc) break;

				npc->ll.lock();
				auto L = npc->L;
				lua_getglobal(L, "event_player_move");
				lua_pushnumber(L, ev.target_id);
				lua_pcall(L, 1, 0, 0);
				//lua_pop(L, 1);
				npc->ll.unlock();
			}
				break;
			case EV_SPAWN_ITEM: {
				if (g_item_EA >= MAX_ITEM_EA) {
					item_event_init.wakeup_time = chrono::system_clock::now() + 15s;
					timer_queue.push(item_event_init);
					break;
				}

				for (int i = 0; i < ITEM_SPAWN_EA; i++) {
					int id = g_item_ID++;

					items[id] = std::make_shared<Item>(id);
					shared_ptr<Item> item = items[id];
					if (item == nullptr) continue;

					for (auto [key, player] : players) {
						shared_ptr<session> p = player;
						if (p == nullptr) continue;
						if (!can_see_item(p->pos_x, p->pos_y, item->pos_x, item->pos_y)) continue;

						sc_packet_put_item pitem;
						pitem.size = sizeof(sc_packet_put_item);
						pitem.type = SC_PUT_ITEM;
						pitem.id = id;
						pitem.item_type = item->type;
						pitem.item_x = item->pos_x;
						pitem.item_y = item->pos_y;

						p->Send_Packet(&pitem);
					}
					//std::cout << "Spwan Item" << item->type << "in " << item->pos_x << ", " << item->pos_y << std::endl;
					g_item_EA += 1;
				}

				item_event_init.wakeup_time = chrono::system_clock::now() + 5s;
				timer_queue.push(item_event_init);
				break;
			}
			case EV_START_RASER: {
				TIMER_EVENT set_event;
				set_event.event_id = EV_SET_DANGER;
				set_event.obj_id = 0;
				set_event.target_id = ev.target_id;
				set_event.wakeup_time = chrono::system_clock::now() + 1s;

				timer_queue.push(set_event);

				std::cout << "레이저 시작, 방 번호 : " << ev.target_id << std::endl;

				start_raser.target_id = rand() % 15;
				start_raser.wakeup_time = chrono::system_clock::now() + 10s;

				timer_queue.push(start_raser);

				break;
			}
			case EV_SET_DANGER:
			case EV_SET_RASER:
			case EV_SET_WHITE: {
				int tile_type = TILE_WHITE;
				if (ev.event_id == EV_SET_DANGER) tile_type = TILE_DANGER;
				if (ev.event_id == EV_SET_RASER) tile_type = TILE_RED;

				int x = ev.target_id % 5;
				int y = ev.target_id / 5;

				int start_x = x * 11;
				int start_y = y * 11;

				for (int i = 0; i < 8; i++) {
					col[start_x + i][start_y + ev.obj_id] = tile_type;
					if (tile_type == TILE_RED) {
						for (auto& [key, player] : players) {
							const shared_ptr<session> p = player;
							if (p == nullptr) continue;
							if (p->pos_x == start_x + i && p->pos_y == start_y + ev.obj_id) {
								p->ChangeHP(p->GetID(), 10);
							}

						}
					}
				}

				for (auto [key, player] : players) {
					shared_ptr<session> p = player;
					if (p == nullptr) continue;
					
					sc_packet_change_tile packet;

					packet.size = sizeof(sc_packet_change_tile);
					packet.type = SC_CHANGE_TILE;
					packet.col_x = start_x;
					packet.col_y = start_y + ev.obj_id;
					packet.tile_type = tile_type;

					p->Send_Packet(&packet);
				}

				if (ev.event_id == EV_SET_DANGER) {
					TIMER_EVENT set_event;
					set_event.event_id = EV_SET_RASER;
					set_event.obj_id = ev.obj_id;
					set_event.target_id = ev.target_id;
					set_event.wakeup_time = chrono::system_clock::now() + 1s;

					timer_queue.push(set_event);

					if (ev.obj_id < 7) {
						TIMER_EVENT set_event2;
						set_event2.event_id = EV_SET_DANGER;
						set_event2.obj_id = ev.obj_id + 1;
						set_event2.target_id = ev.target_id;
						set_event2.wakeup_time = chrono::system_clock::now() + 1s;

						timer_queue.push(set_event2);
					}
				}
				else if (ev.event_id == EV_SET_RASER) {
					TIMER_EVENT set_event;
					set_event.event_id = EV_SET_WHITE;
					set_event.obj_id = ev.obj_id;
					set_event.target_id = ev.target_id;
					set_event.wakeup_time = chrono::system_clock::now() + 1s;

					timer_queue.push(set_event);

				}

				break;
			}
			}
		}


		timer->expires_at(timer->expiry() + boost::asio::chrono::seconds(0));
		timer->async_wait(boost::bind(event_excuter, boost::asio::placeholders::error, timer));
	}
}
