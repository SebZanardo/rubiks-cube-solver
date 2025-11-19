#include "cube.h"


#define CUBE_SIDE_COUNT 6
#define FACE_TILE_COUNT 8
#define BITMASK_TILE 0xFu  // Four bits set to true, 1111

#define TILE_RENDER_ROUNDNESS 0.5f
#define TILE_RENDER_SEGMENTS 4
#define TILE_RENDER_SPACING 0.25f

#define FACE_RENDER_SPACING (4 + TILE_RENDER_SPACING * 2)
#define FACE_RENDER_OFFSET (2 * FACE_RENDER_SPACING + TILE_RENDER_SPACING)

#define CUBE_RENDER_WIDTH (11 + TILE_RENDER_SPACING * 8)
#define CUBE_RENDER_HEIGHT (15 + TILE_RENDER_SPACING * 10)

// Maximum number of quarter turns to solve any 3x3 (upper bound of search)
#define GODS_NUMBER 20
#define GODS_NUMBER_QUARTER 26


// Lookup tables
global_variable const Color cube_colour_table[CUBE_COLOUR_COUNT] = {
    (Color) { 0,   255, 0,   255 },
    (Color) { 255, 0,   0,   255 },
    (Color) { 255, 255, 255, 255 },
    (Color) { 0,   0,   255, 255 },
    (Color) { 255, 155, 0,   255 },
    (Color) { 255, 255, 0,   255 },
};
global_variable const u8 cube_face_rotation_table[2][4] = {
    {0, 2, 4, 6},
    {1, 3, 5, 7}
};
global_variable const u8 cube_sides_rotation_table[CUBE_COLOUR_COUNT][3][8] = {
    {
        // Green Face
        {CUBE_ORANGE, 4, CUBE_WHITE, 6, CUBE_RED, 0, CUBE_YELLOW, 2},
        {CUBE_ORANGE, 3, CUBE_WHITE, 5, CUBE_RED, 7, CUBE_YELLOW, 1},
        {CUBE_ORANGE, 2, CUBE_WHITE, 4, CUBE_RED, 6, CUBE_YELLOW, 0}
    },
    {
        // Red Face
        {CUBE_GREEN, 4, CUBE_WHITE, 4, CUBE_BLUE, 4, CUBE_YELLOW, 4},
        {CUBE_GREEN, 3, CUBE_WHITE, 3, CUBE_BLUE, 3, CUBE_YELLOW, 3},
        {CUBE_GREEN, 2, CUBE_WHITE, 2, CUBE_BLUE, 2, CUBE_YELLOW, 2}
    },
    {
        // White Face
        {CUBE_ORANGE, 2, CUBE_BLUE, 6, CUBE_RED, 2, CUBE_GREEN, 2},
        {CUBE_ORANGE, 1, CUBE_BLUE, 5, CUBE_RED, 1, CUBE_GREEN, 1},
        {CUBE_ORANGE, 0, CUBE_BLUE, 4, CUBE_RED, 0, CUBE_GREEN, 0}
    },
    {
        // Blue Face
        {CUBE_ORANGE, 0, CUBE_YELLOW, 6, CUBE_RED, 4, CUBE_WHITE, 2},
        {CUBE_ORANGE, 7, CUBE_YELLOW, 5, CUBE_RED, 3, CUBE_WHITE, 1},
        {CUBE_ORANGE, 6, CUBE_YELLOW, 4, CUBE_RED, 2, CUBE_WHITE, 0}
    },
    {
        // Orange Face
        {CUBE_BLUE, 0, CUBE_WHITE, 0, CUBE_GREEN, 0, CUBE_YELLOW, 0},
        {CUBE_BLUE, 6, CUBE_WHITE, 6, CUBE_GREEN, 6, CUBE_YELLOW, 6},
        {CUBE_BLUE, 7, CUBE_WHITE, 7, CUBE_GREEN, 7, CUBE_YELLOW, 7}
    },
    {
        // Yellow Face
        {CUBE_ORANGE, 6, CUBE_GREEN, 6, CUBE_RED, 6, CUBE_BLUE, 2},
        {CUBE_ORANGE, 5, CUBE_GREEN, 5, CUBE_RED, 5, CUBE_BLUE, 1},
        {CUBE_ORANGE, 4, CUBE_GREEN, 4, CUBE_RED, 4, CUBE_BLUE, 0}
    }
};
/*
global_variable const char cube_colour_chars[CUBE_COLOUR_COUNT] = "GRWBOY";
global_variable const char *cube_colour_names[CUBE_COLOUR_COUNT] = {
    "GREEN", "RED", "WHITE", "BLUE", "ORANGE", "YELLOW"
};
global_variable const char *turn_type_names[TURN_TYPE_COUNT] = {
    "F",  "R",  "U",  "B",  "L",  "D",
    "F'", "R'", "U'", "B'", "L'", "D'",
    "F2", "R2", "U2", "B2", "L2", "D2"
};
*/


