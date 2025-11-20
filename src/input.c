#include "input.h"


static const KeyboardKey input_keymap_table[INPUT_ACTION_COUNT] = {
    KEY_F,
    KEY_R,
    KEY_U,
    KEY_B,
    KEY_L,
    KEY_D,

    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,

    KEY_S,
    KEY_SPACE
};


bool InputPressed(enum8(InputAction) action) {
    return IsKeyPressed(input_keymap_table[action]);
}

bool InputPressedRepeat(enum8(InputAction) action) {
    return IsKeyPressedRepeat(input_keymap_table[action]);
}

bool InputReleased(enum8(InputAction) action) {
    return IsKeyReleased(input_keymap_table[action]);
}

bool InputDown(enum8(InputAction) action) {
    return IsKeyDown(input_keymap_table[action]);
}

bool InputUp(enum8(InputAction) action) {
    return IsKeyUp(input_keymap_table[action]);
}

