#pragma once
#include "stdafx.h"

class SESSION;

concurrency::concurrent_unordered_map<int, shared_ptr<SESSION>> players;

class SESSION
	: public std::enable_shared_from_this<SESSION>
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

private:
	void Send_Packet(void* packet, unsigned id)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		players[id]->do_write(buff, packet_size);
	}

	void Process_Packet(unsigned char* packet, int id)
	{
		auto P = players[id];
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

		sc_packet_pos sp;

		sp.id = id;
		sp.size = sizeof(sc_packet_pos);
		sp.type = SC_POS;
		sp.x = P->pos_x;
		sp.y = P->pos_y;

		for (auto& pl : players)
			pl.second->Send_Packet(&sp);
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
				unsigned char* buf = data_;
				while (0 < data_to_process) {
					if (0 == curr_packet_size_) {
						curr_packet_size_ = buf[0];
						if (buf[0] > 200) {
							cout << "Invalid Packet Size [ << buf[0] << ] Terminating Server!\n";
							exit(-1);
						}
					}
					int need_to_build = curr_packet_size_ - prev_data_size_;
					if (need_to_build <= data_to_process) {
						memcpy(packet_ + prev_data_size_, buf, need_to_build);
						Process_Packet(packet_, my_id_);
						curr_packet_size_ = 0;
						prev_data_size_ = 0;
						data_to_process -= need_to_build;
						buf += need_to_build;
					}
					else {
						memcpy(packet_ + prev_data_size_, buf, data_to_process);
						prev_data_size_ += data_to_process;
						data_to_process = 0;
						buf += data_to_process;
					}
				}
				do_read();
			});
	}

	void do_write(unsigned char* packet, std::size_t length)
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
					delete packet;
				}
			});
	}

public:
	SESSION(tcp::socket socket, int new_id)
		: socket_(std::move(socket)), my_id_(new_id)
	{
		curr_packet_size_ = 0;
		prev_data_size_ = 0;
	}

	void start()
	{
		do_read();

		/*sc_packet_login_info pl;
		pl.id = my_id_;
		pl.size = sizeof(sc_packet_login_info);
		pl.type = SC_LOGIN_INFO;
		pl.x = pos_x;
		pl.y = pos_y;
		Send_Packet(&pl);

		sc_packet_put p;
		p.id = my_id_;
		p.size = sizeof(sc_packet_put);
		p.type = SC_PUT_PLAYER;
		p.x = pos_x;
		p.y = pos_y;

		for (auto& pl : players)
			if (pl.second != nullptr)
				pl.second->Send_Packet(&p);
		for (auto& pl : players) {
			if (pl.second->my_id_ != my_id_) {
				p.id = pl.second->my_id_;
				p.x = pl.second->pos_x;
				p.y = pl.second->pos_y;
				Send_Packet(&p);
			}
		}*/
	}

	void Send_Packet(void* packet)
	{
		int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
		unsigned char* buff = new unsigned char[packet_size];
		memcpy(buff, packet, packet_size);
		do_write(buff, packet_size);
	}
};
