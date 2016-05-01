#pragma once

#include "types.h"

// these don't necessarily need to match the pp processor's bitmask 1:1 but 
// I'll keep them consistent just because
namespace mods {
	const u32 
		nomod = 0, 
		nf = 1 << 0,
		ez = 1 << 1,
		hd = 1 << 3,
		hr = 1 << 4,
		dt = 1 << 6,
		ht = 1 << 8,
		nc = 1 << 9,
		fl = 1 << 10,
		so = 1 << 12, 
		speed_changing = dt | ht | nc, 
		map_changing = hr | ez | speed_changing;
}

struct beatmap;

// rounds acc_percent to the closest possible 100-count and calculates ppv2.
//
// aim: aim difficulty
// speed: speed difficulty
// b: the beatmap with map-modifying mods already applied
// acc_percent: the desired accuracy in percent (0-100)
// used_mods: the mods bitmask (see namespace mods)
// combo: desired combo. 0xFFFF will assume full combo.
// misses: amount of misses
// score_version: 1 or 2, affects accuracy pp.
f64 pp_calc_acc(f64 aim, f64 speed, beatmap& b, f64 acc_percent, 
	u32 used_mods=mods::nomod, u16 combo = 0xFFFF, u16 misses = 0, 
	u32 score_version = 1);

// calculates ppv2.
//
// aim: aim difficulty
// speed: speed difficulty
// b: the beatmap with map-modifying mods already applied
// used_mods: the mods bitmask (see namespace mods)
// combo: desired combo. 0xFFFF will assume full combo.
// misses: amount of misses
// c300: amount of 300s. 0xFFFF will automatically calculate this value based on
//       the number of misses, 100s and 50s.
// c100, c50: number of 100s and 50s.
// score_version: 1 or 2, affects accuracy pp.
f64 pp_calc(f64 aim, f64 speed, beatmap& b, 
	u32 used_mods=mods::nomod, 
	u16 combo = 0xFFFF, u16 misses = 0, u16 c300 = 0xFFFF, 
	u16 c100 = 0, u16 c50 = 0, u32 score_version = 1);