void CubeInit(Arena* arena, Cube* cube) {
    cube->faces = ArenaAlloc(arena, sizeof(*cube->faces) * FACE_TILE_COUNT);
}

void CubeUpdate(Cube* cube) {
    for (u8 i = 0; i < CUBE_COLOUR_COUNT; i++) {
        if (InputPressed(i)) {
            if (InputDown(INPUT_PRIME)) {
                CubeFaceTurnAntiClockwise(cube, i);
            } else if (InputDown(INPUT_DOUBLE)) {
                CubeFaceTurnDouble(cube, i);
            } else {
                CubeFaceTurnClockwise(cube, i);
            }
        }
    }

    if (InputPressed(INPUT_SHUFFLE)) {
        printf("Random shuffle!\n");
    }

    if (InputPressed(INPUT_SOLVE)) {
        printf("Solve!\n");
    }
}

void CubeSetSolved(Cube* cube) {
    for (u8 colour = 0; colour < CUBE_COLOUR_COUNT; colour++) {
        for (u8 position = 0; position < FACE_TILE_COUNT; position++) {
            FaceSetTile(&cube->faces[colour], colour, position);
        }
    }
}

void CubeFaceTurnClockwise(Cube* cube, enum8(CubeColour) face_colour) {
    CubeColour side_colour;
    u8 side_position;
    CubeColour temp;

    // Rotate face
    for (int j = 0; j < 2; j++) {
        side_position = cube_face_rotation_table[j][0];
        temp = FaceGetTile(cube->faces[face_colour], side_position);
        for (int i = 1; i < 5; i++) {
            side_position = cube_face_rotation_table[j][i % 4];
            temp = FaceSetTile(&cube->faces[face_colour], temp, side_position);
        }
    }

    // Rotate sides of face
    for (int j = 0; j < 3; j++) {
        side_colour = cube_sides_rotation_table[face_colour][j][0];
        side_position = cube_sides_rotation_table[face_colour][j][1];
        temp = FaceGetTile(cube->faces[side_colour], side_position);
        for (int i = 2; i < 10; i += 2) {
            side_colour = cube_sides_rotation_table[face_colour][j][i % 8];
            side_position = cube_sides_rotation_table[face_colour][j][(i + 1) % 8];
            temp = FaceSetTile(&cube->faces[side_colour], temp, side_position);
        }
    }
}

void CubeFaceTurnAntiClockwise(Cube* cube, enum8(CubeColour) face_colour) {
    CubeColour side_colour;
    u8 side_position;
    CubeColour temp;

    // Rotate face
    for (int j = 0; j < 2; j++) {
        side_position = cube_face_rotation_table[j][0];
        temp = FaceGetTile(cube->faces[face_colour], side_position);
        for (int i = 4; i > -1; i--) {
            side_position = cube_face_rotation_table[j][MOD(i, 4)];
            temp = FaceSetTile(&cube->faces[face_colour], temp, side_position);
        }
    }

    // Rotate sides of face
    for (int j = 0; j < 3; j++) {
        side_colour = cube_sides_rotation_table[face_colour][j][0];
        side_position = cube_sides_rotation_table[face_colour][j][1];
        temp = FaceGetTile(cube->faces[side_colour], side_position);
        for (int i = 8; i > -2; i -= 2) {
            side_colour = cube_sides_rotation_table[face_colour][j][MOD(i, 8)];
            side_position = cube_sides_rotation_table[face_colour][j][MOD(i + 1, 8)];
            temp = FaceSetTile(
                &cube->faces[side_colour], temp, side_position
            );
        }
    }
}

