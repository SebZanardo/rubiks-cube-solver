#ifndef CUBE_H
#define CUBE_H


#include "core.h"
#include "raylib.h"


typedef enum {
    CUBE_GREEN,
    CUBE_RED,
    CUBE_WHITE,
    CUBE_BLUE,
    CUBE_ORANGE,
    CUBE_YELLOW,

    CUBE_COLOUR_COUNT
} CubeColour;

typedef enum {
    TURN_FRONT,
    TURN_RIGHT,
    TURN_UP,
    TURN_BACK,
    TURN_LEFT,
    TURN_DOWN,

    TURN_FRONT_PRIME,
    TURN_RIGHT_PRIME,
    TURN_UP_PRIME,
    TURN_BACK_PRIME,
    TURN_LEFT_PRIME,
    TURN_DOWN_PRIME,

    TURN_FRONT_DOUBLE,
    TURN_RIGHT_DOUBLE,
    TURN_UP_DOUBLE,
    TURN_BACK_DOUBLE,
    TURN_LEFT_DOUBLE,
    TURN_DOWN_DOUBLE,

    TURN_TYPE_COUNT
} TurnType;


// These need to be used by solve.c to locate edge positions for search
extern const enum8(CubeColour) CUBE_EDGE_COLOUR_TABLE[12 * 2];
extern const u8 CUBE_EDGE_POSITION_TABLE[12 * 2];

extern const char *TURN_TYPE_NAMES[TURN_TYPE_COUNT];


// There are six tile colours.
// Centre tiles never move so no need to store or simulate them.
// So, on every face there are 8 tiles that can change.
//
// colours * moving face tiles = bits per face
// (4bits) *        (8)        =   (32bits)
//
// To simplify rotations, tile winding order within u32 face is:
//
//  0 1 2
//  7   3
//  6 5 4
//
// Face colours are in the order Green, Red, White, Blue, Orange, Yellow:
//
//    W
//  O G R B
//    Y
//
// Turns are in the order Front, Right, Up, Back, Left, Down:
//
//    U
//  L F R B
//    D
//
// Tile indexes per face for cube:
//
//         0 1 2
//         7   3
//         6 5 4
//
//  0 1 2  0 1 2  0 1 2  0 1 2
//  7   3  7   3  7   3  7   3
//  6 5 4  6 5 4  6 5 4  6 5 4
//
//         0 1 2
//         7   3
//         6 5 4


typedef struct {
    u32* faces;
} Cube;


enum8(CubeColour) FaceGetTile(u32 face, u8 position);
void CubeInit(Arena* arena, Cube* cube);
void CubeSetSolved(Cube* cube);
void CubeHandScramble(Cube* cube);
void CubeTurn(Cube* cube, TurnType turn);
void CubeFaceTurnClockwise(Cube* cube, enum8(CubeColour) face_colour);
void CubeFaceTurnAntiClockwise(Cube* cube, enum8(CubeColour) face_colour);
void CubeFaceTurnDouble(Cube* cube, enum8(CubeColour) face_colour);
Color CubeFaceColour(enum8(CubeColour) colour);
void CubeMousePaint(Cube* cube, Vector2 mouse_position, CubeColour colour, Rectangle cube_rect);
bool CubeValid(Arena* arena_temp, Cube* cube);
void CubeRender(Cube* cube, Rectangle cube_rect, bool valid);


#endif  /* CUBE_H */
