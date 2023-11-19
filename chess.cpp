#include "chess.h"
// generalize?
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
	white_pieces = get_piece_mask(white);
	black_pieces = get_piece_mask(black);
}
void chess::move(vector2 org, vector2 dst, u64& player_pieces)
{
	for (int i = 0; i < pieces_max; ++i) {
		BOARD_RESET(dst.x, dst.y, pieces[i]);
	}
	BOARD_RESET(org.x, org.y, player_pieces);
	BOARD_SET(dst.x, dst.y, player_pieces);
	white_pieces = get_piece_mask(white);
	black_pieces = get_piece_mask(black);
	player = !player;
	std::cout << "moved from " << org.x << ", " << org.y << " to " << dst.x << ", " << dst.y << "\n";
}
u64 chess::legal_moves(u32 x, u32 y, int type)
{
	u64 mv = 0;
	bool white = type >= PIECE_TYPES;
	switch (type % PIECE_TYPES) {
	case pawn:
		mv = pawn_moves(x, y, white);
		break;
	case knight:
		mv = knight_moves(x, y, white);
	default:
		break;
	}
	// pawn
	return mv;
}

u64 chess::pawn_moves(u32 x, u32 y, bool white)
{
	u64 moves = 0;
	int dir = white ? +1 : -1;
	if(IN_FIELD(y + dir)) BOARD_SET(x, y + dir, moves);
	moves &= ~(white_pieces | black_pieces);
	if (moves && ((white && y == 1) || (!white && y == BOARD_DIM - 2))) {
		if(IN_FIELD(y + dir * 2)) BOARD_SET(x, y + dir * 2, moves);
	}
	moves &= ~(white_pieces | black_pieces);
	u64 attacks = pawn_attacks(x, y, white);
	attacks &= white ? black_pieces : white_pieces;
	//check en peasant?
	return moves | attacks;
}
u64 chess::knight_moves(u32 x, u32 y, bool white)
{
	u64 moves = 0;
	int new_x, new_y;
	for (u32 i = 0; i < 4; ++i) {
		int main_dir = i % 2 == 0 ? 2 : -2;
		for (u32 j = 0; j < 2; ++j) {
			int sec_dir = j == 0 ? 1 : -1;
			new_x = i < 2 ? x + sec_dir : x + main_dir;
			new_y = i < 2 ? y + main_dir: y + sec_dir;
			if (IN_FIELD2(new_x, new_y)) {
				BOARD_SET(new_x, new_y, moves);
			}
		}
	}
	moves &= ~(white ? white_pieces : black_pieces);
	return moves;
}
u64 chess::get_piece_mask(bool white)
{
	u64 result = 0;
	int start = white ? PIECE_TYPES : 0;
	for (int i = start; i < start + PIECE_TYPES; ++i) {
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
