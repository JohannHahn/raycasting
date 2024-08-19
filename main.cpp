#include <cstring>
constexpr long num_cols = 10;
constexpr long num_rows = 10;
enum wall_tex {
    EMPTY, FLAT, STONE_WALL, JOHANNDER,
};
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
    EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, FLAT, 
};
#include <cassert>
#include <iostream>
#include <inttypes.h>
#include "raylib/src/raylib.h"
#include "raylib/src/raymath.h"

typedef uint64_t u64;
typedef uint32_t u32;
bool debug_print = false;

Vector2 window_size = {900.f, 900.f}; 
constexpr Vector2 screen_size = {600.f, 600.f}; 
constexpr const char* window_title = "not doom";
//constexpr u64 num_cols = 10;
//constexpr u64 num_rows = 10;
constexpr float fps = 60.f;
constexpr float map_factor = 3.f;
constexpr float epsilon = 0.00001f;
constexpr float max_depth = 999999.f;
Rectangle map_boundary = {0.f, 0.f, screen_size.x / map_factor, screen_size.y / map_factor};
Rectangle game_boundary = {0.f, 0.f, screen_size.x, screen_size.y};
Color bg_color = SKYBLUE;
Color map_bg = GRAY;
Color floor_col = DARKGRAY;
Image map_img = GenImageColor(map_boundary.width, map_boundary.height, map_bg);
Texture map_tex;
Image game_img = GenImageColor(screen_size.x, screen_size.y, bg_color);
Texture game_tex;
Vector2 to_screen_vec = {screen_size. x / num_cols, screen_size.y / num_rows};
const char* wall_tex_path = "tileable10d.png";
const char* johannder_path = "johannder.png";
const char* sprite_path = "woodSword.png";
Image stone_wall_img = LoadImage(wall_tex_path);
Image johannder_img = LoadImage(johannder_path);
Image sword_img = LoadImage(sprite_path);
bool debug_map = true;
Vector2 light_pos = {num_cols / 2.f, num_rows / 2.f};
Texture johannder_tex;

Color colors[] = {BLACK, RED, GREEN, BLUE};
float depth_buffer[(u64)screen_size.x * (u64)screen_size.y] = {num_cols * num_rows};

enum side_kind {
    UNUSED, X, Y, PARALLEL 
};



struct Sprite {
    Vector2 position; 
    Vector2 size = {0.9f, 0.9f};
    float z = 0.f;
    Image* img;
};
Sprite sprite = {.position = {2.f, 2.f}, .size = {0.5f, 0.5f}, .img = &sword_img};
Sprite sprite2 = {.position = {3.f, 2.f}, .size = {1.f, 1.f}, .img = &sword_img};
struct Player {
    Vector2 position = {num_cols / 2.f, num_rows/ 2.f};
    Vector2 direction = {0.f, 1.f};
    float look_vert = 0.f;
    float speed = 5.f;
    float size = 0.4f;
    float rotation_speed = PI * 2.f;
    float near_plane = 0.5f;
    float far_plane = num_cols * 2;
    void change_dir(int sign) {
	if (sign < 0) sign = -1;
	if (sign > 0) sign = 1;
	direction = Vector2Normalize(Vector2Rotate(direction, sign * rotation_speed * GetFrameTime()));
    }
    Color color = RED;
    Vector2 fov_left() {
	return Vector2Subtract(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2));
    }
    Vector2 fov_right() {
	return Vector2Add(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2));
    }
};
Player player = {};

constexpr bool float_equal(float a, float b) {
    return std::abs(a - b) < epsilon;
}

constexpr bool color_equal(Color c1, Color c2) {
    return (c1.a == c2.a && c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}

constexpr u64 index(u64 x, u64 y, u64 width) {
    return x + y * width;
}

Color u32_to_col(u32 i) {
    return *(Color*)(&i);
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

Rectangle scale_rec(Rectangle r, float factor) { 
    float h = r.height * factor;
    float y = (r.height - h) * 0.5f;
    float w = r.width * factor;
    float x = (r.width - w) * 0.5f;
    return {x, y, w, h};
}

Vector2 to_screen(Vector2 v) {
    return Vector2Multiply(v, to_screen_vec);
}

Vector2 to_map(Vector2 v) {
    return Vector2Multiply(v, {map_boundary.width / num_cols, map_boundary.height / num_rows});
}

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



bool inside_wall(Vector2 p) {
    return level[index(p.x, p.y, num_cols)];
}

void controls() {
    float dt = GetFrameTime();
    Vector2 mouse_delta = GetMouseDelta();
    player.change_dir(mouse_delta.x);
    player.look_vert -= player.rotation_speed * mouse_delta.y * dt;
    float max_look = 19.f;
    if (player.look_vert < (-max_look)) player.look_vert = -max_look; 
    else if (player.look_vert > max_look) player.look_vert = max_look; 
    if (IsKeyDown(KEY_E)) {
	player.look_vert += 1.f * dt;
    }
    if (IsKeyDown(KEY_R)) {
	player.look_vert -= 1.f * dt;
    }
    if (IsKeyDown(KEY_U)) {
	sprite.z++; 
    } 
    if (IsKeyPressed(KEY_P)) {
	debug_print = true;
    }
    if (IsKeyDown(KEY_J)) sprite.z--; 
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
    if (IsKeyPressed(KEY_ONE)) {
	player.speed += 0.1f;
    } 
    if (IsKeyPressed(KEY_TWO)) {
	player.speed -= 0.1f;
    } 
    player.position = new_pos;
}


void draw_strip_flat(u64 x, float scale, Color c) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 2.f, screen_size.y - 1}, scale);
    color_brightness(c, scale / 2.f);
    ImageDrawRectangleRec(&game_img, strip, c);
}

