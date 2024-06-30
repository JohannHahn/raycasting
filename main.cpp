#include <iostream>
#include "raylib/src/raylib.h"
#include "raylib/src/raymath.h"

typedef uint64_t u64;

constexpr Vector2 window_size = {900.f, 900.f};
constexpr const char* window_title = "not doom";
constexpr u64 num_cols = 10;
constexpr u64 num_rows = 10;
constexpr float fps = 60.f;
Vector2 player_pos = {window_size.x, window_size.y};
Vector2 player_dir = {1, 1};
float player_speed = fps * 10.f;
float near_plane = 0.5f;

Vector2 map_dims = {window_size.x / 3.f, window_size.y / 3.f};
Color map_bg = DARKGRAY;
Image map_img = GenImageColor(map_dims.x, map_dims.y, map_bg);
Texture map_txt;

constexpr u64 index(u64 x, u64 y) {
    return x + y * num_cols;
}

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
    ImageClearBackground(&map_img, map_bg);
    Vector2 size = {map_dims.x / num_cols, map_dims.y / num_rows};
    for (u64 y = 0; y < num_rows; ++y) {
	float y_scaled = y * size.y;
	float x_inverted_scaled = y * size.x;
	for (u64 x = 0; x < num_cols; ++x) {
	    float x_scaled = x * size.x;
	    ImageDrawLine(&map_img, 0, y_scaled, map_dims.x- 1, y_scaled, RAYWHITE);
	    ImageDrawLine(&map_img, x_inverted_scaled, 0, x_inverted_scaled, map_dims.y - 1, RAYWHITE);
	    if (level[index(x, y)]) {
		ImageDrawRectangleRec(&map_img, {x_scaled, y_scaled, size.x, size.y}, WHITE);
	    }
	}
    }
    ImageDrawCircleV(&map_img, Vector2Multiply(Vector2Divide(player_pos, window_size), map_dims), 10.f, RED);

    UpdateTexture(map_txt, map_img.data); 
    DrawTexture(map_txt, 0, 0, WHITE);
}
// TODO: Left / Right should rotate
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
    InitWindow(window_size.x, window_size.y, window_title);
    SetTargetFPS(fps);
    map_txt = LoadTextureFromImage(map_img);
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(GRAY);
	controls();
	draw_map();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

