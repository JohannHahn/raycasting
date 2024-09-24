#include "raylib/raylib/include/raylib.h"
#include <cassert>
#include <cstring>
#include <iostream>
#define COMMON_IMPL
//#define VS_STUDIO
#include "common.hpp"
#include "draw.hpp"



Vector2 window_size = {1200.f, 1200.f}; 
Rectangle map_boundary = {0.f, 0.f, screen_size.x / map_factor, screen_size.y / map_factor};
Rectangle game_boundary = {0.f, 0.f, screen_size.x, screen_size.y};
Color bg_color = SKYBLUE;
Color map_bg = GRAY;
Color floor_col = DARKGRAY;
Image map_img = GenImageColor(map_boundary.width, map_boundary.height, map_bg);
Image game_img = GenImageColor(screen_size.x, screen_size.y, bg_color);
const char* wall_tex_path = "tileable10d.png";
const char* johannder_path = "johannder.png";
const char* sprite_path = "woodSword.png";
const char* helmet_path = "helmet.png";
const char* blob_path = "blob.png";
Image stone_wall_img = LoadImage(wall_tex_path);
Image johannder_img = LoadImage(johannder_path);
Image sword_img = LoadImage(sprite_path);
Image helmet_img = LoadImage(helmet_path);
Image blob_img = LoadImage(blob_path);
Texture johannder_tex;
Context context;


Wall walls[WALL_TEX_MAX] = {
    {EMPTY, NULL, {}}, 
    {FLAT, NULL, {}}, 
    {STONE_WALL, NULL, {}}, 
    {JOHANNDER, &sword_img, {1.f, 1.f}}
};

void animate_sprite(Sprite& s, float t) {
    float t_lerped = Lerp(0.f, 2.f * PI, t);
    s.height += sin(t_lerped) * 0.1f;
}



void controls() {
    float dt = GetFrameTime();

    Vector2 mouse_delta = GetMouseDelta();
    context.player.change_dir(mouse_delta.x);
    context.player.look_vert -= context.player.rotation_speed * mouse_delta.y * dt;
    float max_look = 19.f;
    if (context.player.look_vert < (-max_look)) context.player.look_vert = -max_look; 
    else if (context.player.look_vert > max_look) context.player.look_vert = max_look; 

    if (IsKeyDown(KEY_E)) {
	context.player.look_vert += 1.f * dt;
    }
    if (IsKeyDown(KEY_R)) {
	context.player.look_vert -= 1.f * dt;
    }
    if (IsKeyDown(KEY_U)) context.sprites[0].height++; 
    if (IsKeyDown(KEY_J)) context.sprites[0].height--; 
    if (IsKeyPressed(KEY_P)) {
	context.debug_print = true;
    }
    Vector2 new_pos = context.player.position;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
	new_pos = Vector2Subtract(context.player.position, Vector2Scale(context.player.direction, context.player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(context.player.direction, context.player.size)), context)) context.player.position = new_pos;
    } 
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
	new_pos = Vector2Add(context.player.position, Vector2Scale(context.player.direction, context.player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(context.player.direction, context.player.size)), context)) context.player.position = new_pos;
    } 
    if (IsKeyDown(KEY_A)) {
	new_pos = Vector2Add(context.player.position, Vector2Scale(Vector2Rotate(context.player.direction, -PI/2.f), context.player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(context.player.direction, context.player.size)), context)) context.player.position = new_pos;
    } 
    if (IsKeyDown(KEY_D)) {
	new_pos = Vector2Add(context.player.position, Vector2Scale(Vector2Rotate(context.player.direction, PI/2.f), context.player.speed * dt));
	if (!inside_wall(Vector2Add(new_pos, Vector2Scale(context.player.direction, context.player.size)), context)) context.player.position = new_pos;
    } 
    if (IsKeyDown(KEY_LEFT)) {
	context.player.change_dir(-1);
    } 
    if (IsKeyDown(KEY_RIGHT)) {
	context.player.change_dir(1);
    } 
    if (IsKeyPressed(KEY_ONE)) {
	context.player.speed += 0.1f;
    } 
    if (IsKeyPressed(KEY_TWO)) {
	context.player.speed -= 0.1f;
    } 
    context.player.position = new_pos;
}

void resize() {
    context.window_size.x = GetScreenWidth();
    context.window_size.y = GetScreenHeight();
}


void render() {
    UpdateTexture(context.game_tex, context.game_img->data);
    DrawTexturePro(context.game_tex, game_boundary, {0.f, 0.f, window_size.x, window_size.y}, {0.f, 0.f}, 0, WHITE);
    UpdateTexture(context.map_tex, context.map_img->data);
    DrawTexturePro(context.map_tex, map_boundary, {0.f, 0.f, window_size.x / map_factor, window_size.y / map_factor}, {0.f, 0.f}, 0, WHITE);
    for(float& n: context.depth_buffer) {
	n = max_depth;
    } 
}

int main() {
    Image flat = GenImageColor(100, 100, GRAY);
    Image none = GenImageColor(1, 1, BLACK);
    context.wall_textures[FLAT] = &flat;
    context.wall_textures[EMPTY] = &flat;
    context.wall_textures[JOHANNDER] = &johannder_img;
    context.wall_textures[STONE_WALL] = &stone_wall_img;
    context.map_img = &map_img;
    context.game_img = &game_img;
    context.game_boundary = game_boundary;
    context.map_boundary = map_boundary;
    context.sprites[0] = (Sprite){.position = {2.5f, 2.5f}, .size = {0.5f, 0.5f}, .img = &blob_img};
    context.sprites[1] = (Sprite){.position = {3.5f, 2.5f}, .size = {1.f, 1.f}, .img = &johannder_img};
    context.sprites[2] = (Sprite){.position = {3.5f, 1.5f}, .size = {0.5f, 0.5f}, .img = &sword_img};
    context.sprites[3] = (Sprite){.position = {1.5f, 2.5f}, .size = {0.33f, 0.33f}, .img = &sword_img};
    ImageFormat(&johannder_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&stone_wall_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&blob_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&sword_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    InitWindow(window_size.x, window_size.y, window_title);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(120);
    context.map_tex = LoadTextureFromImage(map_img);
    context.game_tex = LoadTextureFromImage(game_img);
    context.player.direction = Vector2Normalize({1.f, 1.f});
    context.player.look_vert = 0.f;
    johannder_tex = LoadTextureFromImage(johannder_img);
    context.test_tex = LoadTextureFromImage(context.test_img);
    float t = 0.f;
    while(!WindowShouldClose()) {
	if (IsWindowResized()) resize();
	BeginDrawing();
	ImageClearBackground(&game_img, bg_color);
	draw_map(map_boundary, context);
	controls();
	draw_floor(context);
	//draw_walls(context);
	for(Sprite& s : context.sprites) { 
	    draw_sprite(s, context);
	    animate_sprite(s, t);
	}
	t += GetFrameTime();
	if (t > 1.f) t = 0.f;
	DrawFPS(700, 100);
	render();
	EndDrawing();
    }

    CloseWindow();
    return 0;
}

