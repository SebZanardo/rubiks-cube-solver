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
static const u8 CUBE_FACE_TILE_INDEX_TABLE[3][3] = {
    {0, 1, 2},
    {7, 8, 3},
    {6, 5, 4}
};
static const enum8(CubeColour) CUBE_FACE_COLOUR_TABLE[3][4] = {
    {CUBE_COLOUR_COUNT, CUBE_WHITE,     CUBE_COLOUR_COUNT,  CUBE_COLOUR_COUNT},
    {CUBE_ORANGE,       CUBE_GREEN,     CUBE_RED,           CUBE_BLUE},
    {CUBE_COLOUR_COUNT, CUBE_YELLOW,    CUBE_COLOUR_COUNT,  CUBE_COLOUR_COUNT}
};
static const u8 CUBE_FACE_ROTATION_TABLE[2][4] = {
    {0, 2, 4, 6},
    {1, 3, 5, 7}
};
static const enum8(CubeColour) CUBE_SIDE_COLOUR_TABLE[CUBE_COLOUR_COUNT][4] = {
    {CUBE_ORANGE, CUBE_WHITE, CUBE_RED, CUBE_YELLOW},   // Green Face
    {CUBE_GREEN, CUBE_WHITE, CUBE_BLUE, CUBE_YELLOW},   // Red Face
    {CUBE_ORANGE, CUBE_BLUE, CUBE_RED, CUBE_GREEN},     // White Face
    {CUBE_ORANGE, CUBE_YELLOW, CUBE_RED, CUBE_WHITE},   // Blue Face
    {CUBE_BLUE, CUBE_WHITE, CUBE_GREEN, CUBE_YELLOW},   // Orange Face
    {CUBE_ORANGE, CUBE_GREEN, CUBE_RED, CUBE_BLUE}      // Yellow Face
};
static const u8 CUBE_SIDE_ROTATION_TABLE[CUBE_COLOUR_COUNT][3][4] = {
    { {4, 6, 0, 2}, {3, 5, 7, 1}, {2, 4, 6, 0} },       // Green Face
    { {4, 4, 0, 4}, {3, 3, 7, 3}, {2, 2, 6, 2} },       // Red Face
    { {2, 2, 2, 2}, {1, 1, 1, 1}, {0, 0, 0, 0} },       // White Face
    { {0, 6, 4, 2}, {7, 5, 3, 1}, {6, 4, 2, 0} },       // Blue Face
    { {4, 0, 0, 0}, {3, 7, 7, 7}, {2, 6, 6, 6} },       // Orange Face
    { {6, 6, 6, 6}, {5, 5, 5, 5}, {4, 4, 4, 4} }        // Yellow Face
};
static const enum8(CubeColour) CUBE_EDGE_COLOUR_TABLE[12][2] = {
    { CUBE_GREEN, CUBE_WHITE },
    { CUBE_GREEN, CUBE_RED },
    { CUBE_GREEN, CUBE_YELLOW },
    { CUBE_GREEN, CUBE_ORANGE },

    { CUBE_RED, CUBE_WHITE },
    { CUBE_YELLOW, CUBE_RED },
    { CUBE_ORANGE, CUBE_YELLOW },
    { CUBE_WHITE, CUBE_ORANGE },

    { CUBE_BLUE, CUBE_WHITE },
    { CUBE_BLUE, CUBE_ORANGE },
    { CUBE_BLUE, CUBE_YELLOW },
    { CUBE_BLUE, CUBE_RED }
};
static const u8 CUBE_EDGE_POSITION_TABLE[12][2] = {
    { 1, 5 }, { 3, 7 }, { 5, 1 }, { 7, 3 },
    { 1, 3 }, { 3, 5 }, { 5, 7 }, { 7, 1 },
    { 1, 1 }, { 3, 7 }, { 5, 5 }, { 7, 3 }
};
static const enum8(CubeColour) CUBE_CORNER_COLOUR_TABLE[8][3] = {
    { CUBE_GREEN, CUBE_ORANGE, CUBE_WHITE },
    { CUBE_GREEN, CUBE_WHITE, CUBE_RED },
    { CUBE_GREEN, CUBE_RED, CUBE_YELLOW },
    { CUBE_GREEN, CUBE_YELLOW, CUBE_ORANGE },
    { CUBE_BLUE, CUBE_RED, CUBE_WHITE },
    { CUBE_BLUE, CUBE_WHITE, CUBE_ORANGE },
    { CUBE_BLUE, CUBE_ORANGE, CUBE_YELLOW },
    { CUBE_BLUE, CUBE_YELLOW, CUBE_RED }
};
static const u8 CUBE_CORNER_POSITION_TABLE[8][3] = {
    { 0, 2, 6 }, { 2, 4, 0 }, { 4, 6, 2 }, { 6, 0, 4 },
    { 0, 2, 2 }, { 2, 0, 0 }, { 4, 6, 6 }, { 6, 4, 4 }
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

static void TileRender(Rectangle rec, enum8(CubeColour) colour, bool valid) {
    DrawRectangleRounded(
        rec, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        CUBE_COLOUR_TABLE[colour]
    );
    Color outline_colour = valid ? BLACK : MAGENTA;
    DrawRectangleRoundedLinesEx(
        rec, TILE_RENDER_ROUNDNESS, TILE_RENDER_SEGMENTS,
        rec.width * TILE_RENDER_THICKNESS, outline_colour
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

static void FaceRender(
    Cube* cube, enum8(CubeColour) face_colour, int x, int y, int size, bool valid
) {
    u32 face = cube->faces[face_colour];
    int spacing = size * (1 + TILE_RENDER_SPACING);
    Rectangle tile_rect = (Rectangle) { x, y, size, size };

    for (int y = 0; y < 3; y++) {
        tile_rect.x = x;
        for (int x = 0; x < 3; x++) {
            u8 tile_index = CUBE_FACE_TILE_INDEX_TABLE[y][x];

            if (tile_index < FACE_TILE_COUNT) {
                TileRender(tile_rect, FaceGetTile(face, tile_index), valid);
            } else {
                // Centre tile is fixed colour
                TileRender(tile_rect, face_colour, valid);
            }
            tile_rect.x += spacing;
        }
        tile_rect.y += spacing;
    }
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

void CubeRender(Cube* cube, Rectangle cube_rect, bool valid) {
    int size = MinFloat(
        cube_rect.width / CUBE_RENDER_WIDTH,
        cube_rect.height / CUBE_RENDER_HEIGHT
    );
    IntVector2 offset = (IntVector2) {
        (cube_rect.width - CUBE_RENDER_WIDTH * size) / 2,
        (cube_rect.height - CUBE_RENDER_HEIGHT * size) / 2
    };
    IntVector2 position = (IntVector2) {
        cube_rect.x + offset.x + size * FACE_RENDER_OFFSET,
        cube_rect.y + offset.y + size * TILE_RENDER_SPACING
    };

    for (int y = 0; y < 3; y++) {
        position.x = cube_rect.x + offset.x + size * TILE_RENDER_SPACING;
        for (int x = 0; x < 4; x++) {
            CubeColour face_colour = CUBE_FACE_COLOUR_TABLE[y][x];
            if (face_colour < CUBE_COLOUR_COUNT) {
                FaceRender(cube, face_colour, position.x, position.y, size, valid);
            }
            position.x += size * FACE_RENDER_SPACING;
        }
        position.y += size * FACE_RENDER_SPACING;
    }
}

Color CubeFaceColour(enum8(CubeColour) colour) {
    assert(colour < CUBE_COLOUR_COUNT);
    return CUBE_COLOUR_TABLE[colour];
}

void CubeMousePaint(
    Cube* cube, Vector2 mouse_position, CubeColour colour, Rectangle cube_rect
) {
    int size = MinFloat(
        cube_rect.width / CUBE_RENDER_WIDTH,
        cube_rect.height / CUBE_RENDER_HEIGHT
    );
    IntVector2 offset = (IntVector2) {
        (cube_rect.width - CUBE_RENDER_WIDTH * size) / 2,
        (cube_rect.height - CUBE_RENDER_HEIGHT * size) / 2
    };
    IntVector2 offset_mouse = (IntVector2) {
        (mouse_position.x - cube_rect.x - offset.x),
        (mouse_position.y - cube_rect.y - offset.y)
    };
    IntVector2 face_position = (IntVector2) {
         offset_mouse.x / (size * FACE_RENDER_SPACING),
         offset_mouse.y / (size * FACE_RENDER_SPACING)
    };
    IntVector2 tile_position = (IntVector2) {
        (offset_mouse.x - (face_position.x * size * FACE_RENDER_SPACING)) / (size * (1 + TILE_RENDER_SPACING)),
        (offset_mouse.y - (face_position.y * size * FACE_RENDER_SPACING)) / (size * (1 + TILE_RENDER_SPACING)),
    };

    if (
        face_position.x >= 0 && face_position.x < 4 &&
        face_position.y >= 0 && face_position.y < 3 &&
        tile_position.x >= 0 && tile_position.x < 3 &&
        tile_position.y >= 0 && tile_position.y < 3
    ) {
        CubeColour face_colour = CUBE_FACE_COLOUR_TABLE[face_position.y][face_position.x];
        u8 tile_index = CUBE_FACE_TILE_INDEX_TABLE[tile_position.y][tile_position.x];
        if (tile_index < FACE_TILE_COUNT) {
            FaceSetTile(&cube->faces[face_colour], colour, tile_index);
        }
    }
}

bool CubeValid(Cube* cube) {
    // Store where edges are for permutation parity test later
    // Piece at target position i -> currently at position j
    u8 edge_positions[12] = {0};
    u8 corner_positions[8] = {0};

    // Check all edges exist
    // Count edge parity (+1 flipped. total % 2 == 0)
    u16 seen = 0xF000;  // Highest bits set to true as only 12 edges
    u8 parity = 0;

    for (int i = 0; i < 12; i++) {
        CubeColour face_a = CUBE_EDGE_COLOUR_TABLE[i][0];
        CubeColour face_b = CUBE_EDGE_COLOUR_TABLE[i][1];
        u8 tile_index_a = CUBE_EDGE_POSITION_TABLE[i][0];
        u8 tile_index_b = CUBE_EDGE_POSITION_TABLE[i][1];

        // Current face colours
        CubeColour a = FaceGetTile(cube->faces[face_a], tile_index_a);
        CubeColour b = FaceGetTile(cube->faces[face_b], tile_index_b);

        // Invalid if edge tiles are same colour
        if (a == b) return false;

        // Invalid if edge tiles are opposite colours
        CubeColour opposite_a = (a + 3) % CUBE_COLOUR_COUNT;
        if (opposite_a == b) return false;

        // Now look through all colour combos until match
        int j = 0;
        for (j = 0; j < 12; j++) {
            CubeColour target_a = CUBE_EDGE_COLOUR_TABLE[j][0];
            CubeColour target_b = CUBE_EDGE_COLOUR_TABLE[j][1];

            if (target_a == a && target_b == b) {
                // + 0, correct rotation
                break;
            }
            if (target_a == b && target_b == a) {
                // + 1, flipped rotation
                parity++;
                break;
            }
        }

        // Set edge type seen
        FlagToggle(seen, Bit(j));

        // If not true then second time seeing this edge and invalid
        if (!BitActive(seen, j)) return false;

        // Store where current position of edge is
        edge_positions[i] = j;
    }

    // Ensure all edge types seen
    if (seen != 0xFFFF) return false;

    // Failed edge parity test
    if (parity % 2 != 0) return false;


    // Check all corners exist
    // Count corner parity (+1 clockwise, +2 anti-clockwise. total % 3 == 0)
    seen = 0xFF00;
    parity = 0;

    for (int i = 0; i < 8; i++) {
        CubeColour face_a = CUBE_CORNER_COLOUR_TABLE[i][0];
        CubeColour face_b = CUBE_CORNER_COLOUR_TABLE[i][1];
        CubeColour face_c = CUBE_CORNER_COLOUR_TABLE[i][2];
        u8 tile_index_a = CUBE_CORNER_POSITION_TABLE[i][0];
        u8 tile_index_b = CUBE_CORNER_POSITION_TABLE[i][1];
        u8 tile_index_c = CUBE_CORNER_POSITION_TABLE[i][2];

        // Current face colours
        CubeColour a = FaceGetTile(cube->faces[face_a], tile_index_a);
        CubeColour b = FaceGetTile(cube->faces[face_b], tile_index_b);
        CubeColour c = FaceGetTile(cube->faces[face_c], tile_index_c);

        // Invalid if corner tiles are same colour
        if (a == b || a == c || b == c) return false;

        // Invalid if corner tiles are opposite colours
        CubeColour opposite_a = (a + 3) % CUBE_COLOUR_COUNT;
        CubeColour opposite_b = (b + 3) % CUBE_COLOUR_COUNT;
        if (opposite_a == b || opposite_a == c || opposite_b == c) return false;

        // Now look through all colour combos until match
        int j = 0;
        for (j = 0; j < 8; j++) {
            CubeColour target_a = CUBE_CORNER_COLOUR_TABLE[j][0];
            CubeColour target_b = CUBE_CORNER_COLOUR_TABLE[j][1];
            CubeColour target_c = CUBE_CORNER_COLOUR_TABLE[j][2];

            if (target_a == a && target_b == b && target_c == c) {
                // + 0, correct rotation
                break;
            }
            if (target_a == b && target_b == c && target_c == a) {
                // + 1, clockwise rotation
                parity++;
                break;
            }
            if (target_a == c && target_b == a && target_c == b) {
                // + 2, anticlockwise rotation
                parity += 2;
                break;
            }
        }

        // Set corner type seen
        FlagToggle(seen, Bit(j));

        // If not true then second time seeing this corner and invalid
        if (!BitActive(seen, j)) return false;

        // Store where current position of corner is
        corner_positions[i] = j;
    }

    // Ensure all corner types seen
    if (seen != 0xFFFF) return false;

    // Failed corner parity test
    if (parity % 3 != 0) return false;

    // TODO: Pemutation parity

    return true;
}
