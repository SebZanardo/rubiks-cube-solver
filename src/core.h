#ifndef CORE_H
#define CORE_H


#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


typedef int8_t          i8 ;
typedef int16_t         i16;
typedef int32_t         i32;
typedef int64_t         i64;

typedef uint8_t         u8 ;
typedef uint16_t        u16;
typedef uint32_t        u32;
typedef uint64_t        u64;


#define internal        static
#define local_variable  static
#define global_variable static


#define enum8(type)     u8
#define enum16(type)    u16
#define enum32(type)    u32
#define enum64(type)    u64


#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MOD(x, n) ((((x) % (n)) + (n)) % (n))


#endif  /* CORE_H */
