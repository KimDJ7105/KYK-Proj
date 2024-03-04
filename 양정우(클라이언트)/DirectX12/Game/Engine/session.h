#pragma once
#include "pch.h"

//================================================================================
//서버 통신을 위한 SESSION Class
class SESSION {
private:
	tcp::socket sock;
	int curr_packet_size_;
	int prev_data_size_;
	unsigned char data_[1024];
	unsigned char packet_[1024];
public:

	SESSION(tcp::socket socket_);

	void Process_Packet(unsigned char* packet);

	void do_connect(tcp::resolver::results_type endpoint);

	void do_write(unsigned char* packet, std::size_t length);

	void do_read();

	void Send_Packet(void* packet);

};

void MoveSession(tcp::socket& sock);

extern SESSION* session;

SESSION* GetSession();
