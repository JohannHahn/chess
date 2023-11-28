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

void chess::scan_board(bool player)
{
	u32 start = (player == white ? PIECE_TYPES : 0);
	u64 ally = player == white ? white_pieces : black_pieces;
	u64 king = player == white ? pieces[w_king] : pieces[b_king];
	u64* player_sees = player == white ? &white_sees : &black_sees;
	white_sees = 0;
	black_sees = 0;
	sees_king = 0;
	for (u32 y = 0; y < BOARD_DIM; ++y) {
		for (u32 x = 0; x < BOARD_DIM; ++x) {
			for (u32 i = start; i < start + PIECE_TYPES; ++i) {
				if (BOARD_AT(x, y, pieces[i])) {
					move m = legal_moves(x, y, i);
					*player_sees |= m.attacks;
					if (king & m.attacks) {
						sees_king |= m.attacks;
					}
				}
			}
		}
	}
	sees_king &= ~(king);
}

bool chess::check_for_check() 
{
	u64 king = player == white ? pieces[w_king] : pieces[b_king];
	u64 enemy_sees = player == white ? black_sees : white_sees;
	return king & enemy_sees;
}
void chess::make_move(move move) 
{
	u64 take = move.dst;
	if (move.dst & last_move.en_passant_attack) {
		take = last_move.dst;
	}
	for (int i = 0; i < pieces_max; ++i) {
		if (pieces[i] & take) { 
			pieces[i] &= ~(take);
			taken_pieces.push_back(i);
		}
	}

	for (int i = 0; i < pieces_max; ++i) {
		if(move.org & pieces[i]) {
			pieces[i] &= ~(move.org);
			pieces[i] |= move.dst;
			break;
		}
	}
	//setup for next move
	white_pieces = get_piece_mask(white);
	black_pieces = get_piece_mask(black);
	assert(!(white_pieces & black_pieces) && "pieces overlap");
	all = white_pieces | black_pieces;
	empty = ~(all);
	last_move = move;
	all_moves.push_back(move);
	move_cache.clear();
	scan_board(player);
	player = !player;
	in_check = check_for_check();
	if (in_check) {
		u64 king = player == white ? pieces[w_king] : pieces[b_king];
		for (auto& p : move_cache) {
			if (BOARD_AT_I(p.first, king)) {
				p.second.dst &= ~(sees_king);
			}
			p.second.dst &= sees_king;
		}
	}
	scan_board(player);
}
chess::move chess::legal_moves(u32 x, u32 y, int type)
{
	move moves = { 0 };
	if (!move_cache.contains(INDEX(x, y))) {

		int type_reduced = type % PIECE_TYPES;
		if (type_reduced == pawn) {
			moves = pawn_moves(x, y);
		}
		else if (type_reduced == knight) {
			moves = knight_moves(x, y);
		}
		else {
			moves = sliding_piece(x, y, type_reduced);
		}
		if (in_check) {
		}
		move_cache[INDEX(x, y)] = moves;
	}
	return move_cache[INDEX(x, y)];
}

chess::move chess::pawn_moves(u32 x, u32 y)
{
	move moves = { 0 };
	BOARD_SET(x, y, moves.org);
	int dir = player == white ? +1 : -1;
	u64 attacks = pawn_attacks(x, y, moves);
	if(IN_FIELD(y + dir)) BOARD_SET(x, y + dir, moves.dst);
	moves.dst &= empty;
	if (moves.dst && ((player == white && y == 1) || (player != white && y == BOARD_DIM - 2))) {
		if (IN_FIELD(y + dir * 2)) {
			BOARD_SET(x, y + dir * 2, moves.dst);
			moves.dst &= empty;
			if (BOARD_AT(x, y + dir * 2, moves.dst)) {
				BOARD_SET(x, y + dir, moves.en_passant_attack);
				BOARD_SET(x, y + dir * 2, moves.pawn_double_jump);
			}
		}
	}
	moves.dst |= attacks;
	return moves;
}
u64 chess::pawn_attacks(u32 x, u32 y, move& move)
{
	u64 attacks = 0;
	int dir = player == white ? +1 : -1;
	if (IN_FIELD(y + dir)) {
		if (IN_FIELD(x + 1)) BOARD_SET(x + 1, y + dir, attacks);
		if (IN_FIELD(x - 1)) BOARD_SET(x - 1, y + dir, attacks);
	}
	move.attacks = attacks;
	attacks &= player == white ? black_pieces : white_pieces;
	if (last_move.pawn_double_jump & last_move.dst) {
		u64 t1 = 0;
		u64 t2 = 0;
		u64 y_passant_possible = player == white ? 4 : 3;
		BOARD_SET(x + 1, y, t1);
		if (x > 1) {
			BOARD_SET(x - 1, y, t2);
		}
		if (y == y_passant_possible && (last_move.dst & t1) || 
			last_move.dst & t2) {
			attacks |= last_move.en_passant_attack;
		}
	}
	return attacks;
}

chess::move chess::knight_moves(u32 x, u32 y)
{
	move moves = { 0 };
	BOARD_SET(x, y, moves.org);
	int new_x, new_y;
	for (u32 i = 0; i < 4; ++i) {
		int main_dir = i % 2 == 0 ? 2 : -2;
		for (u32 j = 0; j < 2; ++j) {
			int sec_dir = j == 0 ? 1 : -1;
			new_x = i < 2 ? x + sec_dir : x + main_dir;
			new_y = i < 2 ? y + main_dir: y + sec_dir;
			if (IN_FIELD2(new_x, new_y)) {
				BOARD_SET(new_x, new_y, moves.dst);
			}
		}
	}
	moves.dst &= ~(player == white ? white_pieces : black_pieces);
	moves.attacks = moves.dst;
	return moves;
}
chess::move chess::sliding_piece(u32 x, u32 y, int type)
{
	move moves = { 0 };
	BOARD_SET(x, y, moves.org);
	u32 num_dirs = 2;
	u32 start = 0;
	u32 range = 8;
	u64 dir_open = SIZE_MAX;
	u64 enemy = player == white ? black_pieces : white_pieces;
	u64 ally = player == white ? white_pieces: black_pieces;
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
						BOARD_SET(new_pos.x, new_pos.y, moves.dst);
					}
					else if (single_move & ally) {
						BOARD_RESET(j, i, dir_open);
					}
					//empty
					else {
						BOARD_SET(new_pos.x, new_pos.y, moves.dst);
					}
				}
			}
		}
	}
	assert(!(moves.dst & ally) && "Sliding piece can take its own piece");
	moves.attacks = moves.dst;
	return moves;
}
u64 chess::get_piece_mask(bool color)
{
	u64 result = 0;
	int start = color == white ? PIECE_TYPES : 0;
	for (int i = start; i < start + PIECE_TYPES; ++i) {
		result |= pieces[i];
	}
	return result;
}
