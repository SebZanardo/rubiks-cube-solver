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

    CubeSetSolved(&cube);

    while (!WindowShouldClose()) {
        CubeUpdate(&cube);

        BeginDrawing();
        ClearBackground(GRAY);
        CubeRender(&cube, 0, 0, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
    }

    CloseWindow();
}

