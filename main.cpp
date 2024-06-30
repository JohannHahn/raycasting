#include <cstring>
#include <iostream>
#include "raylib/src/raylib.h"
#include "raylib/src/raymath.h"

typedef uint64_t u64;

constexpr float window_width = 600;
constexpr float window_height = 600;
constexpr const char* window_title = "not doom";
constexpr u64 num_cols = 10;
constexpr u64 num_rows = 10;
constexpr float fps = 60.f;

Vector2 player_pos = {window_width, window_height};
Vector2 player_dir = {1, 1};
float player_speed = fps * 10.f;
float near_plane = 0.5f;

bool level[num_rows * num_cols] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

void draw_map(Rectangle boundary) {
    Vector2 grid_dims = {boundary.width / num_cols, boundary.height / num_rows};
    Vector2 player_local_pos = {grid_dims.x * (player_pos.x / boundary.width), grid_dims.y * (player_pos.y / window_height)};
    for (u64 y = 0; y < num_rows; ++y) {
	for (u64 x = 0; x < num_cols; ++x) {
	    DrawLine(boundary.x + x * grid_dims.x, boundary.y + y * grid_dims.y, boundary.width, y * grid_dims.y, RAYWHITE);
	    DrawLine(boundary.x + y * grid_dims.x, boundary.y + x * grid_dims.y, y * grid_dims.x, boundary.height, RAYWHITE);
	}
    }
    Color player_col = RED;
    if (!CheckCollisionPointRec(player_local_pos, boundary)) {
	player_col = ColorAlpha(player_col, 0.2f);
    }
    DrawCircle(boundary.x + player_local_pos.x, boundary.y + player_local_pos.y, 10, RED);
}

void controls() {
    float dt = GetFrameTime();
    if (IsKeyDown(KEY_DOWN)) {
	player_pos.y += player_speed * dt;
    } 
    if (IsKeyDown(KEY_UP)) {
	player_pos.y -= player_speed * dt;
    } 
    if (IsKeyDown(KEY_LEFT)) {
	player_pos.x -= player_speed * dt;
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	player_pos.x += player_speed * dt;
    } 
}

int main() {
    std::cout << "hi\n";
    InitWindow(window_width, window_height, window_title);
    SetTargetFPS(fps);
    
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(GRAY);
	controls();
	draw_map({0, 0, 200, 200});
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

