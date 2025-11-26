#ifndef INPUT_H
#define INPUT_H


#include "core.h"
#include "raylib.h"


typedef enum {
    INPUT_FRONT,
    INPUT_RIGHT,
    INPUT_UP,
    INPUT_BACK,
    INPUT_LEFT,
    INPUT_DOWN,

    INPUT_PRIME,
    INPUT_DOUBLE,

    INPUT_SHUFFLE,
    INPUT_RESET,
    INPUT_SOLVE,

    INPUT_ACTION_COUNT
} InputAction;


bool InputPressed(enum8(InputAction) action);
bool InputPressedRepeat(enum8(InputAction) action);
bool InputReleased(enum8(InputAction) action);
bool InputDown(enum8(InputAction) action);
bool InputUp(enum8(InputAction) action);


#endif  /* INPUT_H */
