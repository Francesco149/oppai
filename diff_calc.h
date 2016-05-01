#pragma once
#include "types.h"

struct beatmap;

// calculates overall, aim and speed stars for a map.
// aim, speed: pointers to the variables where 
//             aim and speed stars will be stored.
// returns overall stars
f64 d_calc(beatmap& b, f64* aim, f64* speed);
