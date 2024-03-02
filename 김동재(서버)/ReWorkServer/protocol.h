#pragma once
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define BOARD_WIDTH   52
#define BOARD_HEIGHT  30

#define MAX_USER 10000

#define MY_SERVER_IP "127.0.0.1"
#define MY_SERVER_PORT  "4000"
#define MY_PORT 4000

#define CS_POS_INFO 1

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_LOGIN_INFO	 4


#pragma pack (push, 1)

//---------Client to Server-----------------

struct cs_packet_pos_info {
	BYTE size;
	BYTE type;
	float x;
	float y;
	float z;
};

struct cs_packet_logout {
	BYTE size;
	BYTE type;
};

//---------Server To Client-----------------

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
};

struct sc_packet_put {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
};

struct sc_packet_login_info {
	BYTE size;
	BYTE type;
	WORD id;
	float x;
	float y;
	float z;
};

struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

#pragma pack (pop)