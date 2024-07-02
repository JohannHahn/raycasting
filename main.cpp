#include <cassert>
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
constexpr float map_factor = 1.f;
Rectangle map_boundary = {0.f, 0.f, window_size.x / map_factor, window_size.y / map_factor};
Color map_bg = DARKGRAY;
Image map_img = GenImageColor(map_boundary.width, map_boundary.height, map_bg);
Texture map_txt;

struct Player {
    Vector2 position = {num_cols / 2.f, num_rows/ 2.f};
    Vector2 direction = {0.f, 1.f};
    float speed = 0.1f;
    float rotation_speed = PI / fps;
    float near_plane = 0.2f;
    void change_dir(float angle) {
	direction = Vector2Normalize(Vector2Rotate(direction, angle * rotation_speed));
    }
    Color color = RED;
    float size = 0.1f;
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

float snap(float n, float dn) {
    if(dn > 0) return std::ceil(n);
    if(dn < 0) return std::floor(n);
    return n;
}

Vector2 next_point(Vector2 p, Vector2 dir) {
    Vector2 p_next = {0,0};
    Vector2 p2 = Vector2Add(p, dir);
    //y1 = mx1 + c
    //y2 = mx2 + c
    //c = y1 - mx1
    //m = dy / dx
    //x = y2-c/m
    if (dir.x != 0.f) {
	float m = dir.y / dir.x;
	float c = p.y - (p.x * m);
	p_next.x = snap(p2.x, dir.x);
	p_next.y = p_next.x * m + c;
	Vector2 second_candidate;
	std::cout << "m = " << m << ", c = " << c << "\n";
	if (m != 0.f) {
	    second_candidate.y = snap(p2.y, dir.y);
	    second_candidate.x = (second_candidate.y - c) / m;
	    float first_distance = Vector2Length(Vector2Subtract(p_next, p2));
	    float second_distance = Vector2Length(Vector2Subtract(second_candidate, p2));
	    if (second_distance < first_distance) {
		return second_candidate;
	    }
	}
	return p_next;
    }
    assert(0 && "dir.x = 0 not handled lol");
    p_next.x = p2.x;
    p_next.y = snap(p2.y, dir.y);
    return p_next;
}

void draw_map(Rectangle boundary) {
    ImageClearBackground(&map_img, map_bg);
    Vector2 size = {boundary.width / num_cols, boundary.height / num_rows};
    for (u64 x = 0; x < num_cols; ++x) {
	ImageDrawLine(&map_img, x * size.x, 0, x*size.x, num_rows * size.y, RAYWHITE);
    }
    for (u64 y = 0; y < num_rows; ++y) {
	ImageDrawLine(&map_img, 0, y * size.y, num_cols * size.x, y * size.y, RAYWHITE);
    }
    for(u64 y = 0; y < num_rows; ++y) {
	for(u64 x = 0; x < num_cols; ++x) {
	    if(level[index(x, y)]) {
		ImageDrawRectangleV(&map_img, {x * size.x, y * size.y}, size, WHITE);
	    }
	}
    }
    Vector2 player_map = Vector2Multiply(player.position, size);
    Vector2 p2_map = Vector2Multiply(Vector2Add(player.position, Vector2Scale(player.direction, player.near_plane)), size);
    ImageDrawCircleV(&map_img, player_map, player.size * size.x, player.color);
    ImageDrawLineV(&map_img, player_map, p2_map, player.color);
    ImageDrawCircleV(&map_img, p2_map, player.size * size.x / 2.f, player.color);


    UpdateTexture(map_txt, map_img.data); 
    DrawTexturePro(map_txt, map_boundary, map_boundary, {0.f, 0.f}, 0.f, WHITE);
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
    Vector2 size = {player.near_plane * 2.f / window_size.x, player.near_plane * 2.f / window_size.y};
    for(u64 x = 0; x < window_size.x; ++x) {
	Vector2 next = next_point(player.position, Vector2Scale(player.direction, player.near_plane));
	u64 lx = std::floor(next.x);
	u64 ly = std::floor(next.y);
    }
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
	draw_map(map_boundary);
	DrawFPS(0, 0);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

