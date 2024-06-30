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
constexpr Vector2 grid_dims = {window_width / num_cols, window_height / num_rows};
constexpr float fps = 60.f;

Vector2 player_pos = {0, 0};
Vector2 player_dir = {1, 1};
float player_speed = fps * 100.f;
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
    for (u64 y = 0; y < num_rows; ++y) {
	for (u64 x = 0; x < num_cols; ++x) {
	    DrawLine(boundary.x + x * grid_dims.x, boundary.y + y * grid_dims.y, boundary.width, y * grid_dims.y, RAYWHITE);
	    DrawLine(boundary.x + y * grid_dims.x, boundary.y + x * grid_dims.y, y * grid_dims.x, boundary.height, RAYWHITE);
	}
    }
    DrawCircle(boundary.x + player_pos.x / grid_dims.x, boundary.y + player_pos.y / grid_dims.y, 10, RED);
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
	draw_map({0, 0, 100, 100});
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