void CubeFaceTurnDouble(Cube* cube, enum8(CubeColour) face_colour) {
    // TODO: Implement lookup tables for double turns
    // Rotate sides of face
    CubeFaceTurnClockwise(cube, face_colour);
    CubeFaceTurnClockwise(cube, face_colour);
}

void CubeRender(Cube* cube, int offset_x, int offset_y, int width, int height) {
    int size = MIN(
        (width / CUBE_RENDER_WIDTH),
        (height / CUBE_RENDER_HEIGHT)
    );
    int offset_left = (width - CUBE_RENDER_WIDTH * size) / 2;
    int offset_top = (height - CUBE_RENDER_HEIGHT * size) / 2;
    int x = offset_x + offset_left + size * TILE_RENDER_SPACING;
    int y = offset_y + offset_top + size * FACE_RENDER_OFFSET;

    // Left
    FaceRender(cube->faces[CUBE_ORANGE], CUBE_ORANGE, x, y, size);

    // Back, Up, Front, Down
    x += size * FACE_RENDER_SPACING;
    y = offset_y + offset_top + size * TILE_RENDER_SPACING;
    FaceRender(cube->faces[CUBE_BLUE], CUBE_BLUE, x, y, size);
    y += size * FACE_RENDER_SPACING;
    FaceRender(cube->faces[CUBE_WHITE], CUBE_WHITE, x, y, size);
    y += size * FACE_RENDER_SPACING;
    FaceRender(cube->faces[CUBE_GREEN], CUBE_GREEN, x, y, size);
    y += size * FACE_RENDER_SPACING;
    FaceRender(cube->faces[CUBE_YELLOW], CUBE_YELLOW, x, y, size);

    // Right
    x += size * FACE_RENDER_SPACING;
    y = offset_y + offset_top + size * FACE_RENDER_OFFSET;
    FaceRender(cube->faces[CUBE_RED], CUBE_RED, x, y, size);
}

enum8(CubeColour) FaceGetTile(u32 face, u8 position) {
    assert(position < FACE_TILE_COUNT);

    // Calculate tile bit offset. Each tile is 4 bits so offset = position * 4
    u32 tile_offset = position << 2;

    return (face & (BITMASK_TILE << tile_offset)) >> tile_offset;
}

enum8(CubeColour) FaceSetTile(u32* face, enum8(CubeColour) colour, u8 position) {
    assert(colour < CUBE_COLOUR_COUNT);
    assert(position < FACE_TILE_COUNT);

    // Calculate tile bit offset. Each tile is 4 bits so offset = position * 4
    u32 tile_offset = position << 2;

    // Save old colour for return value
    CubeColour old_colour = (*face & (BITMASK_TILE << tile_offset)) >> tile_offset;

    // Clear tile bits
    *face &= ~(BITMASK_TILE << tile_offset);

    // Write tile bits
    *face |= (colour & BITMASK_TILE) << tile_offset;

    return old_colour;
}

void FaceRender(u32 face, enum8(CubeColour) colour, int x, int y, int size) {
    int spacing = size * (1 + TILE_RENDER_SPACING);
    Rectangle tile_rect = (Rectangle) { x, y, size, size };

    // Top row
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 0)]
    );
    tile_rect.x += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 1)]
    );
    tile_rect.x += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 2)]
    );

    // Middle row
    tile_rect.x = x;
    tile_rect.y += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 7)]
    );
    tile_rect.x += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[colour]
    );
    tile_rect.x += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 3)]
    );

    // Bottom row
    tile_rect.x = x;
    tile_rect.y += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 6)]
    );
    tile_rect.x += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 5)]
    );
    tile_rect.x += spacing;
    DrawRectangleRounded(
        tile_rect, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        cube_colour_table[FaceGetTile(face, 4)]
    );
}
