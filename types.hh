#pragma once

typedef float   f32;
typedef double  f64;

#ifndef OPPAI_STDINT
typedef unsigned int       u32;
typedef unsigned short int u16;
typedef unsigned char      u8;

typedef int         i32;
typedef short int   i16;
typedef signed char i8;
#else
#include <stdint.h>

typedef int_least32_t   i32;
typedef int_least16_t   i16;
typedef int_least8_t    i8;

typedef uint_least32_t  u32;
typedef uint_least16_t  u16;
typedef uint_least8_t   u8;
#endif

#define fi32 "d"
#define fi16 "hd"

#define fu32 "u"
#define fu16 "hu"

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

#if !defined(OPPAI_SSIZE_T_DEFINED)
typedef SSIZE_T ssize_t;
#endif
#endif

