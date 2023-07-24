#pragma once
#include "headers.h"

using namespace std;
using namespace chrono;
using boost::asio::ip::tcp;

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
						std::cout << "Incomplete Send occured on session[" << id << "]. This session should be closed.\n";
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

	void send_move_packet()
	{
		unordered_set<int> old_vl;
		for (auto& [key, p] : players) {
			if (p == nullptr) continue;
			if (can_see_npc(key, id))
				old_vl.insert(key);
		}

		int x = pos_x;
		int y = pos_y;
		switch (rand() % 4) {
		case 0: if (x < (BOARD_WIDTH - 1)) x++; break;
		case 1: if (x > 0) x--; break;
		case 2: if (y < (BOARD_HEIGHT - 1)) y++; break;
		case 3:if (y > 0) y--; break;
		}
		pos_x = x;
		pos_y = y;

		unordered_set<int> new_vl;
		for (auto& [key, p] : players) {
			if (p == nullptr) continue;
			if (can_see_npc(key, id))
				new_vl.insert(key);
		}

		for (auto pl : new_vl) {
			if (0 == old_vl.count(pl)) {
				// 플레이어의 시야에 등장

				sc_packet_put_player npc_put; //플레이어 추가 패킷 준비
				npc_put.id = id;
				npc_put.size = sizeof(sc_packet_put_player);
				npc_put.type = SC_PUT_PLAYER;
				npc_put.x = pos_x;
				npc_put.y = pos_y;
				players[pl]->Send_Packet(&npc_put);
			}
			else {
				// 플레이어가 계속 보고 있음.

				sc_packet_pos npc_pos; //이동 패킷 준비

				npc_pos.id = id;
				npc_pos.size = sizeof(sc_packet_pos);
				npc_pos.type = SC_POS;
				npc_pos.x = pos_x;
				npc_pos.y = pos_y;

				players[pl]->Send_Packet(&npc_pos);
			}
		}
		//삭제
		for (auto pl : old_vl) {
			if (0 == new_vl.count(pl)) {
				players[pl]->vl.lock();
				if (0 != players[pl]->view_list.count(id)) {
					players[pl]->vl.unlock();
					sc_packet_remove_player npc_remove;

					npc_remove.id = id;
					npc_remove.size = sizeof(sc_packet_remove_player);
					npc_remove.type = SC_REMOVE_PLAYER;

					players[pl]->Send_Packet(&npc_remove);
				}
				else {
					players[pl]->vl.unlock();
				}
			}
		}
	}
};
