#ifndef DRAW_HPP
#define DRAW_HPP
#include "raylib/raylib/include/raylib.h"
#include "raylib/raylib/include/raymath.h"
#include <cassert>
#include "common.hpp"

void test_image(Context& context) {

    DrawTexture(context.test_tex, 0, 0, WHITE);
}

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
    assert(!float_equal(src.x, 0.f) && !float_equal(src.y, 0.f));
    return Vector2Multiply(point, {dst.width / src.x, dst.height / src.y});
}

Vector2 transform_point (Vector2 point, Rectangle dst, Rectangle src) {
    assert(!float_equal(src.width, 0.f) && !float_equal(src.height, 0.f));
    return Vector2Multiply(point, {dst.width / src.width, dst.height / src.height});
}

Vector2 to_map(Vector2 point, Context& context) {
    return transform_point(point, context.map_boundary, context.game_boundary);
}

void draw_strip_flat(u64 x, float scale, Color c, Vector2 screen_size, Image* img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)x, 0, 2.f, screen_size.y - 1}, scale);
    color_brightness(c, scale / 2.f);
    ImageDrawRectangleRec(img, strip, c);
}

void draw_strip_sprite(const Sprite& sprite, Context& context, float screen_x, u64 texture_x, float scale) {
    if (texture_x >= sprite.img->width) return;

    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)screen_x, 0, 1.f, screen_size.y - 1}, scale);
    strip.y += (1.f - sprite.size.y ) * strip.height;  
    float v = 0.f;
    float dist_light = Vector2Length(Vector2Subtract(context.light_pos, sprite.position));
    float depth = Vector2Length(Vector2Subtract(sprite.position, context.player.position));
    u32* pixels = (u32*)(context.game_img->data);

    for (u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v >= sprite.img->height) break;

	float screen_y = y - ((sprite.height / num_cols * screen_size.y) * scale);
	screen_y += context.player.look_vert / num_cols * screen_size.y;
	if (screen_y < 0.f) screen_y = 0.f; 
	else if (screen_y >= screen_size.y) screen_y = screen_size.y - 1;

	u64 idx = index(screen_x, screen_y, screen_size.x);
	assert(idx < screen_size.x * screen_size.y);

	Color txt_pixel = u32_to_col(((u32*)sprite.img->data)[index(texture_x, v, sprite.img->width)]);
	if (depth < context.depth_buffer[idx] && txt_pixel.a > 100) {
	    Color game_pixel = u32_to_col(((u32*)context.game_img->data)[index(screen_x, screen_y, context.game_img->width)]);
	    Color final_col = ColorAlphaBlend(game_pixel, txt_pixel, WHITE);
	    color_brightness(final_col, (1.f / dist_light + scale));
	    //ImageDrawPixel(&game_img, x, y_screen, final_col);
	    pixels[index(screen_x, screen_y, context.game_img->width)] = *(u32*)&final_col;
	    context.depth_buffer[index(screen_x, screen_y, screen_size.x)] = depth;
	}
	v += 1.f / strip.height * sprite.img->height;
    }
}

void draw_strip(Context& context, const Player& player, Vector2 pos, 
		float screen_x, float top, u64 u, float scale, Image* img) {
    scale = Clamp(scale, 0.f, 1.f);
    Rectangle strip = squish_rec({(float)screen_x, top, 1.f, screen_size.y - 1}, scale);
    if (u > img->width) return;
    float v = 0.f;
    float dist_light = Vector2Length(Vector2Subtract(context.light_pos, pos));
    float depth = Vector2Length(Vector2Subtract(pos,context.player.position));
    u32* pixels = (u32*)(img->data);
    for(u64 y = strip.y; y < strip.y + strip.height; ++y) {
	if (v < img->height) {
	    float screen_y = y +context.player.look_vert / num_cols * screen_size.y;
	    if (screen_y < 0.f) screen_y = 0.f; 
	    else if (screen_y >= screen_size.y) screen_y = screen_size.y - 1;
	    u32 pixel_col = ((u32*)img->data)[u + (u64)v * img->width];
	    Color* col = (Color*)(&pixel_col);
	    color_brightness(*col, (1.f / dist_light + scale));
	    //ImageDrawPixel(&game_img, x, y_screen, *col);
	    pixels[index(screen_x, screen_y, img->width)] = *(u32*)col;
	    context.depth_buffer[index(screen_x, screen_y, screen_size.x)] = depth;
	}
	v += 1.f / strip.height * img->height;
    }
}


