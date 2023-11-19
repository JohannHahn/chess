#pragma once

#ifndef DEFINITIONS 
#define DEFINITIONS
#include <iostream>
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;
#define BOARD_DIM 8
#define PIECE_TYPES 6
#define INDEX(x, y) ((x) + (y) * BOARD_DIM)
#define BOARD_AT(x, y, board) ((board >> INDEX(x, y) & u64(1)))
#define BOARD_RESET(x, y, board) (board &= ~(u64(1) << INDEX(x, y))) 
#define BOARD_SET(x, y, board) (board |= (u64(1) << INDEX(x, y)))
#define IN_FIELD(x) ((x) >= 0 && (x) < BOARD_DIM)
#define IN_FIELD2(x,y) IN_FIELD(x) && IN_FIELD(y)

enum 
{
	black, white  
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
struct vector2
{
	u32 x;
	u32 y;
	vector2() { x = 0; y = 0; };
	vector2(u32 x, u32 y): x(x), y(y){};
	vector2(float x, float y): x((u32)x), y((u32)y){};
	vector2(vector2f v);
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
inline vector2::vector2(vector2f v)
{
	x = (u32)v.x;
	y = (u32)v.y;
}
#endif // DEFINITIONS

class chess 
{
public:
	bool board_flip = true;
	bool player = false;
	u64 current_moves = 0;
	vector2 selected_cell = vector2();
	bool selected = 0;
	int dragging_map = 0;
	u64 pieces[pieces_max] = { 0 };
	u64 white_pieces = 0;
	u64 black_pieces = 0;

	u64 pawn_attacks(u32 x, u32 y, bool white);
	u64 get_piece_mask(bool white);
	u64 pawn_moves(u32 x, u32 y, bool white);
	u64 knight_moves(u32 x, u32 y, bool white);
	u64 legal_moves(u32 x, u32 y, int type);
	void move(vector2 org, vector2 dst, u64& player_pieces);
	void init_board();
	template<typename T> 
	inline T translate_coords(T coords) 
	{
		T result = T();
		switch (board_flip) {
		case false:
			result.x = coords.x;
			result.y = (BOARD_DIM - 1) - coords.y;
			break;
		case true:
			result.x = (BOARD_DIM - 1) - coords.x;
			result.y = coords.y;
			break;
		}
		return result;
	}

private:
};

