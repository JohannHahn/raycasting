#ifndef COMMON_H
#define COMMON_H
#include <inttypes.h>
#include "raylib/raylib/include/raylib.h"
#include "raylib/raylib/include/raymath.h"

typedef uint64_t u64;
typedef uint32_t u32;

static constexpr long num_cols = 10;
static constexpr long num_rows = 10;
static constexpr const char* window_title = "not doom";
static constexpr float fps = 60.f;
static constexpr float map_factor = 3.f;
static constexpr float epsilon = 0.00001f;
static constexpr float max_depth = 999999.f;
static constexpr Vector2 screen_size = {600.f, 600.f}; 


enum side_kind {
    UNUSED, X, Y, PARALLEL 
};

enum wall_tex {
    EMPTY, FLAT, STONE_WALL, JOHANNDER, WALL_TEX_MAX
};

struct Wall {
    wall_tex tex;
    Image* decal;
    Rectangle decal_rec;
};

struct Player {
    Vector2 position;
    Vector2 direction;
    float look_vert = 0.f;
    float speed = 5.f;
    float size = 0.4f;
    float rotation_speed = PI * 2.f;
    float near_plane = 0.2f;
    float far_plane;
    void change_dir(int sign) {
	if (sign < 0) sign = -1;
	if (sign > 0) sign = 1;
	direction = Vector2Normalize(Vector2Rotate(direction, sign * rotation_speed * GetFrameTime()));
    }
    Color color = RED;
    Vector2 fov_left() const {
	return Vector2Subtract(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2.f));
    }
    Vector2 fov_right() const {
	return Vector2Add(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2.f));
    }
};

struct Context {
    Vector2 window_size = {1200.f, 1200.f}; 
    Rectangle map_boundary = {0.f, 0.f, screen_size.x / map_factor, screen_size.y / map_factor};
    Rectangle game_boundary = {0.f, 0.f, screen_size.x, screen_size.y};
    Color bg_color = SKYBLUE;
    Color map_bg = GRAY;
    Color floor_col = DARKGRAY;
    Vector2 light_pos = {num_cols / 2.f, num_rows / 2.f};
    float depth_buffer[(u64)screen_size.x * (u64)screen_size.y] = {num_cols * num_rows};
    bool debug_print = false;
    Player player;
    wall_tex level[num_rows * num_cols] = {
	FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,
	FLAT, EMPTY, EMPTY, EMPTY, EMPTY, JOHANNDER, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, EMPTY, EMPTY, EMPTY, EMPTY, JOHANNDER, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, JOHANNDER, JOHANNDER, JOHANNDER, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, STONE_WALL, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
	FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
	EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    };
    Image images[WALL_TEX_MAX];
};

bool float_equal(float a, float b);
bool color_equal(Color c1, Color c2);
u64 index(u64 x, u64 y, u64 width);
Color u32_to_col(u32 i);
bool inside_wall(Vector2 p);
Vector2 next_point(Vector2 p, Vector2 p2, int& out);

#ifdef COMMON_IMPL
bool float_equal(float a, float b) {
    return std::abs(a - b) < epsilon;
}

bool color_equal(Color c1, Color c2) {
    return (c1.a == c2.a && c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}

u64 index(u64 x, u64 y, u64 width) {
    return x + y * width;
}

Color u32_to_col(u32 i) {
    return *(Color*)(&i);
} 

Vector2 next_point(Vector2 p, Vector2 p2, int& out) {
    Vector2 p_next = {0,0};
    Vector2 dir = Vector2Subtract(p2, p);
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
		out = Y;
		return second_candidate;
	    }
	}
	out = X;
	return p_next;
    }
    p_next.x = p2.x;
    p_next.y = snap(p2.y, dir.y);
    out = PARALLEL;
    return p_next;
}



bool inside_wall(Vector2 p) {
    return level[index(p.x, p.y, num_cols)];
}

#endif //COMMON_IMPL

#endif //COMMON_HPP
