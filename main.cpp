bool array2[] = {1, 0, 1, 0, 1, 0, 1, 0, 1};
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
constexpr float fps = 660.f;
constexpr float map_factor = 2.f;
Rectangle map_boundary = {0.f, 0.f, window_size.x / map_factor, window_size.y / map_factor};
Color bg_color = BLACK;
Color map_bg = DARKGRAY;
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
Vector2 light_pos = {num_cols / 2.f, num_rows / 2.f};

enum wall_tex {
    EMPTY, FLAT, STONE_WALL, JOHANNDER,
};


struct Player {
    Vector2 position = {num_cols / 2.f, num_rows/ 2.f};
    Vector2 direction = {0.f, 1.f};
    float speed = 2.f;
    float rotation_speed = PI;
    float near_plane = .4f;
    float far_plane = num_cols * 2;
    void change_dir(int sign) {
	if (sign < 0) sign = -1;
	if (sign > 0) sign = 1;
	direction = Vector2Normalize(Vector2Rotate(direction, sign * rotation_speed * GetFrameTime()));
    }
    Color color = RED;
    float size = near_plane + EPSILON;
};
Player player;


constexpr u64 index(u64 x, u64 y) {
    return x + y * num_cols;
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
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, EMPTY, JOHANNDER, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, STONE_WALL, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
    FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,  FLAT,
};

float snap(float n, float dn) {
    
    if(dn > 0) {
	return std::ceil(n + EPSILON);
    }

    if(dn < 0) { 
	return std::floor(n - EPSILON);
    }
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
    Vector2 p2_map = Vector2Multiply(Vector2Add(player.position, Vector2Scale(player.direction, num_cols / 2.f)), size);
    ImageDrawCircleV(&map_img, player_map, player.size * size.x, player.color);
    ImageDrawLineV(&map_img, player_map, p2_map, player.color);
    ImageDrawCircleV(&map_img, p2_map, player.size * size.x / 2.f, player.color);


}

bool inside_wall(Vector2 p) {
    return level[index(p.x, p.y)];
}

void controls() {
    float dt = GetFrameTime();
    Vector2 new_pos = player.position;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
	new_pos = Vector2Subtract(player.position, Vector2Scale(player.direction, player.speed * dt));
	if (!inside_wall(new_pos)) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
	new_pos = Vector2Add(player.position, Vector2Scale(player.direction, player.speed * dt));
	if (!inside_wall(new_pos)) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_A)) {
	new_pos = Vector2Add(player.position, Vector2Scale(Vector2Rotate(player.direction, -PI/2.f), player.speed * dt));
	if (!inside_wall(new_pos)) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_D)) {
	new_pos = Vector2Add(player.position, Vector2Scale(Vector2Rotate(player.direction, PI/2.f), player.speed * dt));
	if (!inside_wall(new_pos)) player.position = new_pos;
    } 
    if (IsKeyDown(KEY_LEFT)) {
	player.change_dir(-1);
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	player.change_dir(1);
    } 
}

void draw_strip_flat(u64 x, float scale, Color c) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 1.f, window_size.y - 1}, scale);
    ImageDrawRectangleRec(&game_img, strip, c);
}

void draw_strip(u64 x, u64 u, float scale, Image img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 1.f, window_size.y - 1}, scale);
    if (u > img.width && "!") return;
    float v = 0.f;
    for(u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v < img.height) {
	    u32 pixel_col = ((u32*)img.data)[u + (u64)v * img.width];
	    Color* col = (Color*)(&pixel_col);
	    ImageDrawPixel(&game_img, x, y, ColorBrightness(*col, scale / 4.f));
	}
	v += 1.f / strip.height * img.height;
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
    float strip_width = 1;
    for(u64 x = 0; x < window_size.x; ++x) {
	cell = player.position;
	Color c = RED;
	float distance = 0.f;
	while(distance <= player.far_plane) {
	//for (u64 depth = 0; depth < player.far_plane; ++depth) {
	    cell = Vector2Add(cell, Vector2Scale(Vector2Normalize(Vector2Subtract(cell, prev)), EPSILON));
	    u64 lx = std::floor(cell.x);
	    u64 ly = std::floor(cell.y);
	    //float distance = Vector2Length(Vector2Subtract(cell, player.position));
	    distance = Vector2DotProduct(Vector2Subtract(cell, player.position), player.direction) * 2.f;
	    float scale = 1.f / distance; 
	    float color_scale = 1.f / Vector2Length(Vector2Subtract(light_pos, cell));
	    c = ColorBrightness(RED, color_scale);
	    prev = cell;	
	    cell = next_point(cell, Vector2Scale(direction, EPSILON));
	    //debug
	    
	    if (lx < num_cols && ly < num_rows) {
		wall_tex cell_type = level[index(lx, ly)];
		if(cell_type) {
		    ImageDrawLineV(&map_img, to_map(prev), to_map(cell), BLUE); 
		    ImageDrawCircleV(&map_img, to_map(prev), 3, RED);
		    Vector2 v = to_map({float(lx), float(ly)});
		    ImageDrawRectangleLines(&map_img, {v.x, v.y, to_screen.x / map_factor, to_screen.y / map_factor}, 1, RED);
		    Vector2 camera_plane = Vector2Add(player.position, left_to_right);
		    ImageDrawLineV(&map_img, to_map(player.position), to_map(Vector2Add(camera_plane, Vector2Scale(left_to_right, 100))), GREEN);

		    if (cell_type == FLAT) {
			draw_strip_flat(x, scale, c);
			break;
		    }
		    Image img = cell_type == STONE_WALL ? stone_wall_img : johannder_img; 
		    Vector2 t = Vector2Subtract(prev, {(float)lx, (float)ly});
		    u64 u = 0;
		    if (std::abs(t.x - 1.f) < EPSILON || std::abs(t.x) < EPSILON) {
			u = (1.f - t.y) * img.width;
		    }
		    else u = t.x * img.width;
		    draw_strip(x, u, scale, img);
		    break;
		}
	    }
	}
	left = Vector2Add(left, left_to_right);
	direction = Vector2Normalize(Vector2Subtract(left, player.position));
    }
}

void draw_ceiling() {
    ImageDrawRectangle(&game_img, 0, 0, window_size.x, window_size.y -  window_size.y * (1.f/3.f), BLACK);
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
	//draw_ceiling();
	draw_map(map_boundary);
	draw_walls();
	UpdateTexture(game_tex, game_img.data); 
	UpdateTexture(map_txt, map_img.data); 
	DrawTexture(game_tex, 0, 0, WHITE);
	DrawTexturePro(map_txt, map_boundary, map_boundary, {0.f, 0.f}, 0.f, WHITE);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

