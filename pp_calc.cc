// turns the beatmaps' strain attributes into a larger value, suitable 
// for pp calc. not 100% sure what is going on here, but it probably makes
// strain values scale a bit exponentially.
f64 base_strain(f64 strain) {
	return std::pow(5.0 * std::max(1.0, strain / 0.0675) - 4.0, 3.0) / 
		100000.0;
}

f64 acc_calc(u16 c300, u16 c100, u16 c50, u16 misses) {
	u16 total_hits = c300 + c100 + c50 + misses;
	f64 acc = 0.f;
	if (total_hits > 0) {
		acc = (
			c50 * 50.0 + c100 * 100.0 + c300 * 300.0) / 
			(total_hits * 300.0);
	}
	return acc;
}

struct pp_calc_result {
	f64 acc_percent;
	f64 pp;
	f64 aim_pp;
	f64 speed_pp;
	f64 acc_pp;
};

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
pp_calc_result pp_calc(f64 aim, f64 speed, beatmap& b, 
	u32 used_mods=mods::nomod, 
	u16 combo = 0xFFFF, u16 misses = 0, u16 c300 = 0xFFFF, 
	u16 c100 = 0, u16 c50 = 0, u32 score_version = 1)
{
	pp_calc_result res{0.0};

	f64 od = b.od;
	f64 ar = b.ar;
	u16 circles = b.num_circles;

	if (c300 == 0xFFFF) {
		c300 = (u16)b.num_objects - c100 - c50 - misses;
	}

	if (combo == 0xFFFF) {
		combo = b.max_combo;
	}

	// input validation
	if (!b.max_combo) {
		die("Max combo cannot be zero");
		return res;
	}

	u16 total_hits = c300 + c100 + c50 + misses;
	if (total_hits != b.num_objects) {
		dbgprintf("warning: total hits(%" fu16 ") don't "
			"match hit-object count (%zd)\n", total_hits, b.num_objects);
	}

	if (score_version != 1 && score_version != 2) {
		die("This score version does not exist or isn't supported");
		return res;
	}

	// accuracy (not in percentage, ranges between 0 and 1)
	f64 acc = acc_calc(c300, c100, c50, misses);
	res.acc_percent = acc * 100.0;

	// aim pp ------------------------------------------------------------------
	f64 aim_value = base_strain(aim);

	// length bonus (reused in speed pp)
	f64 total_hits_over_2k = (f64)total_hits / 2000.0;
	f64 length_bonus = 0.95 + 
		0.4 * std::min(1.0, total_hits_over_2k) +
		(total_hits > 2000 ? std::log10(total_hits_over_2k) * 0.5 : 0.0);

	// miss penality (reused in speed pp)
	f64 miss_penality = std::pow(0.97, misses);

	// combo break penality (reused in speed pp)
	f64 combo_break = 
		std::pow((f64)combo, 0.8) / std::pow((f64)b.max_combo, 0.8);

	aim_value *= length_bonus;
	aim_value *= miss_penality;
	aim_value *= combo_break;

	f64 ar_bonus = 1.0;

	// high ar bonus
	if (ar > 10.33) {
		ar_bonus += 0.45 * (ar - 10.33);
	}

	// low ar bonus
	else if (ar < 8.0) {
		f64 low_ar_bonus = 0.01 * (8.0 - ar);

		if (used_mods & mods::hd) {
			low_ar_bonus *= 2.0;
		}

		ar_bonus += low_ar_bonus;
	}

	aim_value *= ar_bonus;

	// hidden
	if (used_mods & mods::hd) {
		aim_value *= 1.18;
	}

	// flashlight
	if (used_mods & mods::fl) {
		aim_value *= 1.45 * length_bonus;
	}

	// acc bonus (bad aim can lead to bad acc, reused in speed for same reason)
	f64 acc_bonus = 0.5 + acc / 2.0;

	// od bonus (low od is easy to acc even with shit aim, reused in speed ...)
	f64 od_bonus = 0.98 + std::pow(od, 2) / 2500.0;

	aim_value *= acc_bonus;
	aim_value *= od_bonus;

	res.aim_pp = aim_value;

	// speed pp ----------------------------------------------------------------
	f64 speed_value = base_strain(speed);
	
	speed_value *= length_bonus;
	speed_value *= miss_penality;
	speed_value *= combo_break;
	speed_value *= acc_bonus;
	speed_value *= od_bonus;

	res.speed_pp = speed_value;

	// acc pp ------------------------------------------------------------------
	f64 real_acc = 0.0; // accuracy calculation changes from scorev1 to scorev2

	if (score_version == 2) {
		circles = total_hits;
		real_acc = acc;
	} else {
		// scorev1 ignores sliders since they are free 300s
		if (circles) {
			real_acc = (
					(c300 - (total_hits - circles)) * 300.0 +
					c100 * 100.0 +
					c50 * 50.0
				) / (circles * 300);
		}

		// can go negative if we miss everything
		real_acc = std::max(0.0, real_acc);
	}

	// arbitrary values tom crafted out of trial and error
	f64 acc_value = 
		std::pow(1.52163, od) * std::pow(real_acc, 24.0) * 2.83; 

	// length bonus (not the same as speed/aim length bonus)
	acc_value *= std::min(1.15, std::pow(circles / 1000.0, 0.3));

	// hidden bonus
	if (used_mods & mods::hd) {
		acc_value *= 1.02;
	}

	// flashlight bonus
	if (used_mods & mods::fl) {
		acc_value *= 1.02;
	}

	res.acc_pp = acc_value;

	// total pp ----------------------------------------------------------------
	f64 final_multiplier = 1.12;

	// nofail
	if (used_mods & mods::nf) {
		final_multiplier *= 0.90;
	}

	// spun-out
	if (used_mods & mods::so) {
		final_multiplier *= 0.95;
	}

	res.pp = std::pow(
			std::pow(aim_value, 1.1) +
			std::pow(speed_value, 1.1) +
			std::pow(acc_value, 1.1), 
			1.0 / 1.1
		) * final_multiplier;

	return res;
}

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
pp_calc_result pp_calc_acc(f64 aim, f64 speed, beatmap& b, f64 acc_percent, 
	u32 used_mods=mods::nomod, u16 combo = 0xFFFF, u16 misses = 0, 
	u32 score_version = 1)
{
	// cap misses to num objects
	misses = std::min((u16)b.num_objects, misses);

	// cap acc to max acc with the given amount of misses
	u16 max300 = (u16)(b.num_objects - misses);

	acc_percent = std::max(0.0, 
			std::min(acc_calc(max300, 0, 0, misses) * 100.0, acc_percent));

	// round acc to the closest amount of 100s or 50s
	u16 c50 = 0;
	u16 c100 = (u16)std::round(-3.0 * ((acc_percent * 0.01 - 1.0) * 
		b.num_objects + misses) * 0.5);

	if (c100 > b.num_objects - misses) {
		// acc lower than all 100s, use 50s
		c100 = 0;
		c50 = (u16)std::round(-6.0 * ((acc_percent * 0.01 - 1.0) * 
			b.num_objects + misses) * 0.2);

		c50 = std::min(max300, c50);
	}
	else {
		c100 = std::min(max300, c100);
	}

	u16 c300 = (u16)b.num_objects - c100 - c50 - misses;

	return pp_calc(aim, speed, b, used_mods, combo, misses, c300, c100, c50, 
		score_version);
}

