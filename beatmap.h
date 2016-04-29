#pragma once

#include <stddef.h>
#include <vector>
#include "v2f.h"

// shit code ahead! I am way too lazy to write nice code for parsers, sorry
// disclaimer: this beatmap parser is meant purely for difficulty calculation 
//             and I don't support any malicious use of it.

struct slider_data {
	char type;

	static const size_t max_points = 0xFF;
	size_t num_points = 0;
	v2f points[max_points];
	std::vector<v2f> pos_at_ms; // position at every millisecond for
								// bezier and catmull sliders
	
	// NOTE: pos_at_ms is initialized by calling .at on hit_object at least once

	u64 repetitions = 0;
	f32 length = 0;
};

// hit-object type
enum class obj : u8 {
	invalid = 0,
	circle, 
	spinner, 
	slider, 
};

struct hit_object {
	v2f pos{0};
	i64 time = 0;
	obj type = obj::invalid;
	i64 end_time = 0; // for spinners
	slider_data slider;

	// get position at given millisecond offset (for sliders)
	// NOTE: the first call to this will be slow, as it will pre-calculate
	//       the entire curve for subsequent calls.
	v2f at(i64 ms);
	i64 num_segments();
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
	f32 stack_leniency = 0;
	u8 mode = 0;

	// metadata
	char title[256] = {0};
	char artist[256] = {0};
	char creator[256] = {0};
	char version[256] = {0};

	// difficulty
	f32 hp = 1337.f;
	f32 cs = 1337.f;
	f32 od = 1337.f;
	f32 ar = 1337.f;
	f32 sv = 1337.f;

	u16 circle_count = 0;
	u16 slider_count = 0;
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
