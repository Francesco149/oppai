#pragma once

#include <stddef.h>
#include <vector>
#include "v2f.h"

// shit code ahead! I am way too lazy to write nice code for parsers, sorry
// disclaimer: this beatmap parser is meant purely for difficulty calculation 
//             and I don't support any malicious use of it

// hit-object type
enum class obj : u8 {
	invalid = 0,
	circle,
	spinner,
	slider,
};

struct slider_data {
	char type;

	std::vector<v2f> points;
	std::vector<v2f> pos_at_ms; // position at every millisecond for
								// bezier and catmull sliders
	
	// NOTE: pos_at_ms is initialized by calling .at on hit_object at least once

	u16 repetitions = 0;
	f64 length = 0;
};

struct hit_object {
	v2f pos{0};
	i64 time = 0;
	obj type = obj::invalid;
	i64 end_time = 0; // for spinners and sliders
	slider_data slider;

	// get position at given millisecond offset (for sliders).
	// the offset is relative to the object's time, which means that it ranges 
	// from 0 to end_time - time.
	// NOTE: the first call to this will be slow, as it will pre-calculate
	//       the entire curve for subsequent calls.
	v2f at(i64 ms);
};

struct timing_point {
	i64 time = 0;
	f64 ms_per_beat = 0;
	bool inherit = false;
};

// note: values not required for diff calc will be omitted from this parser
// at least for now
struct beatmap {
	u32 format_version = 0;

	// general
	f64 stack_leniency = 0;
	u8 mode = 0;

	// metadata
	char title[256] = {0};
	char artist[256] = {0};
	char creator[256] = {0};
	char version[256] = {0};

	// difficulty
	f64 hp = 1337;
	f64 cs = 1337;
	f64 od = 1337;
	f64 ar = 1337;
	f64 sv = 1337;
	f64 tick_rate = 1;

	u16 num_circles = 0;
	u16 num_sliders = 0;
	u16 num_spinners = 0;

	u16 max_combo = 0;

	static const size_t max_timing_points = 0xFFFF;
	size_t num_timing_points = 0;
	timing_point timing_points[max_timing_points];

	static const size_t max_objects = max_timing_points;
	size_t num_objects = 0;
	hit_object objects[max_objects];

	// parse .osu file into a beatmap object
	static void parse(const char* osu_file, beatmap& b);

	// get timing point at the given time
	timing_point* timing(i64 time);

	// find parent of a inherited timing point
	timing_point* parent_timing(timing_point* tp);

	// apply map-modifying mods (such as EZ, HR, DT, HT)
	void apply_mods(u32 mods);
};
