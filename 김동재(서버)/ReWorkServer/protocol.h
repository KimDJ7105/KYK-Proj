#pragma once
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define BOARD_WIDTH   52
#define BOARD_HEIGHT  30

#define MAX_USER 10000

#define MY_SERVER_PORT  4000

#define CS_UP    1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT 4

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_REMOVE_PLAYER 3
#define SC_LOGIN_INFO	 4


#pragma pack (push, 1)

struct cs_packet_up {
	BYTE size;
	BYTE type;
	unsigned int move_time;
};

struct cs_packet_down {
	BYTE size;
	BYTE type;
	unsigned int move_time;
};

struct cs_packet_left {
	BYTE size;
	BYTE type;
	unsigned int move_time;
};

struct cs_packet_right {
	BYTE size;
	BYTE type;
	unsigned int move_time;
};

struct cs_packet_logout {
	BYTE size;
	BYTE type;
};

struct sc_packet_pos {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
};

struct sc_packet_put {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
};

struct sc_packet_login_info {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
};

struct sc_packet_remove_player {
	BYTE size;
	BYTE type;
	WORD id;
};

#pragma pack (pop)