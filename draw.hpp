#ifndef DRAW_HPP
#define DRAW_HPP
#include "raylib/raylib/include/raylib.h"
#include "raylib/raylib/include/raymath.h"
#include <cassert>
#include "common.hpp"

struct Sprite {
    Vector2 position;
    Vector2 size;
    float height;
    Image* img;
};



void color_brightness(Color& c, float scale) {
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


Vector2 transform_point (Vector2 point, Rectangle dst, Vector2 src) {
    return Vector2Multiply(point, {dst.width / src.x, dst.height / src.y});
}


void draw_strip_flat(u64 x, float scale, Color c, Vector2 screen_size, Image* img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 2.f, screen_size.y - 1}, scale);
    color_brightness(c, scale / 2.f);
    ImageDrawRectangleRec(img, strip, c);
}

void draw_strip_sprite(const Sprite& sprite, Context& context, const Player& player, 
		       float screen_x, u64 texture_x, float scale, Image* img) {
    if (texture_x >= sprite.img->width) return;

    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)screen_x, 0, 1.f, screen_size.y - 1}, scale);
    strip.y += (1.f - sprite.size.y ) * strip.height;  
    float v = 0.f;
    float dist_light = Vector2Length(Vector2Subtract(context.light_pos, sprite.position));
    float depth = Vector2Length(Vector2Subtract(sprite.position, player.position));
    u32* pixels = (u32*)(img->data);

    for (u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v >= sprite.img->height) break;

	float screen_y = y - ((sprite.height / num_cols * screen_size.y) * scale);
	screen_y += player.look_vert / num_cols * screen_size.y;
	if (screen_y < 0.f) screen_y = 0.f; 
	else if (screen_y >= screen_size.y) screen_y = screen_size.y - 1;

	u64 idx = index(screen_x, screen_y, screen_size.x);
	assert(idx < screen_size.x * screen_size.y);

	Color txt_pixel = u32_to_col(((u32*)sprite.img->data)[index(texture_x, v, sprite.img->width)]);
	if (depth < context.depth_buffer[idx] && txt_pixel.a > 100) {
	    Color game_pixel = u32_to_col(((u32*)img->data)[index(screen_x, screen_y, img->width)]);
	    Color final_col = ColorAlphaBlend(game_pixel, txt_pixel, WHITE);
	    color_brightness(final_col, (1.f / dist_light + scale));
	    //ImageDrawPixel(&game_img, x, y_screen, final_col);
	    pixels[index(screen_x, screen_y, img->width)] = *(u32*)&final_col;
	    context.depth_buffer[index(screen_x, screen_y, screen_size.x)] = depth;
	}
	v += 1.f / strip.height * sprite.img->height;
    }
}

void draw_strip(Context& context, const Player& player, Vector2 pos, float screen_x, float top, u64 u, float scale, Image* img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)screen_x, top, 1.f, screen_size.y - 1}, scale);
    if (u > img->width) return;
    float v = 0.f;
    float dist_light = Vector2Length(Vector2Subtract(context.light_pos, pos));
    float depth = Vector2Length(Vector2Subtract(pos, player.position));
    u32* pixels = (u32*)(img->data);
    for(u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v < img->height) {
	    float screen_y = y + player.look_vert / num_cols * screen_size.y;
	    if (screen_y < 0.f) screen_y = 0.f; 
	    else if (screen_y >= screen_size.y) screen_y = screen_size.y - 1;
	    u32 pixel_col = ((u32*)img->data)[u + (u64)v * img->width];
	    Color* col = (Color*)(&pixel_col);
	    color_brightness(*col, (1.f / dist_light + scale));
	    //ImageDrawPixel(&game_img, x, y_screen, *col);
	    pixels[index(screnn_x, screen_y, img->width)] = *(u32*)col;
	    context.depth_buffer[index(screen_x, screen_y, screen_size.x)] = depth;
	}
	v += 1.f / strip.height * img->height;
    }
}


void draw_walls(Rectangle boundary, const Player& player, Context& context) {
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
		Vector2 cell = Vector2Add(next, Vector2Scale(ray_dir, epsilon));
		cell.x = floor(cell.x);
		cell.y = floor(cell.y);
		if (cell.x >= num_cols || cell.y >= num_rows) continue;
		wall_tex cell_type = context.level[index(cell.x, cell.y, num_cols)];
		if (cell_type) {
		    float distance = Vector2DotProduct(Vector2Subtract(next, player.position), player.direction) * 2.f;
		    Image img = context.images[cell_type];
		    Vector2 t = Vector2Subtract(next, cell);
		    float txt_x = 0.f;
		    if(float_equal(t.y, 1.f)) txt_x = t.x;
		    else if(float_equal(t.y, 0.f)) txt_x = 1.f - t.x;
		    else if(float_equal(t.x, 0.f)) txt_x = t.y;
		    else  txt_x = 1.f - t.y;
		    draw_strip(context, player, next, x, 0.f, txt_x * img.width, 1.f / distance, &img);
		    break;
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

    for(const Sprite& s : sprites) {
	ImageDrawCircleV(&map_img, to_map(s.position), 2, RED);
    }

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
    ImageDrawRectangle(&game_img, 0, end, screen_size.x, screen_size.y - end, ColorTint(floor_col, c));
#if 0
    for (float y = screen_size.y - 1; y >= end; --y) {
	ImageDrawLineV(&game_img, {0, y}, {screen_size.x, y}, ColorTint(floor_col, c));
	//color_brightness(c, scale);
    }
#endif
}
#endif
