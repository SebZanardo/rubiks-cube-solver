#include "cube.h"


static const int CUBE_FACE_COUNT = 6;
static const int FACE_TILE_COUNT = 8;
static const int SIDE_TURN_COUNT = 4;
static const u8 BITMASK_TILE = 0xFu;  // Four bits set to true, 1111

static const float TILE_RENDER_SPACING = 0.25f;
static const float TILE_RENDER_ROUNDNESS = 0.3f;
static const int TILE_RENDER_SEGMENTS = 4;
static const float TILE_RENDER_THICKNESS = 0.075f;

static const float FACE_RENDER_SPACING = 4 + (TILE_RENDER_SPACING * 2);
static const float FACE_RENDER_OFFSET = FACE_RENDER_SPACING + TILE_RENDER_SPACING;

static const float CUBE_RENDER_WIDTH = 15 + (TILE_RENDER_SPACING * 10);
static const float CUBE_RENDER_HEIGHT = 11 + (TILE_RENDER_SPACING * 8);

/*
static const int GODS_NUMBER = 20;  // Maximum number of turns to solve any 3x3
static const int GODS_NUMBER_QUARTER = 26;
*/


// Lookup tables
static const Color CUBE_COLOUR_TABLE[CUBE_COLOUR_COUNT] = {
    (Color) { 0,   255, 0,   255 },
    (Color) { 255, 0,   0,   255 },
    (Color) { 255, 255, 255, 255 },
    (Color) { 0,   0,   255, 255 },
    (Color) { 255, 155, 0,   255 },
    (Color) { 255, 255, 0,   255 },
};
static const u8 CUBE_FACE_ROTATION_TABLE[2][4] = {
    {0, 2, 4, 6},
    {1, 3, 5, 7}
};
static const enum8(CubeColour) CUBE_SIDE_COLOUR_TABLE[CUBE_COLOUR_COUNT][4] = {
    // Green Face
    {CUBE_ORANGE, CUBE_WHITE, CUBE_RED, CUBE_YELLOW},
    // Red Face
    {CUBE_GREEN, CUBE_WHITE, CUBE_BLUE, CUBE_YELLOW},
    // White Face
    {CUBE_ORANGE, CUBE_BLUE, CUBE_RED, CUBE_GREEN},
    // Blue Face
    {CUBE_ORANGE, CUBE_YELLOW, CUBE_RED, CUBE_WHITE},
    // Orange Face
    {CUBE_BLUE, CUBE_WHITE, CUBE_GREEN, CUBE_YELLOW},
    // Yellow Face
    {CUBE_ORANGE, CUBE_GREEN, CUBE_RED, CUBE_BLUE},
};
static const u8 CUBE_SIDE_ROTATION_TABLE[CUBE_COLOUR_COUNT][3][4] = {
    // Green Face
    { {4, 6, 0, 2}, {3, 5, 7, 1}, {2, 4, 6, 0} },
    // Red Face
    { {4, 4, 0, 4}, {3, 3, 7, 3}, {2, 2, 6, 2} },
    // White Face
    { {2, 2, 2, 2}, {1, 1, 1, 1}, {0, 0, 0, 0} },
    // Blue Face
    { {0, 6, 4, 2}, {7, 5, 3, 1}, {6, 4, 2, 0} },
    // Orange Face
    { {4, 0, 0, 0}, {3, 6, 6, 6}, {2, 7, 7, 7} },
    // Yellow Face
    { {6, 6, 6, 6}, {5, 5, 5, 5}, {4, 4, 4, 4} }
};
/*
static const char CUBE_COLOUR_CHARS[CUBE_COLOUR_COUNT] = "GRWBOY";
static char *CUBE_COLOUR_NAMES[CUBE_COLOUR_COUNT] = {
    "GREEN", "RED", "WHITE", "BLUE", "ORANGE", "YELLOW"
};
static const char *TURN_TYPE_NAMES[TURN_TYPE_COUNT] = {
    "F",  "R",  "U",  "B",  "L",  "D",
    "F'", "R'", "U'", "B'", "L'", "D'",
    "F2", "R2", "U2", "B2", "L2", "D2"
};
*/


