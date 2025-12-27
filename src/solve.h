#ifndef SOLVE_H
#define SOLVE_H


#include "core.h"
#include "cube.h"


DECLARE_TYPED_STACK(TurnType, MoveStack)
DECLARE_TYPED_QUEUE(u32, QueueU32)


MoveStack* SolveCube(Arena* arena, Cube* cube);


#endif  /* SOLVE_H */
