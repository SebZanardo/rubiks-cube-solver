#ifndef ARENA_H
#define ARENA_H


#include "core.h"


typedef struct {
    u8* base;
    u64 size;
    u64 used;
} Arena;


void ArenaInit(Arena* arena, u64 size);
void ArenaFree(Arena* arena);
void* ArenaAlloc(Arena* arena, u64 size);
void* ArenaAllocAligned(Arena* arena, u64 size, u64 align);
void ArenaReset(Arena* arena);


#endif  /* ARENA_H */