static void TileRender(Rectangle rec, enum8(CubeColour) colour) {
    DrawRectangleRounded(
        rec, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        CUBE_COLOUR_TABLE[colour]
    );
    DrawRectangleRoundedLinesEx(
        rec, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        rec.width * TILE_RENDER_THICKNESS, BLACK
    );
}

enum8(CubeColour) FaceGetTile(u32 face, u8 position) {
    assert(position < FACE_TILE_COUNT);

    // Calculate tile bit offset. Each tile is 4 bits so offset = position * 4
    u32 offset = position << 2;

    return FlagGet(face, BITMASK_TILE << offset) >> offset;
}

enum8(CubeColour) FaceSetTile(u32* face, enum8(CubeColour) colour, u8 position) {
    assert(colour < CUBE_COLOUR_COUNT);
    assert(position < FACE_TILE_COUNT);

    // Calculate tile bit offset. Each tile is 4 bits so offset = position * 4
    u32 offset = position << 2;

    // Save old colour for return value
    CubeColour old_colour = FlagGet(*face, BITMASK_TILE << offset) >> offset;

    FlagClear(*face, BITMASK_TILE << offset);
    FlagSet(*face, colour << offset);

    return old_colour;
}

static void FaceRender(Cube* cube, enum8(CubeColour) face_colour, int x, int y, int size) {
    u32 face = cube->faces[face_colour];
    int spacing = size * (1 + TILE_RENDER_SPACING);
    Rectangle tile_rect = (Rectangle) { x, y, size, size };

    // Top row
    TileRender(tile_rect, FaceGetTile(face, 0));
    tile_rect.x += spacing;
    TileRender(tile_rect, FaceGetTile(face, 1));
    tile_rect.x += spacing;
    TileRender(tile_rect, FaceGetTile(face, 2));

    // Middle row
    tile_rect.x = x;
    tile_rect.y += spacing;
    TileRender(tile_rect, FaceGetTile(face, 7));
    tile_rect.x += spacing;
    TileRender(tile_rect, face_colour);
    tile_rect.x += spacing;
    TileRender(tile_rect, FaceGetTile(face, 3));

    // Bottom row
    tile_rect.x = x;
    tile_rect.y += spacing;
    TileRender(tile_rect, FaceGetTile(face, 6));
    tile_rect.x += spacing;
    TileRender(tile_rect, FaceGetTile(face, 5));
    tile_rect.x += spacing;
    TileRender(tile_rect, FaceGetTile(face, 4));
}

void CubeInit(Arena* arena, Cube* cube) {
    cube->faces = ArenaPushArray(arena, CUBE_FACE_COUNT, u32);
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
    // Rotate face
    for (int j = 0; j < 2; j++) {
        const u8* position_lookup = CUBE_FACE_ROTATION_TABLE[j];

        u8 side_position = position_lookup[0];
        CubeColour temp = FaceGetTile(cube->faces[face_colour], side_position);

        for (int i = 1; i <= SIDE_TURN_COUNT; i++) {
            u8 wrapped = i % SIDE_TURN_COUNT;
            side_position = position_lookup[wrapped];
            temp = FaceSetTile(&cube->faces[face_colour], temp, side_position);
        }
    }

    // Rotate sides of face
    for (int j = 0; j < 3; j++) {
        const enum8(CubeColour)* colour_lookup = CUBE_SIDE_COLOUR_TABLE[face_colour];
        const u8* position_lookup = CUBE_SIDE_ROTATION_TABLE[face_colour][j];

        CubeColour side_colour = colour_lookup[0];
        u8 side_position = position_lookup[0];
        CubeColour temp = FaceGetTile(cube->faces[side_colour], side_position);

        for (int i = 1; i <= SIDE_TURN_COUNT;  i++) {
            u8 wrapped = i % SIDE_TURN_COUNT;
            side_colour = colour_lookup[wrapped];
            side_position = position_lookup[wrapped];
            temp = FaceSetTile(&cube->faces[side_colour], temp, side_position);
        }
    }
}

