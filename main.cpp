#include <iostream>
#include "raylib.h"
#include "chess.h"

float window_width = 900;
float window_height = 600;
Texture grid_tex;
Texture pieces_tex[pieces_max];
Texture select_tex;
Rectangle board_area;
Rectangle move_highlights[BOARD_DIM * BOARD_DIM] = {0};
const char* assets_path = "assets\\";
const char* img_path = "assets\\img\\";
float cell_size;
Color dark = BROWN;
Color light = GRAY;
Color highlight_col = DARKGRAY;
chess game;

vector2f draggie;
bool dragging = 0;

void load_assets() 
{
	std::string path = img_path;
	for (int i = 0; i < pieces_max; ++i) {
		path += i < PIECE_TYPES ? "b_" : "w_";
		int rel_i = i % PIECE_TYPES;
		switch (rel_i) {
		case 0:
			path += "pawn.png";
			break;
		case 1:
			path += "knight.png";
			break;
		case 2:
			path += "bishop.png";
			break;
		case 3:
			path += "rook.png";
			break;
		case 4:
			path += "king.png";
			break;
		case 5:
			path += "queen.png";
			break;
		default:
			break;
		}
		pieces_tex[i] = LoadTexture(path.c_str());
		SetTextureFilter(pieces_tex[i], TEXTURE_FILTER_POINT);
		path = img_path;
	}
	Image img = GenImageColor(10, 10, highlight_col);
	select_tex = LoadTextureFromImage(img);
}
Image draw_grid(int width, int height) 
{
	int side = std::min(width, height) / BOARD_DIM;
	Image grid = GenImageColor(width, height, dark);
	for (int y = 0; y < BOARD_DIM; ++y) {
		for (int x = 0; x < BOARD_DIM; ++x) {
			Color c = (x + y) % 2 == 0 ? light: dark;
			ImageDrawRectangle(&grid, side * x, side * y, side, side, c);
		}
	}
	return grid;
}
void draw_piece(size_t tex, Rectangle dst) 
{
	Rectangle org = Rectangle(0, 0, (float)pieces_tex[tex].width, (float)pieces_tex[tex].height);
	DrawTexturePro(pieces_tex[tex], org, dst, Vector2(0, 0), 0.f, WHITE);
}

//CAN BE OPTIMISED ALOT
void draw_board(Rectangle board_area)
{
	DrawTexture(grid_tex, 0, 0, WHITE);
	u32 counter = 0;
	for (u32 y = 0; y < BOARD_DIM; ++y) {
		for (u32 x = 0; x < BOARD_DIM; ++x) {
			int tex = -1;
			for (int i = 0; i < pieces_max; ++i) {
				if (BOARD_AT(x, y, game.pieces[i])) {
					tex = i;
				}
			}
			float small_cell = cell_size * 0.8f;
			float cell_diff = cell_size - small_cell;
			vector2f screen_coords = game.translate_coords(vector2f(x, y));
			Rectangle dst = Rectangle(
				screen_coords.x * cell_size + cell_diff / 2.f, 
				screen_coords.y * cell_size + cell_diff / 2.f, 
				small_cell, small_cell);
			if (dragging && game.selected_cell.x == x && game.selected_cell.y == y) {
				vector2f mouse_coords = game.translate_coords(draggie);
				dst.x = mouse_coords.x * cell_size + cell_diff / 2.f - cell_size / 2.f;
				dst.y = mouse_coords.y * cell_size + cell_diff / 2.f - cell_size / 2.f;
			}
			if (tex >= 0) {
				draw_piece(tex, dst);
			}
			if (game.selected && BOARD_AT(x, y, game.current_moves))
			{
				Rectangle dst =	Rectangle(screen_coords.x * cell_size + cell_size / 2.f, screen_coords.y * cell_size + cell_size / 2.f, cell_size / 10.f, cell_size / 10.f);
				move_highlights[counter++] = dst;
			}
			if (BOARD_AT(x, y, game.en_passant)) {
				DrawCircle(x, y, 100, WHITE);
			}
		}
	}
	for (u32 i = 0; i < counter; ++i) {
		DrawTexturePro(select_tex, Rectangle(0, 0, (float)select_tex.width, (float)select_tex.height), move_highlights[i], {0,0}, 0, WHITE);
	}
}

void resize() 
{
	window_width = (float)GetScreenWidth();
	window_height = (float)GetScreenHeight();
	float side = std::min(window_width, window_height);
	board_area = Rectangle(0, 0, side, side);
	cell_size = side / (float)BOARD_DIM;
	Image img = draw_grid((int)window_width, (int)window_height);
	UnloadTexture(grid_tex);
	grid_tex = LoadTextureFromImage(img);
	UnloadImage(img);
}

void controls()
{
	Vector2 mouse_pos = GetMousePosition();
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse_pos, board_area)) {
		float x = std::floor(mouse_pos.x / cell_size);
		float y = std::floor(mouse_pos.y / cell_size);
		vector2 game_coords = game.translate_coords(vector2(x,y));
		int start = (game.player ? 0 : PIECE_TYPES);
		for (int i = start; i < start + PIECE_TYPES; ++i) {
			if (BOARD_AT(game_coords.x, game_coords.y, game.pieces[i])) {
				game.dragging_map = i;
				draggie = game_coords;
				game.current_moves = game.legal_moves(game_coords.x, game_coords.y, i);
				game.selected_cell = game_coords;
				game.selected = true;
				dragging = true;
				break;
			}
		}
	}
	if (dragging && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
		vector2 dst = game.board_flip ? 
			vector2(std::ceil(draggie.x), std::floor(draggie.y)) :
			vector2(std::floor(draggie.x), std::ceil(draggie.y));
		u64 move = 0;
		BOARD_SET((u64)dst.x, (u64)dst.y, move);
		move &= game.current_moves;
		if (move && 
			(game.selected_cell.x != dst.x || game.selected_cell.y != dst.y) && 
			CheckCollisionPointRec(mouse_pos, board_area)) {
			
			game.move(game.selected_cell, dst, game.pieces[game.dragging_map]);
		}
		dragging = false;
		game.selected = false;
		game.current_moves = 0;
	}
	if (dragging && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
		float x = mouse_pos.x / cell_size;
		float y = mouse_pos.y / cell_size;
		draggie = game.translate_coords(vector2f(x, y));
	}
}

int main()
{
	InitWindow((int)window_width, (int)window_height, "TestWindow");
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(200);
	resize();
	load_assets();
	game.init_board();
	while (!WindowShouldClose()) {
		controls();
		ClearBackground(BLACK);
		if (IsWindowResized()) {
			resize();
		}
		BeginDrawing();
		draw_board(board_area);
		DrawFPS((int)board_area.width + 10, 10);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}