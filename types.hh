#pragma once

#if _MSC_VER >= 1800
#include <cinttypes>
#else
#include <cstdint>
#endif

typedef float   f32;
typedef double  f64;

typedef int_least64_t   i64;
typedef int_least32_t   i32;
typedef int_least16_t   i16;
typedef int_least8_t    i8;

typedef uint_least64_t  u64;
typedef uint_least32_t  u32;
typedef uint_least16_t  u16;
typedef uint_least8_t   u8;

#if _MSC_VER >= 1800
#define fi64 PRIdLEAST64
#else
#define fi64 "lld"
#endif
#define fi32 "d"
#define fi16 "hd"
#define fi8  "hhd"

#if _MSC_VER >= 1800
#define fu64 PRIuLEAST64
#else
#define fu64 "llu"
#endif
#define fu32 "u"
#define fu16 "hu"
#define fu8  "hhu"

