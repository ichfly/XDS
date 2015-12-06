#ifndef _COMMON_H
#define _COMMON_H

#include <string>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include "Platform.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef unsigned int uint;
typedef signed int sint;

typedef uint8_t  bit8;
typedef uint16_t bit16;
typedef uint32_t bit32;
typedef uint64_t bit64;




#ifdef __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

#define notcritical(x) (x)

#include "Log.h"

#endif
