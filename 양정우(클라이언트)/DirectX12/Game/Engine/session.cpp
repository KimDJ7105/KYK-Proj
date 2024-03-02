#pragma once
#include "pch.h"
#include "session.h"

SESSION::SESSION(tcp::socket socket_) : sock(std::move(socket_))
{
	curr_packet_size_ = 0;
	prev_data_size_ = 0;
}

void SESSION::Process_Packet(unsigned char* packet)
{
	int test = 0;
	switch (packet[1]) {
	case SC_LOGIN_INFO:
	{
		sc_packet_login_info* p = reinterpret_cast<sc_packet_login_info*>(packet + 2);
		int object_type = 0;
		int object_id = p->id;
		float x = p->x;
		float y = p->y;
		float z = p->z;
		int animation_id = 0;
		float direction = 0;
		//game->CreateAvatar(object_type, object_id, x, y, z, animation_id, direction);
		break;
	}
	case SC_PUT_PLAYER:
	{
		sc_packet_put* p = reinterpret_cast<sc_packet_put*>(packet + 2);
		int object_type = 0;
		int object_id = p->id;
		float x = p->x;
		float y = p->y;
		float z = p->z;
		int animation_id = 0;
		float direction = 0;
		//game->CreateObject(object_type, object_id, x, y, z, animation_id, direction);
		break;
	}
	case SC_POS:
	{
		sc_packet_pos* p = reinterpret_cast<sc_packet_pos*>(packet + 2);
		int object_id = p->id;
		float x = p->x;
		float y = p->y;
		float z = p->z;		// sock log info에 z에 해당하는 것이 없어 0으로 하였음
		float direction = 0;
		//game->ChangeObjectLocation(object_id, x, y, z, direction);
		break;
	}
	default: // 지정되지 않은 패킷을 수신받았을 때
		return;
		break;
	}
}

void SESSION::do_connect(tcp::resolver::results_type endpoint)
{
	async_connect(sock, endpoint,
		[this](const boost::system::error_code& ec, const tcp::endpoint& endpoint)
		{
			if (!ec) {
				do_read();
			}

			else return -1;
		}
	);
}

void SESSION::do_write(unsigned char* packet, std::size_t length)
{
	sock.async_write_some(boost::asio::buffer(packet, length),
		[packet, length](boost::system::error_code ec, std::size_t bytes_transferred)
		{
			if (!ec)
			{
				if (length != bytes_transferred) {
					return -1;
				}
				delete packet;
			}
		});
}

void SESSION::do_read()
{
	sock.async_read_some(boost::asio::buffer(data_),
		[this](boost::system::error_code ec, std::size_t length)
		{
			if (ec)
			{
				if (ec.value() == boost::asio::error::operation_aborted) return;

				return;
			}

			int data_to_process = static_cast<int>(length);
			unsigned char* buf = data_;
			while (0 < data_to_process) {
				if (0 == curr_packet_size_) {
					curr_packet_size_ = buf[0];
					if (buf[0] > 200) {
						exit(-1);
					}
				}
				int need_to_build = curr_packet_size_ - prev_data_size_;
				if (need_to_build <= data_to_process) {
					memcpy(packet_ + prev_data_size_, buf, need_to_build);
					Process_Packet(packet_);
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

void SESSION::Send_Packet(void* packet)
{
	int packet_size = reinterpret_cast<unsigned char*>(packet)[0];
	unsigned char* buff = new unsigned char[packet_size];
	memcpy(buff, packet, packet_size);
	do_write(buff, packet_size);
}

SESSION* session;

void MoveSession(tcp::socket& sock)
{
	session = new SESSION(std::move(sock));
}

SESSION* GetSession() { return session; }