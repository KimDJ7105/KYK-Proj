#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_BUFF_SIZE   4000
#define MAX_PACKET_SIZE  255

#define BOARD_WIDTH   52
#define BOARD_HEIGHT  30

#define VIEW_RADIUS  3
#define VIEW_RANGE   4

#define MAX_USER 10000

#define NPC_START  2000
#define NUM_OF_NPC  50

#define MY_SERVER_PORT  4000

#define MAX_STR_SIZE  100

#define CS_UP    1
#define CS_DOWN  2
#define CS_LEFT  3
#define CS_RIGHT 4
#define CS_SPACE 5
#define CS_CHAT  6
#define CS_LOGOUT 7

#define SC_POS           1
#define SC_PUT_PLAYER    2
#define SC_PUT_OBJECT	 3
#define SC_REMOVE_PLAYER 4
#define SC_CHAT			 5
#define SC_LOGIN_INFO	 6
#define SC_TARGET_HIT    7
#define SC_GAME_OVER     8
#define SC_PUT_ITEM      9
#define SC_REMOVE_ITEM   10
#define SC_CHANGE_TILE   11

#define	VIEW_UP    1
#define VIEW_DOWN  2
#define VIEW_LEFT  3
#define VIEW_RIGHT 4

#define ITEM_BULLET 0
#define ITEM_HEAL   1
#define ITEM_SPAWN_EA 5
#define MAX_ITEM_EA 1000

#define TILE_WHITE 0
#define TILE_BLACK 1
#define TILE_DANGER 2
#define TILE_RED 3

unsigned short col[BOARD_WIDTH][BOARD_HEIGHT];

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

struct cs_packet_space {
	BYTE size;
	BYTE type;
};

struct cs_packet_chat {
	BYTE size;
	BYTE type;
	WCHAR message[MAX_STR_SIZE];
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
	WORD view_dir;
	unsigned int move_time;
};

struct sc_packet_put_player {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
	WORD view_dir;
};

struct sc_packet_put_object {
	BYTE size;
	BYTE type;
	WORD id;
	WORD x;
	WORD y;
	WORD view_dir;
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

struct sc_packet_chat {
	BYTE size;
	BYTE type;
	WORD id;
	char message[MAX_STR_SIZE];
};

struct sc_packet_target_hit {
	BYTE size;
	BYTE type;
	WORD shoter_id;
	WORD target_id;
	WORD shoter_x;
	WORD shoter_y;
	WORD target_x;
	WORD target_y;
};

struct sc_packet_game_over {
	BYTE size;
	BYTE type;
};

struct sc_packet_put_item {
	BYTE size;
	BYTE type;
	WORD id;
	WORD item_type;
	WORD item_x;
	WORD item_y;
};

struct sc_packet_remove_item {
	BYTE size;
	BYTE type;
	WORD id;
};

struct sc_packet_change_tile {
	BYTE size;
	BYTE type;
	unsigned short col_x;
	unsigned short col_y;
	WORD tile_type;
};

#pragma pack (pop)