#include "core.h"
#include "cube.h"
#include "raylib.h"


static const int WINDOW_WIDTH = 720;
static const int WINDOW_HEIGHT = 720;
static const char WINDOW_CAPTION[] = "rubiks cube solver";
static const int WINDOW_FPS = 60;


int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_CAPTION);

    SetTargetFPS(WINDOW_FPS);
    SetRandomSeed(0);

    Image icon = LoadImage("src/data/textures/icon.png");
    SetWindowIcon(icon);

    Arena arena;
    ArenaInit(&arena, Megabytes(4));

    Cube cube;
    CubeInit(&arena, &cube);
    CubeColour active_colour = CUBE_GREEN;
    bool valid = true;

    CubeSetSolved(&cube);

    while (!WindowShouldClose()) {
        Rectangle cube_rect = (Rectangle) {
            0, 0, GetScreenWidth(), GetScreenHeight()
        };

        Vector2 mouse_position = GetMousePosition();
        int key = GetKeyPressed();
        if (key >= KEY_ONE && key <= KEY_SIX) {
            active_colour = key - KEY_ONE;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            CubeMousePaint(&cube, mouse_position, active_colour, cube_rect);
            valid = CubeValid(&cube);
        }

        CubeUpdate(&cube, &valid);

        BeginDrawing();
            ClearBackground(GRAY);
            CubeRender(&cube, cube_rect, valid);
            DrawRectangleLinesEx(cube_rect, 2.0f, CubeFaceColour(active_colour));
        EndDrawing();
    }

    CloseWindow();
}

