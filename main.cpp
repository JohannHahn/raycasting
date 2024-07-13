#include <cassert>
#include <iostream>
#include <inttypes.h>
#include "raylib/src/raylib.h"
#include "raylib/src/raymath.h"

typedef uint64_t u64;
typedef uint32_t u32;

constexpr Vector2 window_size = {900.f, 900.f}; constexpr const char* window_title = "not doom";
constexpr u64 num_cols = 10;
constexpr u64 num_rows = 10;
constexpr float fps = 60.f;
constexpr float map_factor = 2.f;
constexpr float epsilon = 0.00001f;
Rectangle map_boundary = {0.f, 0.f, window_size.x / map_factor, window_size.y / map_factor};
Rectangle game_boundary = {0.f, 0.f, window_size.x, window_size.y};
Color bg_color = SKYBLUE;
Color map_bg = GRAY;
Color floor_col = DARKGRAY;
Image map_img = GenImageColor(map_boundary.width, map_boundary.height, map_bg);
Texture map_txt;
Image game_img = GenImageColor(window_size.x, window_size.y, bg_color);
Texture game_tex;
Vector2 to_screen = {window_size. x / num_cols, window_size.y / num_rows};
const char* wall_tex_path = "tileable10d.png";
const char* johannder_path = "johannder.png";
Image stone_wall_img = LoadImage(wall_tex_path);
Image johannder_img = LoadImage(johannder_path);
Texture stone_wall_tex;
Texture johannder_tex;
Vector2 light_pos = {0.f, 0.f};
bool debug_map = false;

enum side_kind {
    UNUSED, X, Y, PARALLEL 
};

enum wall_tex {
    EMPTY, FLAT, STONE_WALL, JOHANNDER,
};

struct Point{
    u64 x;
    u64 y;
};

Color colors[] = {BLACK, RED, GREEN, BLUE};

constexpr bool float_equal(float a, float b) {
    return std::abs(a - b) < epsilon;
}

struct Player {
    Vector2 position = {num_cols / 2.f, num_rows/ 2.f};
    Vector2 direction = {0.f, 1.f};
    float speed = 2.f;
    float rotation_speed = PI;
    float near_plane = .1f;
    float far_plane = num_cols * 2;
    void change_dir(int sign) {
	if (sign < 0) sign = -1;
	if (sign > 0) sign = 1;
	direction = Vector2Normalize(Vector2Rotate(direction, sign * rotation_speed * GetFrameTime()));
    }
    Color color = RED;
    float size = 0.4f;
    Vector2 fov_left() {
	return Vector2Subtract(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2));
    }
    Vector2 fov_right() {
	return Vector2Add(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2));
    }
};
Player player;


constexpr u64 index(u64 x, u64 y) {
    return x + y * num_cols;
}

void color_brightness(Color& c, float scale) {
    scale = Clamp(scale, 0.f, 1.f);
    c.r *= scale;
    c.g *= scale;
    c.b *= scale;
}

Rectangle squish_rec(Rectangle r, float factor) {
    float h = r.height * factor;
    float y = (r.height - h) * 0.5f;
    return {r.x, y, r.width, h};
}

Vector2 to_map(Vector2 v) {
    return Vector2Multiply(v, {map_boundary.width / num_cols, map_boundary.height / num_rows});
}

wall_tex level[num_rows * num_cols] = {
    FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,
    FLAT, EMPTY, EMPTY, EMPTY, EMPTY,	    JOHANNDER, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, EMPTY, EMPTY, EMPTY, EMPTY,	    JOHANNDER, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, JOHANNDER, JOHANNDER, JOHANNDER, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, STONE_WALL, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,
};

