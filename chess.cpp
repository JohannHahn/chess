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
	pieces[w_queen] = 0b00001000;
	pieces[b_queen] = pieces[w_queen] << (u64)(8 * 7);
	pieces[w_king] =  0b00010000;
	pieces[b_king] =  pieces[w_king] << (u64)(8 * 7);
	white_pieces = get_piece_mask(white);
	black_pieces = get_piece_mask(black);
	assert(!(white_pieces & black_pieces) && "pieces overlap");
	all = white_pieces | black_pieces;
	empty = ~(all);
}
void chess::move(vector2 org, vector2 dst)
{
	u64 player_pieces = player == white ? white_pieces : black_pieces;
	u64 enemy_pieces = all & ~(player_pieces);
	vector2 take = dst;
	for (int i = 0; i < pieces_max; ++i) {
		BOARD_RESET(take.x, take.y, pieces[i]);
	}
	if (BOARD_AT(dst.x, dst.y, get_piece_mask(!player))) {
		std::cout << "wtf `?\n";
	}
	for (int i = 0; i < pieces_max; ++i) {
		if (BOARD_AT(org.x, org.y, pieces[i])) {
			BOARD_RESET(org.x, org.y, pieces[i]);
			BOARD_SET(dst.x, dst.y, pieces[i]);
		}
	}
	white_pieces = get_piece_mask(white);
	assert(!(white_pieces & black_pieces) && "pieces overlap");
	all = white_pieces | black_pieces;
	empty = ~(all);
	player = !player;
	move_cache.clear();
	std::cout << "en_passant = " << en_passant << "\n";
	std::cout << "passant_take = " << passant_take << "\n";
	std::cout << "passtant take pos = " << passant_take_index.x << ", " << passant_take_index.y << "\n";
}
u64 chess::legal_moves(u32 x, u32 y, int type)
{
	u64 moves = 0;
	if (!move_cache.contains(INDEX(x, y))) {

		bool white = type >= PIECE_TYPES;
		int type_reduced = type % PIECE_TYPES;
		if (type_reduced == pawn) {
			moves = pawn_moves(x, y, white);
		}
		else if (type_reduced == knight) {
			moves = knight_moves(x, y, white);
		}
		else {
			moves = sliding_piece(x, y, type_reduced, white);
		}
		move_cache[INDEX(x, y)] = moves;
	}
	return move_cache[INDEX(x, y)];
}

u64 chess::pawn_moves(u32 x, u32 y, bool white)
{
	u64 moves = 0;
	int dir = white ? +1 : -1;
	u64 attacks = pawn_attacks(x, y, white);
	if(IN_FIELD(y + dir)) BOARD_SET(x, y + dir, moves);
	// check if blocked
	moves &= empty;
	// if not blocked
	if (moves && ((white && y == 1) || (!white && y == BOARD_DIM - 2))) {
		if (IN_FIELD(y + dir * 2)) {
			BOARD_SET(x, y + dir * 2, moves);
			moves &= empty;
			passant_take = true;
			en_passant = 0;
			BOARD_SET(x, y + dir, en_passant);
			passant_take_index = vector2(x, y + dir * 2);
		}
	}
	return moves | attacks;
}
u64 chess::pawn_attacks(u32 x, u32 y, bool white)
{
	u64 attacks = 0;
	int dir = white ? +1 : -1;
	if (IN_FIELD(y + dir)) {
		if (IN_FIELD(x + 1)) BOARD_SET(x + 1, y + dir, attacks);
		if (IN_FIELD(x - 1)) BOARD_SET(x - 1, y + dir, attacks);
	}
	attacks &= white ? black_pieces : white_pieces;
	return attacks;
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
u64 chess::sliding_piece(u32 x, u32 y, int type, bool white)
{
	u64 moves = 0;
	u32 num_dirs = 2;
	u32 start = 0;
	u32 range = 8;
	u64 dir_open = SIZE_MAX;
	u64 enemy = white ? black_pieces : white_pieces;
	u64 ally = white ? white_pieces: black_pieces;
	if (type == king) {
		num_dirs = 4;
		range = 1;
	}
	if (type == rook) start = 2;
	if (type == queen) num_dirs = 4;
	for (u32 r = 1; r <= range; ++r) {
		for (u32 i = start; i < start + num_dirs; ++i) {
			vector2i dir = sliding_dirs[i];
			for (u32 j = 0; j < 2; ++j) {
			if (!BOARD_AT(j, i, dir_open)) continue;
				vector2i new_pos = j % 2 == 0 ?
					vector2i(x + dir.x * r, y + dir.y * r) :
					vector2i(x - dir.x * r, y - dir.y * r);
				if (IN_FIELD2(new_pos.x, new_pos.y)) {
					u64 single_move = 0;
					BOARD_SET(new_pos.x, new_pos.y, single_move);
					if (single_move & enemy) {
						BOARD_RESET(j, i, dir_open);
						BOARD_SET(new_pos.x, new_pos.y, moves);
					}
					else if (single_move & ally) {
						BOARD_RESET(j, i, dir_open);
					}
					//empty
					else {
						BOARD_SET(new_pos.x, new_pos.y, moves);
					}
				}
			}
		}
	}
	assert(!(moves & ally) && "Sliding piece can take its own piece");
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
