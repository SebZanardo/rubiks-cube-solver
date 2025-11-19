#include "arena.h"
#include "cube.h"
#include "raylib.h"


#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 720
#define WINDOW_CAPTION "rubiks cube solver"
#define WINDOW_FPS 60


int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_CAPTION);
    printf(WINDOW_CAPTION);

    SetTargetFPS(WINDOW_FPS);
    SetRandomSeed(0);

    Image icon = LoadImage("src/data/textures/icon.png");
    SetWindowIcon(icon);

    Arena arena;
    ArenaInit(&arena, UINT32_MAX);

    Cube cube;
    CubeInit(&arena, &cube);

    CubeSetSolved(&cube);

    while (!WindowShouldClose()) {
        CubeUpdate(&cube);

        BeginDrawing();
        ClearBackground(BLACK);
        CubeRender(&cube, 0, 0, GetScreenWidth(), GetScreenHeight());
        EndDrawing();
    }

    CloseWindow();
}

