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

// TODO: player position in normalized coordinates
struct Player {
    Vector2 position = {window_size.x / 2, window_size.y / 2};
    Vector2 direction = {0, 1};
    float speed = 2;
    float rotation_speed = PI / fps;
    float near_plane = 100.f;
    void change_dir(float angle) {
	direction = Vector2Normalize(Vector2Rotate(direction, angle * rotation_speed));
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

Vector2 next_point(Vector2 p, Vector2 dir) {
    Vector2 p_next;
    if (dir.x > 0) {
	p_next.x = std::ceil(p.x + dir.x);
    }
    else if (dir.x < 0) {
	//p_next.x = std::floor(p.x + dir.x);
    }
    return p_next;
}

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
    Vector2 player_map = to_map(player.position);
    ImageDrawCircleV(&map_img, player_map, 10.f, RED);
    Vector2 end = to_map(Vector2Add(player.position, Vector2Scale(player.direction, player.near_plane)));
    ImageDrawLineEx(&map_img, player_map, end, 5, RED);
    ImageDrawCircleV(&map_img,end, 10, BLUE);
    Vector2 p_next = next_point(end, to_map(player.direction));
    ImageDrawLineEx(&map_img, end, p_next, 5, RED);
    ImageDrawCircleV(&map_img, p_next, 10, GREEN);

    UpdateTexture(map_txt, map_img.data); 
    DrawTexture(map_txt, 0, 0, WHITE);
}
void controls() {
    float dt = GetFrameTime();
    if (IsKeyDown(KEY_DOWN)) {
	//player.position.y += player.speed * dt;
	player.position = Vector2Subtract(player.position, Vector2Scale(player.direction, player.speed));
    } 
    if (IsKeyDown(KEY_UP)) {
	//player.position.y -= player.speed * dt;
	player.position = Vector2Add(player.position, Vector2Scale(player.direction, player.speed));
    } 
    if (IsKeyDown(KEY_LEFT)) {
	player.change_dir(-1);
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	player.change_dir(1);
    } 
}

void draw_walls() {

}

int main() {
    InitWindow(window_size.x, window_size.y, window_title);
    SetTargetFPS(fps);
    player.change_dir(3.14f);
    map_txt = LoadTextureFromImage(map_img);
    while(!WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(BLACK);
	controls();
	draw_map();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

