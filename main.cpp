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
constexpr float map_factor = 3.f;
Rectangle map_boundary = {0.f, 0.f, window_size.x / map_factor, window_size.y / map_factor};
Color map_bg = DARKGRAY;
Image map_img = GenImageColor(map_boundary.width, map_boundary.height, map_bg);
Texture map_txt;
Vector2 to_screen = {window_size. x / num_cols, window_size.y / num_rows};


struct Player {
    Vector2 position = {num_cols / 2.f, num_rows/ 2.f};
    Vector2 direction = {0.f, 1.f};
    float speed = 0.1f;
    float rotation_speed = PI / fps;
    float near_plane = 0.5f;
    float far_plane = 10.f;
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

Rectangle squish_rec(Rectangle r, float factor) {
    float a = r.height * (1.f - factor) / 2.f;
    return {r.x, r.y + a, r.width, r.height * factor};
}

Vector2 to_map(Vector2 v) {
    return Vector2Multiply(v, {map_boundary.width / num_cols, map_boundary.height / num_rows});
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
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
	//player.position.y += player.speed * dt;
	player.position = Vector2Subtract(player.position, Vector2Scale(player.direction, player.speed));
    } 
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
	//player.position.y -= player.speed * dt;
	player.position = Vector2Add(player.position, Vector2Scale(player.direction, player.speed));
    } 
    if (IsKeyDown(KEY_A)) {
	//player.position.y -= player.speed * dt;
	player.position = Vector2Add(player.position, Vector2Scale(Vector2Rotate(player.direction, -PI/2.f), player.speed));
    } 
    if (IsKeyDown(KEY_D)) {
	//player.position.y -= player.speed * dt;
	player.position = Vector2Add(player.position, Vector2Scale(Vector2Rotate(player.direction, PI/2.f), player.speed));
    } 
    if (IsKeyDown(KEY_LEFT)) {
	player.change_dir(-1);
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	player.change_dir(1);
    } 
}


void draw_walls() {
    Vector2 cell = player.position;
    Vector2 p2 = Vector2Add(player.position, Vector2Scale(player.direction, player.near_plane));
    Vector2 left = Vector2Add(p2, Vector2Rotate(Vector2Scale(player.direction, player.near_plane), -PI / 2.f));
    Vector2 right = Vector2Add(p2, Vector2Rotate(Vector2Scale(player.direction, player.near_plane), PI / 2.f));
    Vector2 left_to_right = Vector2Scale(Vector2Normalize(Vector2Subtract(right, left)), player.near_plane * 2.f / window_size.x);
    Vector2 direction = Vector2Normalize(Vector2Subtract(left, player.position));
    Vector2 prev = cell;
    DrawLineV(to_map(player.position), to_map(left), BLUE);
    DrawLineV(to_map(player.position), to_map(right), BLUE);
    for(u64 x = 0; x < window_size.x; ++x) {
	cell = player.position;
	for (u64 depth = 0; depth < player.far_plane; ++depth) {
	    u64 lx = std::ceil(cell.x);
	    u64 ly = std::ceil(cell.y);
	    u64 lx2 = std::floor(cell.x);
	    u64 ly2 = std::floor(cell.y);
	    if (lx < num_cols && ly < num_rows) {
		float distance = Vector2Length(Vector2Subtract(player.position, cell));
		prev = cell;	
		cell = next_point(cell, Vector2Scale(direction, player.near_plane));
		if(level[index(lx, ly)]) {
		    DrawRectangleRec(squish_rec({(float)x, 0, 1, window_size.y}, 1.f / distance), WHITE);
		    DrawLineV(to_map(prev), to_map(cell), BLUE);
		}
		if(level[index(lx2, ly2)]) {
		    DrawRectangleRec(squish_rec({(float)x, 0, 1, window_size.y}, 1.f / distance), WHITE);
		    DrawLineV(to_map(prev), to_map(cell), BLUE);
		}
	    }
	}
	left = Vector2Add(left, left_to_right);
	direction = Vector2Normalize(Vector2Subtract(left, player.position));
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
	draw_walls();
	DrawFPS(0, 0);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

