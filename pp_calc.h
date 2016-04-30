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

f64 pp_calc_acc(f64 aim, f64 speed, beatmap& b, f64 acc_percent, 
	u32 used_mods=mods::nomod, u16 combo = 0xFFFF, u16 misses = 0, 
	u32 score_version = 1);

f64 pp_calc(f64 aim, f64 speed, beatmap& b, 
	u32 used_mods=mods::nomod, 
	u16 combo = 0xFFFF, u16 misses = 0, u16 c300 = 0xFFFF, 
	u16 c100 = 0, u16 c50 = 0, u32 score_version = 1);
// 0xFFFF means max combo for combo and automatically calculates
// 300s based on 100, 50, miss count for c300
