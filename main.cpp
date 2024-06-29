#include <iostream>
#include "raylib/src/raylib.h"

float window_width = 600;
float window_height = 600;
const char* window_title = "not doom";

int main() {
    std::cout << "hi\n";
    InitWindow(window_width, window_height, window_title);

    while(WindowShouldClose()) {
	BeginDrawing();
	ClearBackground(RED);
	EndDrawing();
    }
    CloseWindow();
    return 0;
}