void draw_strip_sprite(Vector2 pos, float x, float height, u64 u, float scale, Image* img) {
    if (u >= img->width) return;

    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 1.f, screen_size.y - 1}, scale);
    float v = 0.f;
    float dist_light = Vector2Length(Vector2Subtract(light_pos, pos));
    float depth = Vector2Length(Vector2Subtract(pos, player.position));

    for (u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v >= img->height) break;
	float y_screen = y - ((height / num_cols * screen_size.y) * scale);
	y_screen += player.look_vert / num_cols * screen_size.y;
	if (y_screen < 0.f) y_screen = 0.f; 
	else if (y_screen >= screen_size.y) y_screen = screen_size.y - 1;
	u64 idx = index(x, y_screen, screen_size.x);
	assert(idx < screen_size.x * screen_size.y);
	Color txt_pixel = u32_to_col(((u32*)img->data)[index(u, v, img->width)]);
	if (depth < depth_buffer[idx] && txt_pixel.a > 100) {
	    Color txt_pixel = u32_to_col(((u32*)img->data)[index(u, v, img->width)]);
	    Color game_pixel = u32_to_col(((u32*)game_img.data)[index(x, y_screen, game_img.width)]);
	    Color final_col = ColorAlphaBlend(game_pixel, txt_pixel, WHITE);
	    color_brightness(final_col, (1.f / dist_light + scale));
	    ImageDrawPixel(&game_img, x, y_screen, final_col);
	    depth_buffer[index(x, y_screen, screen_size.x)] = depth;
	}
	v += 1.f / strip.height * img->height;
    }
}

void draw_strip(Vector2 pos, float x, float top, u64 u, float scale, Image img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, top, 1.f, screen_size.y - 1}, scale);
    if (u > img.width) return;
    float v = 0.f;
    float dist_light = Vector2Length(Vector2Subtract(light_pos, pos));
    float depth = Vector2Length(Vector2Subtract(pos, player.position));
    for(u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v < img.height) {
	    float y_screen = y + player.look_vert / num_cols * screen_size.y;
	    if (y_screen < 0.f) y_screen = 0.f; 
	    else if (y_screen >= screen_size.y) y_screen = screen_size.y - 1;
	    u32 pixel_col = ((u32*)img.data)[u + (u64)v * img.width];
	    Color* col = (Color*)(&pixel_col);
	    color_brightness(*col, (1.f / dist_light + scale));
	    ImageDrawPixel(&game_img, x, y_screen, *col);
	    depth_buffer[index(x, y_screen, screen_size.x)] = depth;
	}
	v += 1.f / strip.height * img.height;
    }
}