float snap(float n, float dn) {
    
    if(dn > 0) {
	return std::ceil(n + epsilon);
    }

    if(dn < 0) { 
	return std::floor(n - epsilon);
    }
    return n;
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


void draw_map(Rectangle boundary) {
    ImageClearBackground(&map_img, map_bg);
    Vector2 size = {boundary.width / num_cols, boundary.height / num_rows};
    for (u64 x = 0; x < num_cols; ++x) {
	ImageDrawLine(&map_img, x * size.x, 0, x*size.x, num_rows * size.y, ColorAlpha(RAYWHITE, 0.5f));
    }
    for (u64 y = 0; y < num_rows; ++y) {
	ImageDrawLine(&map_img, 0, y * size.y, num_cols * size.x, y * size.y, ColorAlpha(RAYWHITE, 0.5f));
    }
    for(u64 y = 0; y < num_rows; ++y) {
	for(u64 x = 0; x < num_cols; ++x) {
	    if(level[index(x, y)]) {
		ImageDrawRectangleV(&map_img, {x * size.x, y * size.y}, size, ColorAlpha(WHITE, 0.5f));
	    }
	}
    }
    Vector2 player_map = Vector2Multiply(player.position, size);
    Vector2 p2 = Vector2Add(player.position, Vector2Scale(player.direction, player.near_plane));
    Vector2 p2_map = Vector2Multiply(p2, size);
    ImageDrawCircleV(&map_img, player_map, 2, player.color);
    ImageDrawLineV(&map_img, player_map, p2_map, player.color);
    ImageDrawCircleV(&map_img, p2_map, 2, player.color);
    ImageDrawLineV(&map_img, player_map, to_map(player.fov_left()), BLUE);
    ImageDrawLineV(&map_img, player_map, to_map(player.fov_right()), GREEN);
    ImageDrawLineV(&map_img, to_map(player.fov_right()), to_map(player.fov_left()), RED);

    if (debug_map) {
	Vector2 prev = player.position;
	Vector2 next = p2;
	Vector2 ray_dir = Vector2Normalize(Vector2Subtract(p2, player.position));
	Vector2 left = player.fov_left();
	Vector2 right = player.fov_right();
	Vector2 left_right = Vector2Subtract(right, left);
	u64 step_max = 100;
	for (int step = 0; step < step_max; ++step) {
	    prev = player.position;
	    ray_dir = Vector2Subtract(left, player.position);
	    next = Vector2Add(player.position, ray_dir);
	    for(int i = 0; i < player.far_plane; ++i) {
		int kind = 0;
		Vector2 p = next_point(prev, next, kind);
		Color c = colors[kind];
		prev = next;
		next = p;
		ImageDrawLineV(&map_img, to_map(prev), to_map(next), c);
		Vector2 cell = Vector2Add(next, Vector2Scale(ray_dir, epsilon));
		cell.x = floor(cell.x);
		cell.y = floor(cell.y);
		Vector2 cell_map = to_map(cell);
		Rectangle r = {cell_map.x, cell_map.y, size.x, size.y};
		ImageDrawRectangleLines(&map_img, r, 2, RED);
		ImageDrawLineV(&map_img, to_map(next), cell_map, YELLOW);
		if (level[index(cell.x, cell.y)]) {
		    ImageDrawCircleV(&map_img, to_map(next), 10, MAGENTA);
		    ImageDrawCircleV(&map_img, to_map({(float)cell.x, (float)cell.y}), 10, RED);
		    break;
		}
		else {
		    ImageDrawCircleV(&map_img, to_map(next), 3, WHITE);
		}
	    }
	    left = Vector2Add(left, Vector2Scale(left_right, 1.f / step_max));
	}
    }
}

bool inside_wall(Vector2 p) {
    return level[index(p.x, p.y)];
}

void controls() {
    float dt = GetFrameTime();
    Vector2 new_pos = player.position;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
	new_pos = Vector2Subtract(player.position, Vector2Scale(player.direction, player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(player.direction, player.size)))) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
	new_pos = Vector2Add(player.position, Vector2Scale(player.direction, player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(player.direction, player.size)))) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_A)) {
	new_pos = Vector2Add(player.position, Vector2Scale(Vector2Rotate(player.direction, -PI/2.f), player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(player.direction, player.size)))) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_D)) {
	new_pos = Vector2Add(player.position, Vector2Scale(Vector2Rotate(player.direction, PI/2.f), player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(player.direction, player.size)))) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_LEFT)) {
	player.change_dir(-1);
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	player.change_dir(1);
    } 
    //player.position = new_pos;
}

