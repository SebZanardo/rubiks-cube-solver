#ifndef CORE_H
#define CORE_H


#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


typedef int8_t          i8 ;
typedef int16_t         i16;
typedef int32_t         i32;
typedef int64_t         i64;

typedef uint8_t         u8 ;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;


#define enum8(type)     u8
#define enum16(type)    u16
#define enum32(type)    u32
#define enum64(type)    u64


#define Bit(x)              (1 << (x))
#define BitActive(n, pos)   ((n) & Bit(pos))

#define FlagGet(n, flag)    ((n) & (flag))
#define FlagSet(n, flag)    ((n) |= (flag))
#define FlagClear(n, flag)  ((n) &= ~(flag))
#define FlagToggle(n, flag) ((n) ^= (flag))


#define Kilobytes(value)    ((value) * 1024LL)
#define Megabytes(value)    (Kilobytes(value) * 1024LL)
#define Gigabytes(value)    (Megabytes(value) * 1024LL)
#define Terabytes(value)    (Gigabytes(value) * 1024LL)

#define ToKilobytes(value)  ((u64)((value) / 1024.0f))
#define ToMegabytes(value)  ((u64)(ToKilobytes(value) / 1024.0f))
#define ToGigabytes(value)  ((u64)(ToMegabytes(value) / 1024.0f))
#define ToTerabytes(value)  ((u64)(ToGigabytes(value) / 1024.0f))


typedef struct {
    u8* base;
    u64 size;
    u64 used;
} Arena;

typedef struct {
    int x;
    int y;
} IntVector2;


#define ArenaDefaultAlignment 8
#define ArenaDefaultZeroValue 0
#define ArenaPush(arena, size) _ArenaPush(arena, size, ArenaDefaultAlignment, true)
#define ArenaPushStruct(arena, type) (type*) _ArenaPush(arena, sizeof(type), alignof(type), true)
#define ArenaPushArray(arena, count, type) (type*) _ArenaPush(arena, (count) * sizeof(type), alignof(type[1]), true)
void* _ArenaPush(Arena* arena, u64 size, u64 align, bool clear);
void ArenaInit(Arena* arena, u64 size);
void ArenaFree(Arena* arena);
void ArenaReset(Arena* arena);

void* MemCopy(void* dest, void* src, u64 size);
void* MemSet(void* ptr, u8 value, u64 size);
i32 MemCmp(void* a, void* b, u64 count);
#define MemZero(ptr, size) MemSet(ptr, 0, size)

u8  MinU8 (u8  a, u8  b);
u16 MinU16(u16 a, u16 b);
u32 MinU32(u32 a, u32 b);
u64 MinU64(u64 a, u64 b);
u8  MaxU8 (u8  a, u8  b);
u16 MaxU16(u16 a, u16 b);
u32 MaxU32(u32 a, u32 b);
u64 MaxU64(u64 a, u64 b);
u8  MinI8 (i8  a, i8  b);
u16 MinI16(i16 a, i16 b);
u32 MinI32(i32 a, i32 b);
u64 MinI64(i64 a, i64 b);
u8  MaxI8 (i8  a, i8  b);
u16 MaxI16(i16 a, i16 b);
u32 MaxI32(i32 a, i32 b);
u64 MaxI64(i64 a, i64 b);
float MinFloat(float a, float b);
float MaxFloat(float a, float b);
int MinInt(int a, int b);
int MaxInt(int a, int b);

int ModWrap(int x, int n);

int Index2D(int x, int y, int width, int height);

// STACK //////////////////////////////////////////////////////////////////////
#define DECLARE_TYPED_STACK(type, name)                                       \
    typedef struct {                                                          \
        type* items;                                                          \
        u32 head;                                                             \
        u32 capacity;                                                         \
    } name;                                                                   \
                                                                              \
    void name##_init(name* stack, type* items, u32 capacity);                 \
    bool name##_append(name* stack, type item);                               \
    bool name##_pop(name* stack, type* item);                                 \
    u32 name##_length(name* stack);                                           \
    void name##_clear(name* stack);                                           \

#define DEFINE_TYPED_STACK(type, name)                                        \
    void name##_init(name* stack, type* items, u32 capacity) {                \
        stack->items = items;                                                 \
        stack->capacity = capacity;                                           \
        name##_clear(stack);                                                  \
    }                                                                         \
                                                                              \
    bool name##_append(name* stack, type item) {                              \
        if (stack->head >= stack->capacity) return false;                     \
        stack->items[stack->head++] = item;                                   \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool name##_pop(name* stack, type* item) {                                \
        if (stack->head == 0) return false;                                   \
        *item = stack->items[stack->head--];                                  \
        return true;                                                          \
    }                                                                         \
                                                                              \
    u32 name##_length(name* stack) {                                          \
        return stack->head;                                                   \
    }                                                                         \
                                                                              \
    void name##_clear(name* stack) {                                          \
        stack->head = 0;                                                      \
    }                                                                         \

// QUEUE //////////////////////////////////////////////////////////////////////
#define DECLARE_TYPED_QUEUE(type, name)                                       \
    typedef struct {                                                          \
        type* items;                                                          \
        u32 head;                                                             \
        u32 tail;                                                             \
        u32 capacity;                                                         \
    } name;                                                                   \
                                                                              \
    void name##_init(name* queue, type* items, u32 capacity);                 \
    bool name##_append(name* queue, type item);                               \
    bool name##_pop(name* queue, type* item);                                 \
    u32 name##_length(name* queue);                                           \
    void name##_clear(name* queue);                                           \

#define DEFINE_TYPED_QUEUE(type, name)                                        \
    void name##_init(name* queue, type* items, u32 capacity) {                \
        queue->items = items;                                                 \
        queue->capacity = capacity;                                           \
        name##_clear(queue);                                                  \
    }                                                                         \
                                                                              \
    bool name##_append(name *queue, type item) {                              \
        u32 next = (queue->tail + 1) % queue->capacity;                       \
        if (next == queue->head) return false;                                \
        queue->items[queue->tail] = item;                                     \
        queue->tail = next;                                                   \
        return true;                                                          \
    }                                                                         \
                                                                              \
    bool name##_pop(name *queue, type *item) {                                \
        if (queue->tail == queue->head) return false;                         \
        *item = queue->items[queue->head];                                    \
        queue->head = (queue->head + 1) % queue->capacity;                    \
        return true;                                                          \
    }                                                                         \
                                                                              \
    u32 name##_length(name *queue) {                                          \
        return ((queue->tail+queue->capacity)-queue->head) % queue->capacity; \
    }                                                                         \
                                                                              \
    void name##_clear(name *queue) {                                          \
        queue->head = 0;                                                      \
        queue->tail = 0;                                                      \
    }                                                                         \


#endif  /* CORE_H */