void draw_walls(Context& context) {
	Rectangle boundary = context.game_boundary;
    Vector2 size = {boundary.width / num_cols, boundary.height / num_rows};
    Vector2 prev =context.player.position;
    Vector2 p2 = Vector2Add(context.player.position, Vector2Scale(context.player.direction, context.player.near_plane));
    Vector2 next = p2;
    Vector2 ray_dir = Vector2Normalize(Vector2Subtract(p2,context.player.position));
    Vector2 left =context.player.fov_left();
    Vector2 right =context.player.fov_right();
    Vector2 left_right = Vector2Subtract(right, left);
    for (int x = boundary.x; x < boundary.width; ++x) {
	prev =context.player.position;
	ray_dir = Vector2Subtract(left,context.player.position);
	next = Vector2Add(context.player.position, ray_dir);
	for(int i = 0; i <context.player.far_plane; ++i) {
		int kind = 0;
		Vector2 p = next_point(prev, next, kind);
		prev = next;
		next = p;
		Vector2 cell = Vector2Add(next, Vector2Scale(ray_dir, epsilon));
		cell.x = floor(cell.x);
		cell.y = floor(cell.y);
		if (cell.x >= num_cols || cell.y >= num_rows || cell.x < 0 || cell.y < 0) continue;
		wall_tex cell_type = context.level[index(cell.x, cell.y, num_cols)];
		if (cell_type) {
		    float distance = Vector2DotProduct(Vector2Subtract(next,context.player.position),context.player.direction) * 2.f;
		    Image* img = context.wall_textures[cell_type];
		    Vector2 t = Vector2Subtract(next, cell);
		    float txt_x = 0.f;
		    if(float_equal(t.y, 1.f)) txt_x = t.x;
		    else if(float_equal(t.y, 0.f)) txt_x = 1.f - t.x;
		    else if(float_equal(t.x, 0.f)) txt_x = t.y;
		    else  txt_x = 1.f - t.y;
		    draw_strip(context,context.player, next, x, 0.f, txt_x * img->width, 1.f / distance, img);
		    break;
		}
	}
	left = Vector2Add(left, Vector2Scale(left_right, 1.f / screen_size.x));
    }
}

void draw_map(Rectangle boundary, Context& context) {
    ImageClearBackground(context.map_img, context.map_bg);
    Vector2 size = {boundary.width / num_cols, boundary.height / num_rows};
    for (u64 x = 0; x < num_cols; ++x) {
	ImageDrawLine(context.map_img, x * size.x, 0, x*size.x, num_rows * size.y, ColorAlpha(RAYWHITE, 0.5f));
    }
    for (u64 y = 0; y < num_rows; ++y) {
	ImageDrawLine(context.map_img, 0, y * size.y, num_cols * size.x, y * size.y, ColorAlpha(RAYWHITE, 0.5f));
    }
    for(u64 y = 0; y < num_rows; ++y) {
	for(u64 x = 0; x < num_cols; ++x) {
	    if(context.level[index(x, y, num_cols)]) {
		ImageDrawRectangleV(context.map_img, {x * size.x, y * size.y}, size, ColorAlpha(WHITE, 0.5f));
	    }
	}
    }
    Vector2 player_map = Vector2Multiply(context.player.position, size);
    Vector2 p2 = Vector2Add(context.player.position, Vector2Scale(context.player.direction, context.player.near_plane));
    Vector2 p2_map = Vector2Multiply(p2, size);
    ImageDrawCircleV(context.map_img, player_map, 2, context.player.color);
    ImageDrawLineV(context.map_img, player_map, p2_map, context.player.color);
    ImageDrawCircleV(context.map_img, p2_map, 2, context.player.color);
    
    ImageDrawLineV(context.map_img, player_map, to_map(context.player.fov_left(), context), BLUE);
    ImageDrawLineV(context.map_img, player_map, to_map(context.player.fov_right(), context), GREEN);
    ImageDrawLineV(context.map_img, to_map(context.player.fov_right(), context), to_map(context.player.fov_left(), context), RED);

    for(const Sprite& s : context.sprites) {
	ImageDrawCircleV(context.map_img, to_map(s.position, context), 2, RED);
    }

    if (context.debug_map) {
	Vector2 prev =context.player.position;
	Vector2 next = p2;
	Vector2 ray_dir = Vector2Normalize(Vector2Subtract(p2,context.player.position));
	Vector2 left =context.player.fov_left();
	Vector2 right =context.player.fov_right();
	Vector2 left_right = Vector2Subtract(right, left);
	u64 step_max = 10;
	for (int step = 0; step < step_max; ++step) {
	    prev =context.player.position;
	    ray_dir = Vector2Subtract(left,context.player.position);
	    next = Vector2Add(context.player.position, ray_dir);
	    for(int i = 0; i < context.player.far_plane; ++i) {
		int kind = 0;
		Vector2 p = next_point(prev, next, kind);
		prev = next;
		next = p;
		// COLOR?
		ImageDrawLineV(context.map_img, to_map(prev, context), to_map(next, context), *(Color*)&kind);
		Vector2 cell = Vector2Add(next, Vector2Scale(ray_dir, epsilon));
		cell.x = floor(cell.x);
		cell.y = floor(cell.y);
		Vector2 cell_map = to_map(cell, context);
		Rectangle r = {cell_map.x, cell_map.y, size.x, size.y};
		ImageDrawRectangleLines(context.map_img, r, 2, RED);
		ImageDrawLineV(context.map_img, to_map(next, context), cell_map, YELLOW);
		if (context.level[index(cell.x, cell.y, num_cols)]) {
		    ImageDrawCircleV(context.map_img, to_map(next, context), 1, MAGENTA);
		    ImageDrawCircleV(context.map_img, to_map({(float)cell.x, (float)cell.y}, context), 1, RED);
		    break;
		}
		else {
		    ImageDrawCircleV(context.map_img, to_map(next, context), 3, WHITE);
		}
	    }
	    left = Vector2Add(left, Vector2Scale(left_right, 1.f / step_max));
	}
    }
}