void draw_strip_flat(u64 x, float scale, Color c) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 2.f, window_size.y - 1}, scale);
    color_brightness(c, scale / 2.f);
    ImageDrawRectangleRec(&game_img, strip, c);
}

void draw_strip(u64 x, u64 u, float scale, Image img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 2.f, window_size.y - 1}, scale);
    if (u > img.width && "!") return;
    float v = 0.f;
    for(u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v < img.height) {
	    u32 pixel_col = ((u32*)img.data)[u + (u64)v * img.width];
	    Color* col = (Color*)(&pixel_col);
	    color_brightness(*col, scale * 2.f);
	    ImageDrawPixel(&game_img, x, y, *col);
	}
	v += 1.f / strip.height * img.height;
    }
}


void draw_walls2(Rectangle boundary) {
    Vector2 size = {boundary.width / num_cols, boundary.height / num_rows};
    Vector2 prev = player.position;
    Vector2 p2 = Vector2Add(player.position, Vector2Scale(player.direction, player.near_plane));
    Vector2 next = p2;
    Vector2 ray_dir = Vector2Normalize(Vector2Subtract(p2, player.position));
    Vector2 left = player.fov_left();
    Vector2 right = player.fov_right();
    Vector2 left_right = Vector2Subtract(right, left);
    for (int x = boundary.x; x < boundary.width; ++x) {
	prev = player.position;
	ray_dir = Vector2Subtract(left, player.position);
	next = Vector2Add(player.position, ray_dir);
	for(int i = 0; i < player.far_plane; ++i) {
	    int kind = 0;
	    Vector2 p = next_point(prev, next, kind);
	    prev = next;
	    next = p;
	    Vector2 cell = Vector2Add(next, Vector2Scale(ray_dir, epsilon * 100.f));
	    cell.x = floor(cell.x);
	    cell.y = floor(cell.y);
	    wall_tex cell_type = level[index(cell.x, cell.y)];
	    if (cell_type) {
		float distance = Vector2DotProduct(Vector2Subtract(next, player.position), player.direction) * 2.f;
		Color c = colors[kind];
		if (cell_type == FLAT) draw_strip_flat(x, 1.f / distance, c);
		else if (cell_type == JOHANNDER) {
		    Vector2 t = Vector2Subtract(next, cell);
		    float u = 0;
		    if(float_equal(t.y, 1.f)) u = t.x;
		    else if(float_equal(t.y, 0.f)) u = 1.f - t.x;
		    else if(float_equal(t.x, 0.f)) u = t.y;
		    else  u = 1.f - t.y;
		    draw_strip(x, u * johannder_img.width, 1.f / distance, johannder_img);
		}
		else if (cell_type == STONE_WALL) {
		    Vector2 t = Vector2Subtract(next, cell);
		    float u = 0;
		    if(float_equal(t.y, 1.f)) u = t.x;
		    else if(float_equal(t.y, 0.f)) u = 1.f - t.x;
		    else if(float_equal(t.x, 0.f)) u = t.y;
		    else  u = 1.f - t.y;
		    draw_strip(x, u * stone_wall_img.width, 1.f / distance, stone_wall_img);
		}
		break;
	    }
	    else {
	    }
	}
	left = Vector2Add(left, Vector2Scale(left_right, 1.f / window_size.x));
    }
}

