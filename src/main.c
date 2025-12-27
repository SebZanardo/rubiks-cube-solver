#include "core.h"
#include "cube.h"
#include "input.h"
#include "solve.h"
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
    ArenaInit(&arena, Kilobytes(1));  // [ 24 / 1024 ] bytes used

    Arena arena_temp;
    ArenaInit(&arena_temp, Kilobytes(1));  // [ 256 / 1024 ] bytes used

    Arena arena_solve;
    ArenaInit(&arena_solve, Megabytes(5));  // [ ~4.3 / 5 ] megabytes used

    // Only one cube for now
    Cube cube;
    CubeInit(&arena, &cube);

    CubeColour active_colour = CUBE_GREEN;

    CubeSetSolved(&cube);

    bool valid = CubeValid(&arena_temp, &cube);
    ArenaReset(&arena_temp);

    while (!WindowShouldClose()) {
        Rectangle cube_rect = (Rectangle) {
            0, 0, GetScreenWidth(), GetScreenHeight()
        };

        Vector2 mouse_position = GetMousePosition();

        int key = GetKeyPressed();
        if (key >= KEY_ONE && key <= KEY_SIX) {
            active_colour = key - KEY_ONE;
        }

        // UPDATE
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            CubeMousePaint(&cube, mouse_position, active_colour, cube_rect);

            valid = CubeValid(&arena_temp, &cube);
            ArenaReset(&arena_temp);
        }

        if (InputPressed(INPUT_RESET)) {
            CubeSetSolved(&cube);
            valid = true;
        }

        if (valid) {
            for (u8 i = 0; i < CUBE_COLOUR_COUNT; i++) {
                if (InputPressed(i)) {
                    if (InputDown(INPUT_PRIME)) {
                        CubeFaceTurnAntiClockwise(&cube, i);
                    } else if (InputDown(INPUT_DOUBLE)) {
                        CubeFaceTurnDouble(&cube, i);
                    } else {
                        CubeFaceTurnClockwise(&cube, i);
                    }
                }
            }

            if (InputPressed(INPUT_SHUFFLE)) {
                // Always scramble from solved position
                CubeSetSolved(&cube);
                CubeHandScramble(&cube);
            }
        }

        if (valid && InputPressed(INPUT_SOLVE)) {
            SolveCube(&arena_solve, &cube);
        }

        // RENDER
        BeginDrawing();
            ClearBackground(GRAY);
            CubeRender(&cube, cube_rect, valid);
            DrawRectangleLinesEx(cube_rect, 2.0f, CubeFaceColour(active_colour));
        EndDrawing();
    }

    CloseWindow();
}

