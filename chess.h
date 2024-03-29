#pragma once
#include <iostream>
#include <unordered_map>
#include <assert.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
#define BOARD_DIM 8
#define PIECE_TYPES 6
#define INDEX(x, y) ((x) + (y) * BOARD_DIM)
#define BOARD_AT(x, y, board) ((board >> INDEX(x, y) & u64(1)))
#define BOARD_AT_I(i, board) ((board >> i & u64(1)))
#define BOARD_RESET(x, y, board) (board &= ~(u64(1) << INDEX(x, y))) 
#define BOARD_RESET_I(i, board) (board &= ~(u64(1) << i)) 
#define BOARD_SET(x, y, board) (board |= (u64(1) << INDEX(x, y)))
#define IN_FIELD(x) ((x) >= 0 && (x) < BOARD_DIM)
#define IN_FIELD2(x,y) IN_FIELD(x) && IN_FIELD(y)

enum 
{
	white, black 
};
enum
{
	pawn, knight, bishop, rook, king, queen
};
enum piece_index 
{
	b_pawn, b_knight, b_bishop, b_rook, b_king, b_queen,
	w_pawn, w_knight, w_bishop, w_rook, w_king, w_queen,
	pieces_max
};

struct vector2f;
struct vector2i;
struct vector2
{
	u32 x;
	u32 y;
	vector2() { x = 0; y = 0; };
	vector2(u32 x, u32 y): x(x), y(y){};
	vector2(float x, float y): x((u32)x), y((u32)y){};
	vector2(vector2f v);
	vector2(vector2i v);
};
struct vector2f
{
	float x;
	float y;
	vector2f() { x = 0; y = 0; };
	vector2f(float x, float y): x(x), y(y){};
	vector2f(u32 x, u32 y): x((float)x), y((float)y){};
	vector2f(vector2 v) {
		x = (float)v.x;
		y = (float)v.y;
	}
};
struct vector2i
{
	int x;
	int y;
	vector2i() { x = 0; y = 0; };
	vector2i(int x, int y): x(x), y(y){};
	vector2i(u32 x, u32 y): x((int)x), y((int)y){};
	vector2i(vector2 v) {
		x = (int)v.x;
		y = (int)v.y;
	}
	vector2i(vector2f v) {
		x = (int)v.x;
		y = (int)v.y;
	}
};
inline vector2::vector2(vector2f v)
{
	x = (u32)v.x;
	y = (u32)v.y;
}
inline vector2::vector2(vector2i v)
{
	x = (u32)v.x;
	y = (u32)v.y;
}
class chess 
{
public:
	struct move 
	{
		u64 org = 0;
		u64 dst = 0;
		u64 attacks = 0;
		u64 pawn_double_jump = 0;
		u64 en_passant_attack = 0;
	};

	bool player = false;
	move current_moves = { 0 };
	vector2 selected_cell = vector2();
	bool selected = 0;
	bool in_check = false;
	int selected_type =  0;
	u64 pieces[pieces_max] = { 0 };
	u64 white_pieces = 0;
	u64 black_pieces = 0;
	u64 rook_moved = 0;
	u64 all = 0;
	u64 empty = 0;
	u64 white_sees = 0;
	u64 black_sees = 0;
	u64 sees_king = 0;
	move last_move = { 0 };
	std::vector<move> all_moves = {};
	std::vector<int> taken_pieces = {};
 
	u64 pawn_attacks(u32 x, u32 y, move& move);
	u64 get_piece_mask(bool white);
	move pawn_moves(u32 x, u32 y);
	move knight_moves(u32 x, u32 y);
	move sliding_piece(u32 x, u32 y, int type);
	move legal_moves(u32 x, u32 y, int type);
	void make_move(move move);
	void init_board();
	void scan_board(bool player);
	bool check_for_check();

private:
	vector2i sliding_dirs[4] = { {-1, 1}, {1, 1}, {1, 0}, {0, 1}};
	std::unordered_map<u64, move> move_cache;
};