void draw_walls() {
    Vector2 cell = player.position;
    Vector2 p2 = Vector2Add(player.position, Vector2Scale(player.direction, player.near_plane));
    Vector2 left = player.fov_left();
    Vector2 right = player.fov_right();
    Vector2 left_to_right = Vector2Scale(Vector2Normalize(Vector2Subtract(right, left)), player.near_plane * 2.f / window_size.x);
    Vector2 direction = Vector2Normalize(Vector2Subtract(left, player.position));
    Vector2 prev = cell;
    DrawLineV(to_map(player.position), to_map(left), BLUE);
    DrawLineV(to_map(player.position), to_map(right), BLUE);
    float strip_width = 1;

    for(u64 x = window_size.x / 2; x < window_size.x/2 + 10; ++x) {
	int kind;
	cell = next_point(player.position, left, kind);
	Color c = RED;
	float distance = 0.f;
	while(distance <= player.far_plane) {
	//for (u64 depth = 0; depth < player.far_plane; ++depth) {
	    cell = Vector2Add(cell, Vector2Scale(Vector2Subtract(cell, prev), epsilon));
	    u64 lx = std::floor(cell.x);
	    u64 ly = std::floor(cell.y);
	    distance = Vector2DotProduct(Vector2Subtract(cell, player.position), player.direction) * 2.f;
	    float scale = 1.f / distance; 
	    float color_scale = scale - 0.6f;
	    c = ColorBrightness(RED, color_scale);
	    if (lx < num_cols && ly < num_rows) {
		wall_tex cell_type = level[index(lx, ly)];
		if(cell_type) {
		    //debug
		    ImageDrawLineV(&map_img, to_map(prev), to_map(cell), BLUE); 
		    Vector2 v = to_map({float(lx), float(ly)});
		    ImageDrawCircleV(&map_img, to_map(cell), 3, RED);
		    ImageDrawRectangleLines(&map_img, {v.x, v.y, to_screen.x / map_factor, to_screen.y / map_factor}, 1, RED);
		    Vector2 camera_plane = Vector2Add(player.position, left_to_right);
		    ImageDrawLineV(&map_img, to_map(player.position), to_map(Vector2Add(camera_plane, Vector2Scale(left_to_right, 100))), GREEN);
		    if (cell_type == FLAT) {
			draw_strip_flat(x, scale, c);
			break;
		    }
		    Image img = cell_type == STONE_WALL ? stone_wall_img : johannder_img; 
		    Vector2 t = Vector2Subtract(cell, {(float)lx, (float)ly});
		    u64 u = 0;
		    if (std::abs(t.x - 1.f) < epsilon || std::abs(t.x) < epsilon) {
			u = t.y * img.width;
		    }
		    else u = t.x * img.width;
		    u = t.y * img.width;
		    draw_strip(x, u, scale, img);
		    break;
		}
	    }
	    prev = cell;	
	    cell = next_point(cell, Vector2Add(cell, Vector2Scale(direction, player.near_plane)), kind);
	}
	left = Vector2Add(left, left_to_right);
	direction = Vector2Normalize(Vector2Subtract(left, player.position));
    }
}


void draw_floor() {
    ImageDrawRectangleV(&game_img, {0.f, window_size.y / 2.f}, {window_size.x, window_size.y / 2.f}, floor_col);
}


int main() {
    InitWindow(window_size.x, window_size.y, window_title);
    SetTargetFPS(fps);
    map_txt = LoadTextureFromImage(map_img);
    ImageFormat(&stone_wall_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&johannder_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    stone_wall_tex = LoadTextureFromImage(stone_wall_img);
    game_tex = LoadTextureFromImage(game_img);
    johannder_tex = LoadTextureFromImage(johannder_img);
    while(!WindowShouldClose()) {
	BeginDrawing();
	//ClearBackground(bg_color);
	ImageClearBackground(&game_img, bg_color);
	controls();
	draw_floor();
	draw_map(map_boundary);
	draw_walls2(game_boundary);
	UpdateTexture(game_tex, game_img.data); 
	UpdateTexture(map_txt, map_img.data); 
	DrawTexture(game_tex, 0, 0, WHITE);
	DrawTexturePro(map_txt, map_boundary, map_boundary, {0.f, 0.f}, 0.f, WHITE);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