void draw_walls(Rectangle boundary) {
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
	    if (cell.x >= num_cols || cell.y >= num_rows) continue;
	    wall_tex cell_type = level[index(cell.x, cell.y, num_cols)];
	    if (cell_type) {
		float distance = Vector2DotProduct(Vector2Subtract(next, player.position), player.direction) * 2.f;
		Color c = colors[kind];
		if (cell_type == JOHANNDER) {
		    Vector2 t = Vector2Subtract(next, cell);
		    float u = 0;
		    if(float_equal(t.y, 1.f)) u = t.x;
		    else if(float_equal(t.y, 0.f)) u = 1.f - t.x;
		    else if(float_equal(t.x, 0.f)) u = t.y;
		    else  u = 1.f - t.y;
		    draw_strip(next, x, 0.f, u * johannder_img.width, 1.f / distance, johannder_img);
		}
		else {
		    Vector2 t = Vector2Subtract(next, cell);
		    float u = 0;
		    if(float_equal(t.y, 1.f)) u = t.x;
		    else if(float_equal(t.y, 0.f)) u = 1.f - t.x;
		    else if(float_equal(t.x, 0.f)) u = t.y;
		    else  u = 1.f - t.y;
		    draw_strip(next, x, 0.f, u * stone_wall_img.width, 1.f / distance, stone_wall_img);
		}
		//else draw_strip_flat(x, 1.f / distance, c);
		break;
	    }
	    else {
	    }
	}
	left = Vector2Add(left, Vector2Scale(left_right, 1.f / screen_size.x));
    }
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
	    if(level[index(x, y, num_cols)]) {
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

    ImageDrawCircleV(&map_img, Vector2Add(to_map(sprite.position), to_map({0.5f, 0.5f})), 2, RED);
    ImageDrawCircleV(&map_img, Vector2Add(to_map(sprite2.position), to_map({0.5f, 0.5f})), 2, RED);

    if (debug_map) {
	Vector2 prev = player.position;
	Vector2 next = p2;
	Vector2 ray_dir = Vector2Normalize(Vector2Subtract(p2, player.position));
	Vector2 left = player.fov_left();
	Vector2 right = player.fov_right();
	Vector2 left_right = Vector2Subtract(right, left);
	u64 step_max = 10;
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
		if (level[index(cell.x, cell.y, num_cols)]) {
		    ImageDrawCircleV(&map_img, to_map(next), 1, MAGENTA);
		    ImageDrawCircleV(&map_img, to_map({(float)cell.x, (float)cell.y}), 1, RED);
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

void draw_floor() {
    float scale = 1.f - EPSILON;
    Color c = WHITE;
    float end = screen_size.y / 2.f + (player.look_vert / num_cols * screen_size.y);
    if (end >= screen_size.y) end = screen_size.y - 1;
    else if (end < 0.f) end = 0.f;
    for (float y = screen_size.y - 1; y >= end; --y) {
	ImageDrawLineV(&game_img, {0, y}, {screen_size.x, y}, ColorTint(floor_col, c));
	color_brightness(c, scale);
    }
}

void resize() {
    window_size.x = GetScreenWidth();
    window_size.y = GetScreenHeight();
}

void draw_sprite(const Sprite& sprite) {
    float distance = Vector2DotProduct(Vector2Subtract(sprite.position, player.position), player.direction);
    if (distance >= player.far_plane) return;
    float scale = sprite.size.x / distance;
    Vector2 fov_plane = Vector2Subtract(player.fov_right(), player.fov_left());
    Vector2 camera_plane_dir = Vector2Normalize(fov_plane); 
    Vector2 sprite_left = Vector2Subtract(sprite.position, Vector2Scale(camera_plane_dir, sprite.size.x / 2.f));
    Vector2 sprite_right = Vector2Add(sprite.position, Vector2Scale(camera_plane_dir, sprite.size.x / 2.f));
    
    Vector2 collision_left, collision_right;
    bool left_visible = CheckCollisionLines(player.position, sprite_left, player.fov_left(), player.fov_right(), &collision_left); 
    bool right_visible = CheckCollisionLines(player.position, sprite_right, player.fov_left(), player.fov_right(), &collision_right);
    if (!left_visible && !right_visible) return;

    float x_start = Vector2Length(Vector2Subtract(collision_left, player.fov_left())) / Vector2Length(fov_plane) * screen_size.x;
    float x_end = Vector2Length(Vector2Subtract(collision_right, player.fov_left())) / Vector2Length(fov_plane) * screen_size.x;

    if (!right_visible) {
	x_end = screen_size.x - 1;
    }
    else if (!left_visible) {
	x_start = 0.f;
    }
    if (x_end < x_start) {
	std::cout << "x_end < x_start, x_end = " << x_end << "\n";
	return;
    }

    float full_length = scale * screen_size.x;
    float visible_length = (x_end - x_start);
    float step = (sprite.img->width * (visible_length / full_length)) / (x_end - x_start);
    float u = right_visible ? step * (full_length - visible_length) : 0.f; 
    for(u64 x = x_start; x <= x_end; ++x) {
	draw_strip_sprite(sprite.position, x, sprite.z, u, scale, sprite.img);
	u += step;
    }
}

void fill_depth_buffer(float val) {
   for(float& n: depth_buffer) {
	n = val;
    } 
}

void render() {
    UpdateTexture(game_tex, game_img.data);
    DrawTexturePro(game_tex, game_boundary, {0.f, 0.f, window_size.x, window_size.y}, {0.f, 0.f}, 0, WHITE);
    UpdateTexture(map_tex, map_img.data);
    DrawTexturePro(map_tex, map_boundary, {0.f, 0.f, window_size.x / map_factor, window_size.y / map_factor}, {0.f, 0.f}, 0, WHITE);
    fill_depth_buffer(max_depth);
}




int main() {
    InitWindow(window_size.x, window_size.y, window_title);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(fps);
    ImageFormat(&stone_wall_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&johannder_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&sword_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    map_tex = LoadTextureFromImage(map_img);
    game_tex = LoadTextureFromImage(game_img);
    player.direction = Vector2Normalize({1.f, 1.f});
    player.look_vert = 0.f;
    johannder_tex = LoadTextureFromImage(johannder_img);
    while(!WindowShouldClose()) {
	if (IsWindowResized()) resize();
	BeginDrawing();
	ImageClearBackground(&game_img, bg_color);
	controls();
	draw_floor();
	draw_walls(game_boundary);
	draw_map(map_boundary);
	draw_sprite(sprite);
	draw_sprite(sprite2);
	render();
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

