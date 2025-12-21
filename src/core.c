#include "core.h"


#define _Max(a, b) (((a) > (b)) ? (a) : (b))
#define _Min(a, b) (((a) < (b)) ? (a) : (b))
#define _Mod(x, n) ((((x) % (n)) + (n)) % (n))


static u64 ArenaAlignedOffset(Arena* arena, u64 align) {
    u64 current = (u64)(arena->base + arena->used);

    u64 misalignment = current % align;
    if (misalignment == 0) return arena->used;

    u64 adjustment = align - misalignment;
    if (arena->used + adjustment > arena->size) return UINT64_MAX;

    return arena->used + adjustment;
}

void* _ArenaPush(Arena* arena, u64 size, u64 align, bool clear) {
    assert(arena != NULL);
    assert(arena->base != NULL);

    u64 aligned_used = ArenaAlignedOffset(arena, align);

    if (aligned_used == UINT64_MAX || aligned_used + size > arena->size) {
        assert(false && "Arena out of memory!");
        return NULL;
    }

    void* ptr = arena->base + aligned_used;
    arena->used = aligned_used + size;

    if (clear) MemSet(ptr, ArenaDefaultZeroValue, size);

    return ptr;
}

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

void ArenaReset(Arena* arena) {
    assert(arena != NULL);

    arena->used = 0;
}

void* MemCopy(void* dest, void* src, u64 size) {
    u8* d = (u8*) dest;
    const u8* s = (const u8*) src;

    while (size--) {
        *d++ = *s++;
    }

    return dest;
}

void* MemSet(void* ptr, u8 value, u64 size) {
    u8 *dst = (u8*) ptr;

    while (size--) {
        *dst++ = (unsigned char)value;
    }

    return ptr;
}

i32 MemCmp(void* a, void* b, u64 count) {
    const u8 *p1 = (const u8*) a;
    const u8 *p2 = (const u8*) b;

    while (count--) {
        u8 c1 = *p1++;
        u8 c2 = *p2++;

        if (c1 != c2) {
            return (c1 < c2) ? -1 : 1;
        }
    }

    return 0;
}

u8  MinU8 (u8  a, u8  b) { return _Min(a, b); }
u16 MinU16(u16 a, u16 b) { return _Min(a, b); }
u32 MinU32(u32 a, u32 b) { return _Min(a, b); }
u64 MinU64(u64 a, u64 b) { return _Min(a, b); }
u8  MaxU8 (u8  a, u8  b) { return _Max(a, b); }
u16 MaxU16(u16 a, u16 b) { return _Max(a, b); }
u32 MaxU32(u32 a, u32 b) { return _Max(a, b); }
u64 MaxU64(u64 a, u64 b) { return _Max(a, b); }
u8  MinI8 (i8  a, i8  b) { return _Min(a, b); }
u16 MinI16(i16 a, i16 b) { return _Min(a, b); }
u32 MinI32(i32 a, i32 b) { return _Min(a, b); }
u64 MinI64(i64 a, i64 b) { return _Min(a, b); }
u8  MaxI8 (i8  a, i8  b) { return _Max(a, b); }
u16 MaxI16(i16 a, i16 b) { return _Max(a, b); }
u32 MaxI32(i32 a, i32 b) { return _Max(a, b); }
u64 MaxI64(i64 a, i64 b) { return _Max(a, b); }
float MinFloat(float a, float b) { return _Min(a, b); }
float MaxFloat(float a, float b) { return _Max(a, b); }
int MinInt(int a, int b) { return _Min(a, b); }
int MaxInt(int a, int b) { return _Max(a, b); }

int ModWrap(int x, int n) { return _Mod(x, n); }

int Index2D(int x, int y, int width, int height) {
    assert(x >= 0 && x < width && y >= 0 && y < height);
    return y * width + x;
}
