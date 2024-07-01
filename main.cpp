#include <iostream>
#include <inttypes.h>
#include "raylib/src/raylib.h"
#include "raylib/src/raymath.h"

typedef uint64_t u64;

constexpr Vector2 window_size = {900.f, 900.f};
constexpr const char* window_title = "not doom";
constexpr u64 num_cols = 10;
constexpr u64 num_rows = 10;
constexpr float fps = 60.f;
Vector2 map_dims = {window_size.x / 3.f, window_size.y / 3.f};
Color map_bg = DARKGRAY;
Image map_img = GenImageColor(map_dims.x, map_dims.y, map_bg);
Texture map_txt;

Vector2 to_map(Vector2 v) {
    return Vector2Multiply(Vector2Divide(v, window_size), map_dims);
}


struct Player {
    Vector2 position = {window_size.x / 2, window_size.y / 2};
    Vector2 direction = {0, 1};
    float speed = fps * 10.f;
    float near_plane = 100.f;
    void change_dir(float angle) {
	direction = Vector2Scale(Vector2Normalize(Vector2Rotate(direction, angle)), near_plane);
    }
    Vector2 position_map() {
	return to_map(position);
    }
    Vector2 direction_map() {
	return to_map(direction);
    }
    Vector2 fov_left() {
	return 
    }
};
Player player;


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
    ImageDrawCircleV(&map_img, player.position_map(), 10.f, RED);
    ImageDrawLineEx(&map_img, player.position_map(), Vector2Add(player.position_map(), player.direction_map()), 5, RED);

    UpdateTexture(map_txt, map_img.data); 
    DrawTexture(map_txt, 0, 0, WHITE);
}
void controls() {
    float dt = GetFrameTime();
    if (IsKeyDown(KEY_DOWN)) {
	player.position.y += player.speed * dt;
    } 
    if (IsKeyDown(KEY_UP)) {
	player.position.y -= player.speed * dt;
    } 
    if (IsKeyDown(KEY_LEFT)) {
	player.position.x -= player.speed * dt;
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	player.position.x += player.speed * dt;
    } 
}

int main() {
    InitWindow(window_size.x, window_size.y, window_title);
    SetTargetFPS(fps);
    player.change_dir(3.14f);
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