void draw_sprite(const Sprite& sprite, Context& context) {
    float distance = Vector2DotProduct(Vector2Subtract(sprite.position, context.player.position), context.player.direction);
    if (distance >= context.player.far_plane) return;
    float scale = sprite.size.x / distance / 2.f;
    Vector2 fov_plane = Vector2Subtract(context.player.fov_right(), context.player.fov_left());
    Vector2 camera_plane_dir = Vector2Normalize(fov_plane); 
    Vector2 sprite_left = Vector2Subtract(sprite.position, Vector2Scale(camera_plane_dir, sprite.size.x / 2.f));
    Vector2 sprite_right = Vector2Add(sprite.position, Vector2Scale(camera_plane_dir, sprite.size.x / 2.f));
    
    Vector2 collision_left, collision_right;
    bool left_visible = CheckCollisionLines(context.player.position, sprite_left, context.player.fov_left(), context.player.fov_right(), &collision_left); 
    bool right_visible = CheckCollisionLines(context.player.position, sprite_right,context.player.fov_left(), context.player.fov_right(), &collision_right);
    if (!left_visible && !right_visible) return;

    float x_start = Vector2Length(Vector2Subtract(collision_left,context.player.fov_left())) / Vector2Length(fov_plane) * screen_size.x;
    float x_end = Vector2Length(Vector2Subtract(collision_right,context.player.fov_left())) / Vector2Length(fov_plane) * screen_size.x;

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
	draw_strip_sprite(sprite, context, x, u, scale);
	u += step;
    }
}

void draw_floor(Context& context) {
    float scale = 1.f - EPSILON;
    Color c = WHITE;
    float end = screen_size.y / 2.f + (context.player.look_vert / num_cols * screen_size.y);
    if (end >= screen_size.y) end = screen_size.y - 1;
    else if (end < 0.f) end = 0.f;
    ImageDrawRectangle(context.game_img, 0, end, screen_size.x, screen_size.y - end, ColorTint(context.floor_col, c));
#if 0
    for (float y = screen_size.y - 1; y >= end; --y) {
	ImageDrawLineV(&game_img, {0, y}, {screen_size.x, y}, ColorTint(floor_col, c));
	//color_brightness(c, scale);
    }
#endif
}

void fill_depth_buffer(float val, Context& context) {
    for(float& n: context.depth_buffer) {
	n = val;
    } 
}

#endif // DRAW_HPP
