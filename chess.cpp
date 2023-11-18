#include "chess.h"
void chess::init_board()
{
	pieces[w_pawn] = 0b1111111100000000;
	pieces[b_pawn] = (u64)0b1111111100000000 << (u64)(8 * 5);
	pieces[w_knight] = 0b01000010;
	pieces[b_knight] = (u64)0b01000010 << (u64)(8 * 7);
	pieces[w_bishop] = 0b00100100;
	pieces[b_bishop] = (u64)0b00100100 << (u64)(8 * 7);
	pieces[w_rook] = 0b10000001;
	pieces[b_rook] = (u64)0b10000001 << (u64)(8 * 7);
	pieces[w_queen] = 0b00010000;
	pieces[b_queen] = (u64)0b00010000 << (u64)(8 * 7);
	pieces[w_king] =  0b00001000;
	pieces[b_king] =  (u64)0b00001000 << (u64)(8 * 7);
	white_pieces = get_piece_mask(true);
	black_pieces = get_piece_mask(false);
}
void chess::move(vector2 org, vector2 dst, u64& player_pieces)
{
	for (int i = 0; i < 12; ++i) {
		BOARD_RESET(dst.x, dst.y, pieces[i]);
	}
	BOARD_RESET(org.x, org.y, player_pieces);
	BOARD_SET(dst.x, dst.y, player_pieces);
	white_pieces = get_piece_mask(true);
	black_pieces = get_piece_mask(false);
	player = !player;
	std::cout << "moved from " << org.x << ", " << org.y << " to " << dst.x << ", " << dst.y << "\n";
}
u64 chess::legal_moves(u32 x, u32 y, int type)
{
	u64 mv = 0;
	bool white = type >= 6;
	// pawn
	if (type % 6 == 0) {
		mv = pawn_moves(x, y, white);
	}
	//int start = type < 6 ? 6 : 0;
	//for(int i = type < 6; i < start + 6; ++i)
	return mv;
}

u64 chess::pawn_moves(u32 x, u32 y, bool white)
{
	u64 moves = 0;
	int dir = white ? +1 : -1;
	if(IN_FIELD(y + dir)) BOARD_SET(x, y + dir, moves);
	moves &= ~(white_pieces | black_pieces);
	if (moves && ((white && y == 1) || (!white && y == 6))) {
		if(IN_FIELD(y + dir * 2)) BOARD_SET(x, y + dir * 2, moves);
	}
	moves &= ~(white_pieces | black_pieces);
	u64 attacks = pawn_attacks(x, y, white);
	attacks &= white ? black_pieces : white_pieces;
	//check en peasant?
	return moves | attacks;
}
u64 chess::get_piece_mask(bool white)
{
	u64 result = 0;
	int start = white ? 6 : 0;
	for (int i = start; i < start + 6; ++i) {
		result |= pieces[i];
	}
	return result;
}
u64 chess::pawn_attacks(u32 x, u32 y, bool white)
{
	u64 attacks = 0;
	int dir = white ? +1 : -1;
	if (IN_FIELD(y + dir)) {
		if (IN_FIELD(x + 1)) BOARD_SET(x + 1, y + dir, attacks);
		if (IN_FIELD(x - 1)) BOARD_SET(x - 1, y + dir, attacks);
	}
	return attacks;
}