void CubeFaceTurnAntiClockwise(Cube* cube, enum8(CubeColour) face_colour) {
    // Rotate face
    for (int j = 0; j < 2; j++) {
        const u8* position_lookup = CUBE_FACE_ROTATION_TABLE[j];

        u8 side_position = position_lookup[0];
        CubeColour temp = FaceGetTile(cube->faces[face_colour], side_position);

        for (int i = SIDE_TURN_COUNT; i > -1; i--) {
            u8 wrapped = ModWrap(i, SIDE_TURN_COUNT);
            side_position = position_lookup[wrapped];
            temp = FaceSetTile(&cube->faces[face_colour], temp, side_position);
        }
    }

    // Rotate sides of face
    for (int j = 0; j < 3; j++) {
        const enum8(CubeColour)* colour_lookup = CUBE_SIDE_COLOUR_TABLE[face_colour];
        const u8* position_lookup = CUBE_SIDE_ROTATION_TABLE[face_colour][j];

        CubeColour side_colour = colour_lookup[0];
        u8 side_position = position_lookup[0];
        CubeColour temp = FaceGetTile(cube->faces[side_colour], side_position);

        for (int i = SIDE_TURN_COUNT; i > -1; i--) {
            u8 wrapped = ModWrap(i, SIDE_TURN_COUNT);
            side_colour = colour_lookup[wrapped];
            side_position = position_lookup[wrapped];
            temp = FaceSetTile(&cube->faces[side_colour], temp, side_position);
        }
    }
}

void CubeFaceTurnDouble(Cube* cube, enum8(CubeColour) face_colour) {
    // Rotate face
    for (int j = 0; j < 2; j++) {
        const u8* position_lookup = CUBE_FACE_ROTATION_TABLE[j];

        for (int i = 0; i < (SIDE_TURN_COUNT / 2); i++) {
            u8 a = position_lookup[i];
            u8 b = position_lookup[i + (SIDE_TURN_COUNT / 2)];

            CubeColour temp = FaceGetTile(cube->faces[face_colour], a);
            temp = FaceSetTile(&cube->faces[face_colour], temp, b);
            FaceSetTile(&cube->faces[face_colour], temp, a);
        }
    }

    // Rotate sides of face
    for (int j = 0; j < 3; j++) {
        const enum8(CubeColour)* colour_lookup = CUBE_SIDE_COLOUR_TABLE[face_colour];
        const u8* position_lookup = CUBE_SIDE_ROTATION_TABLE[face_colour][j];

        u8 half = (SIDE_TURN_COUNT / 2);
        for (int i = 0; i < half; i++) {
            CubeColour a_colour = colour_lookup[i];
            u8 a = position_lookup[i];
            CubeColour b_colour = colour_lookup[i + half];
            u8 b = position_lookup[i + half];

            CubeColour temp = FaceGetTile(cube->faces[a_colour], a);
            temp = FaceSetTile(&cube->faces[b_colour], temp, b);
            FaceSetTile(&cube->faces[a_colour], temp, a);
        }
    }
}

void CubeRender(Cube* cube, int offset_x, int offset_y, int width, int height) {
    int size = MinFloat(
        (width / CUBE_RENDER_WIDTH),
        (height / CUBE_RENDER_HEIGHT)
    );
    int offset_left = (width - CUBE_RENDER_WIDTH * size) / 2;
    int offset_top = (height - CUBE_RENDER_HEIGHT * size) / 2;
    int x = offset_x + offset_left + size * FACE_RENDER_OFFSET;
    int y = offset_y + offset_top + size * TILE_RENDER_SPACING;

    // Up
    FaceRender(cube, CUBE_WHITE, x, y, size);

    // Left, Front, Right, Back
    x = offset_x + offset_left + size * TILE_RENDER_SPACING;
    y += size * FACE_RENDER_SPACING;
    FaceRender(cube, CUBE_ORANGE, x, y, size);
    x += size * FACE_RENDER_SPACING;
    FaceRender(cube, CUBE_GREEN, x, y, size);
    x += size * FACE_RENDER_SPACING;
    FaceRender(cube, CUBE_RED, x, y, size);
    x += size * FACE_RENDER_SPACING;
    FaceRender(cube, CUBE_BLUE, x, y, size);

    // Down
    x = offset_x + offset_left + size * FACE_RENDER_OFFSET;
    y += size * FACE_RENDER_SPACING;
    FaceRender(cube, CUBE_YELLOW, x, y, size);
}
