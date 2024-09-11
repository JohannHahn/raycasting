#include <cassert>
#include <iostream>
#include "common.hpp"
#include "draw.cpp"



Vector2 window_size = {1200.f, 1200.f}; 
Rectangle map_boundary = {0.f, 0.f, screen_size.x / map_factor, screen_size.y / map_factor};
Rectangle game_boundary = {0.f, 0.f, screen_size.x, screen_size.y};
Color bg_color = SKYBLUE;
Color map_bg = GRAY;
Color floor_col = DARKGRAY;
Image map_img = GenImageColor(map_boundary.width, map_boundary.height, map_bg);
Texture map_tex;
Image game_img = GenImageColor(screen_size.x, screen_size.y, bg_color);
Texture game_tex;
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
bool debug_map = false;
Texture johannder_tex;


Wall walls[WALL_TEX_MAX] = {
    {EMPTY, NULL, {}}, 
    {FLAT, NULL, {}}, 
    {STONE_WALL, NULL, {}}, 
    {JOHANNDER, &sword_img, {1.f, 1.f}}
};

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



Sprite sprites[] = {{.position = {2.5f, 2.5f}, .size = {0.5f, 0.5f}, .img = &blob_img}, {.position = {3.5f, 2.5f}, .size = {1.f, 1.f}, .img = &johannder_img}, 
		    {.position = {3.5f, 1.5f}, .size = {0.5f, 0.5f}, .img = &sword_img}, {.position = {1.5f, 2.5f}, .size = {0.33f, 0.33f}, .img = &sword_img}};
struct Player {
    Vector2 position = {num_cols / 2.f, num_rows/ 2.f};
    Vector2 direction = {0.f, 1.f};
    float look_vert = 0.f;
    float speed = 5.f;
    float size = 0.4f;
    float rotation_speed = PI * 2.f;
    float near_plane = 0.2f;
    float far_plane = num_cols * 2;
    void change_dir(int sign) {
	if (sign < 0) sign = -1;
	if (sign > 0) sign = 1;
	direction = Vector2Normalize(Vector2Rotate(direction, sign * rotation_speed * GetFrameTime()));
    }
    Color color = RED;
    Vector2 fov_left() {
	return Vector2Subtract(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2.f));
    }
    Vector2 fov_right() {
	return Vector2Add(Vector2Add(position, Vector2Scale(direction, near_plane)), Vector2Rotate(Vector2Scale(direction, near_plane), PI/2.f));
    }
};


void animate_sprite(Sprite& s, float t) {
    float t_lerped = Lerp(0.f, 2.f * PI, t);
    s.z += sin(t_lerped) * 0.1f;
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
    if (IsKeyDown(KEY_U)) sprites[0].z++; 
    if (IsKeyDown(KEY_J)) sprites[0].z--; 
    if (IsKeyPressed(KEY_P)) {
	debug_print = true;
    }
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



void resize() {
    window_size.x = GetScreenWidth();
    window_size.y = GetScreenHeight();
}

void draw_sprite(const Sprite& sprite) {
    float distance = Vector2DotProduct(Vector2Subtract(sprite.position, player.position), player.direction);
    if (distance >= player.far_plane) return;
    float scale = sprite.size.x / distance / 2.f;
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
		draw_strip_sprite(sprite, x, u, scale);
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
    Player player = {};
    Context context;

    InitWindow(window_size.x, window_size.y, window_title);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    ImageFormat(&stone_wall_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&johannder_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ImageFormat(&sword_img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    map_tex = LoadTextureFromImage(map_img);
    game_tex = LoadTextureFromImage(game_img);
    player.direction = Vector2Normalize({1.f, 1.f});
    player.look_vert = 0.f;
    johannder_tex = LoadTextureFromImage(johannder_img);
    float t = 0.f;
    while(!WindowShouldClose()) {
	if (IsWindowResized()) resize();
	BeginDrawing();
	ImageClearBackground(&game_img, bg_color);
	controls();
	draw_floor();
	draw_walls(game_boundary);
	draw_map(map_boundary);
	for(Sprite& s : sprites) { 
		draw_sprite(s);
		animate_sprite(s, t);
	}
	t += GetFrameTime();
	if (t > 1.f) t = 0.f;
	render();
	EndDrawing();
    }

    CloseWindow();
    return 0;
}

