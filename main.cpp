#include <cstring>
#include <iostream>
#include "raylib/src/raylib.h"

typedef uint64_t u64;

constexpr float window_width = 600;
constexpr float window_height = 600;
constexpr const char* window_title = "not doom";
constexpr u64 num_cols = 10;
constexpr u64 num_rows = 10;

Vector2 player_pos = {0, 0};
Vector2 player_dir = {1, 1};

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

void draw_map() {
    for
}

int main() {
    std::cout << "hi\n";
    InitWindow(window_width, window_height, window_title);

    while(WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RED);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

