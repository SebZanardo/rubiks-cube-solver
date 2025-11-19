#include "arena.h"


void ArenaInit(Arena* arena, u64 size) {
    assert(arena != NULL);

    arena->base = malloc(size);
    arena->size = size;
    arena->used = 0;
}

void ArenaFree(Arena* arena) {
    assert(arena != NULL);

    free(arena->base);
    arena->base = NULL;
    arena->size = 0;
    arena->used = 0;
}

internal u64 ArenaAlignForward(Arena* arena, u64 align) {
    assert(arena != NULL);
    assert(arena->base != NULL);

    u64 current = (u64)(arena->base + arena->used);

    u64 misalignment = current % align;
    if (misalignment == 0) return arena->used;

    u64 adjustment = align - misalignment;
    if (arena->used + adjustment > arena->size) return UINT64_MAX;

    return arena->used + adjustment;
}

void* ArenaAlloc(Arena* arena, u64 size) {
    assert(arena != NULL);
    assert(arena->base != NULL);

    // Default alignment to 8
    u64 aligned_used = ArenaAlignForward(arena, 8);
    if (aligned_used == UINT64_MAX) return NULL;
    if (aligned_used + size > arena->size) return NULL;

    void* ptr = arena->base + aligned_used;
    arena->used = aligned_used + size;
    return ptr;
}

void* ArenaAllocAligned(Arena* arena, u64 size, u64 align) {
    assert(arena != NULL);
    assert(arena->base != NULL);

    u64 aligned_used = ArenaAlignForward(arena, 8);
    if (aligned_used == UINT64_MAX) return NULL;
    if (aligned_used + size > arena->size) return NULL;

    void* ptr = arena->base + aligned_used;
    arena->used = aligned_used + size;
    return ptr;
}


void ArenaReset(Arena* arena) {
    assert(arena != NULL);

    arena->used = 0;
}
