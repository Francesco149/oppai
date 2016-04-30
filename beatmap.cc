#include "beatmap.h"
#include "common.h"

#include <string.h>
#include <algorithm>

#include "utils.h"
#include "slider_calc.h"
#include "pp_calc.h" // TODO: move the mods namespace elsewhere?

namespace {
	// too fucking lazy to do proper buffering, I will just read the entire
	// file into memory
	const size_t bufsize = 2000000; // 2 mb
	u8 buf[bufsize];
}

timing_point* beatmap::timing(i64 time) {
	for (size_t i = num_timing_points - 1; i >= 0; i--) {
		auto& cur = timing_points[i];

		if (cur.time <= time) {
			return &cur;
		}
	}

	return &timing_points[0];
}

timing_point* beatmap::parent_timing(timing_point* t) {
	if (!t->inherit) {
		return t;
	}

	for (size_t i = num_timing_points - 1; i >= 0; i--) {
		auto& cur = timing_points[i];
		
		if (cur.time <= t->time && !cur.inherit) {
			return &cur;
		}
	}

	die("Orphan timing section");
}

// TODO: throw some consts in here
void beatmap::apply_mods(u32 mods) {
	if ((mods & mods::map_changing) == 0) {
		return;
	}

	// playback speed
	f64 speed = 1;
	
	if ((mods & mods::dt) != 0 || (mods & mods::nc) != 0) {
		speed *= 1.5;
	}

	if (mods & mods::ht) {
		speed *= 0.75;
	}

	// od
	f64 od_multiplier = 1;

	if (mods & mods::hr) {
		od_multiplier *= 1.4;
	}

	if (mods & mods::ez) {
		od_multiplier *= 0.5;
	}

	od *= od_multiplier;
	f64 odms = 79.5 - 6.0 * od;

	// ar
	f64 ar_multiplier = 1;

	if (mods & mods::hr) {
		ar_multiplier = 1.4;
	}

	if (mods & mods::ez) {
		ar_multiplier = 0.5;
	}

	ar *= ar_multiplier;
	f64 arms = ar <= 5 
		? (1800 - 120 * ar) 
		: (1200 - 150 * (ar - 5));

	// cs
	f64 cs_multiplier = 1;

	if (mods & mods::hr) {
		cs_multiplier = 1.3;
	}

	if (mods & mods::ez) {
		cs_multiplier = 0.5;
	}

	// stats must be capped to 0-10 before HT/DT which bring them to a range
	// of -4.42 to 11.08 for OD and -5 to 11 for AR
	odms = std::min(79.5, std::max(19.5, odms));
	arms = std::min(1800.0, std::max(450.0, arms));

	odms /= speed;
	arms /= speed;
	od = (79.5 - odms) / 6.0;
	ar = ar <= 5.0
		? ((1800.0 - arms) / 120.0)
		: (5.0 + (1200.0 - arms) / 150.0);

	cs *= cs_multiplier;
	cs = std::max(0.0, std::min(10.0, cs));

	if ((mods & mods::speed_changing) == 0) {
		// not speed-modifying
		return;
	}

	for (size_t i = 0; i < num_timing_points; i++) {
		auto &tp = timing_points[i];
		tp.time = (i64)((f64)tp.time / speed);
		if (!tp.inherit) {
			tp.ms_per_beat /= speed;
		}
	}

	for (size_t i = 0; i < num_objects; i++) {
		auto& o = objects[i];
		o.time = (i64)((f64)o.time / speed);
		o.end_time = (i64)((f64)o.end_time / speed);
	}
}

i64 hit_object::num_segments() {
	if (type != obj::slider) {
		//puts("Warning: tried to call .num_segments on a non-slider object");
		return 1;
	}

	return slider_segment_count(*this);
}

v2f hit_object::at(i64 ms) {
	if (type != obj::slider) {
		//puts("Warning: tried to call .at on a non-slider object");
		return pos;
	}

	return slider_at(*this, ms);
}

void beatmap::parse(const char* osu_file, beatmap& b) {
	auto f = fopen(osu_file, "rb");
	if (!f) {
		die("Failed to open beatmap");
	}

	auto cb = fread(buf, 1, bufsize, f);
	if (cb == bufsize) {
		die("Beatmap is too big for the internal buffer because I am a lazy "
			"fuck who can't parse files the proper way");
	}

	fclose(f);

	buf[cb] = 0;

	char* tok = strtok((char*)buf, "\n");

	// ---

	while (tok && *tok != '[') {
		if (sscanf(tok, "osu file format v%d", 
				   &b.format_version) == 1) {
			break;
		}
		tok = strtok(nullptr, "\n");
	}

	if (!b.format_version) {
		//die("File format version not found");
	}

	// ---
	
	while (tok) {
		if (strstr(tok, "[General]")) {
			goto found_general;
		}
		tok = strtok(nullptr, "\n");
	}

	die("Could not find General info");

found_general:

	// ---
	
	// NOTE: I could just store all properties and map them by section and name
	// but I'd rather parse only the ones I need since I'd still need to parse
	// them one by one and check for format errors.

	// StackLeniency and Mode are not present in older formats
	tok = strtok(nullptr, "\n");
	for (; tok && *tok != '['; tok = strtok(nullptr, "\n")) {

		if (sscanf(tok, "StackLeniency: %lf", &b.stack_leniency) == 1) {
			continue;
		}

		else if (sscanf(tok, "Mode: %hhd", &b.mode) == 1) {
			continue;
		}
	}

	// ---

	while (tok) {
		if (strstr(tok, "[Metadata]")) {
			goto found_metadata;
		}
		tok = strtok(nullptr, "\n");
	}

	die("Could not find metadata");

found_metadata:

	// ---
	
	tok = strtok(nullptr, "\n");
	for (; tok && *tok != '['; tok = strtok(nullptr, "\n")) {

		if (sscanf(tok, "Title: %[^\r\n]", b.title) == 1) {
			continue;
		}

		else if (sscanf(tok, "Artist: %[^\r\n]", b.artist) == 1) {
			continue;
		}

		else if (sscanf(tok, "Creator: %[^\r\n]", b.creator) == 1) {
			continue;
		}

		else if (sscanf(tok, "Version: %[^\r\n]", b.version) == 1) {
			continue;
		}
	}

	if (!strlen(b.title)) {
		die("Missing title in metadata");
	}

	if (!strlen(b.artist)) {
		die("Missing artist in metadata");
	}

	if (!strlen(b.creator)) {
		die("Missing creator in metadata");
	}

	if (!strlen(b.version)) {
		die("Missing version in metadata");
	}

	// ---
	
	while (tok) {
		if (strstr(tok, "[Difficulty]")) {
			goto found_difficulty;
		}
		tok = strtok(nullptr, "\n");
	}

	die("Could not find difficulty");

found_difficulty:

	// ---
	
	tok = strtok(nullptr, "\n");
	for (; tok && *tok != '['; tok = strtok(nullptr, "\n")) {

		if (sscanf(tok, "HPDrainRate: %lf", &b.hp) == 1) {
			continue;
		}

		if (sscanf(tok, "CircleSize: %lf", &b.cs) == 1) {
			continue;
		}

		if (sscanf(tok, "OverallDifficulty: %lf", &b.od) == 1) {
			continue;
		}

		if (sscanf(tok, "ApproachRate: %lf", &b.ar) == 1) {
			continue;
		}

		if (sscanf(tok, "SliderMultiplier: %lf", &b.sv) == 1) {
			continue;
		}

		if (sscanf(tok, "SliderTickRate: %lf", &b.tick_rate) == 1) {
			continue;
		}
	}

	if (b.hp > 10) {
		die("Invalid or missing HP");
	}

	if (b.cs > 10) {
		die("Invalid or missing CS");
	}

	if (b.od > 10) {
		die("Invalid or missing OD");
	}

	if (b.ar > 10) {
		puts("warning: AR not found, assuming old map and setting AR=OD");
		b.ar = b.od;
	}

	if (b.sv > 10) { // not sure what max sv is
		die("Invalid or missing SV");
	}

	// ---
	
	// skip until the TimingPoints section
	while (tok) {
		if (strstr(tok, "[TimingPoints]")) {
			goto found_timing;
		}
		tok = strtok(nullptr, "\n");
	}

	die("Could not find timing points");

found_timing:

	// ---

	i32 useless;

	tok = strtok(nullptr, "\n");
	for (; *tok != '['; tok = strtok(nullptr, "\n")) {

		// ghetto way to ignore empty lines, will mess up on whitespace lines
		if (!strlen(tok) || !strcmp(tok, "\r")) {
			continue;
		}

		if (b.num_timing_points >= beatmap::max_timing_points) {
			die("Too many timing points for the internal buffer");
		}

		auto& tp = b.timing_points[b.num_timing_points];

		u8 not_inherited = 0;
		f64 time_tmp; // I'm rounding times to milliseconds. 
					  // not sure if making them floats will matter for diff calc.
		if (sscanf(tok, "%lf,%lf,%d,%d,%d,%d,%hhd", 
				   &time_tmp, &tp.ms_per_beat, 
				   &useless, &useless, &useless, &useless, 
				   &not_inherited) == 7) {

			tp.time = (i64)time_tmp;
			tp.inherit = not_inherited == 0;
			goto parsed_timing_pt;
		}

		// older formats might not have inherit and the other info
		if (sscanf(tok, "%lf,%lf", &time_tmp, &tp.ms_per_beat) != 2) {
			tp.time = (i64)time_tmp;
			die("Invalid format for timing point");
		}

parsed_timing_pt:
		b.num_timing_points++;
	}

	// ---
	
	// skip until the HitObjects section
	while (tok) {
		if (strstr(tok, "[HitObjects]")) {
			goto found_objects;
		}
		tok = strtok(nullptr, "\n");
	}

	die("Could not find hit objects");

found_objects:

	// ---

	tok = strtok(nullptr, "\n");
	for (; tok; tok = strtok(nullptr, "\n")) {

		// ghetto way to ignore empty lines, will mess up on whitespace lines
		if (!strlen(tok) || !strcmp(tok, "\r")) {
			continue;
		}

		if (b.num_objects >= beatmap::max_objects) {
			die("Too many hit objects for the internal buffer");
		}

		auto& ho = b.objects[b.num_objects];

		i32 type_num;

		// slider
		if (sscanf(tok, "%lf,%lf,%" fi64 ",%d,%d,%c", 
				   &ho.pos.x, &ho.pos.y, &ho.time, &useless, &useless, 
				   &ho.slider.type) == 6 && 
				ho.slider.type >= 'A' && ho.slider.type <= 'Z') {
			
			// the slider type check is for old maps that have trailing 
			// commas on circles and sliders

			// x,y,time,type,hitSound,sliderType|curveX:curveY|...,repeat,
			// 		pixelLength,edgeHitsound,edgeAddition,addition
			ho.type = obj::slider;
		}

		// circle, or spinner
		else if (sscanf(tok, "%lf,%lf,%" fi64 ",%d,%d,%" fi64,
				   &ho.pos.x, &ho.pos.y, &ho.time, &type_num, &useless, 
				   &ho.end_time) == 6) {

			if (type_num & 8) {
				// x,y,time,type,hitSound,endTime,addition
				ho.type = obj::spinner;
			} else {
				// x,y,time,type,hitSound,addition
				ho.type = obj::circle;
				ho.end_time = ho.time;
			}
		}

		// old circle
		else if (sscanf(tok, "%lf,%lf,%" fi64 ",%d,%d", 
			&ho.pos.x, &ho.pos.y, &ho.time, &type_num, &useless) == 5) {

			ho.type = obj::circle;
			ho.end_time = ho.time;
		}

		else {
			die("Invalid hit object found");
		}

		b.num_objects++;

		// increase max combo and circle/slider count
		b.max_combo++; // slider ticks are calculated later
		switch (ho.type) {
			case obj::circle:
				b.circle_count++;
				break;

			case obj::slider:
				b.slider_count++;
				break;

			case obj::spinner:
				break;

			case obj::invalid:
				die("How did you get here????????");
		}

		// slider points are separated by |
		if (!strstr(tok, "|")) {	
			
			// expected slider but no points found
			if (ho.type == obj::slider) {
				die("Slider is missing points");
			}

			continue;
		}

		// not a slider yet slider points were found
		if (ho.type != obj::slider) {
			die("Invalid slider found");
		}

		auto& sl = ho.slider;

		// gotta make a copy of the line to tokenize sliders without affecting
		// the current line-by-line tokenization
		char line[2048];
		strcpy(line, tok);

		char* saveptr = nullptr;
		char* slider_tok = strtok_r(line, "|", &saveptr);
		slider_tok = strtok_r(nullptr, "|", &saveptr); // skip first token

		sl.points[sl.num_points++] = ho.pos;

		for (; slider_tok; slider_tok = strtok_r(nullptr, "|", &saveptr)) {

			if (sl.num_points >= slider_data::max_points) {
				die("Too many slider points for the internal buffer");
			}

			auto& pt = sl.points[sl.num_points];

			// lastcurveX:lastcurveY,repeat,pixelLength,
			// 		edgeHitsound,edgeAddition,addition
			if (sscanf(slider_tok, 
				      "%lf:%lf,%hd,%lf", 
					  &pt.x, &pt.y, &sl.repetitions, &sl.length) == 4) {

				sl.num_points++;
				// end of point list
				break;
			}
			
			// curveX:curveY
			else if (sscanf(slider_tok, "%lf:%lf", &pt.x, &pt.y) != 2) {
				die("Invalid slider found");
			}

			sl.num_points++;
		}

		// find which timing section the slider belongs to
		auto tp = b.timing(ho.time);
		auto parent = b.parent_timing(tp);

		// calculate slider velocity multiplier for inherited sections
		f64 sv_multiplier = 1;
		if (tp->inherit && tp->ms_per_beat < 0) {
			sv_multiplier = (-100.0 / tp->ms_per_beat);
		}

		// calculate slider end time
		f64 px_per_beat = b.sv * 100.0 * sv_multiplier;
		f64 num_beats = (sl.length * sl.repetitions) / px_per_beat;
		i64 duration = (i64)std::ceil(num_beats * parent->ms_per_beat);
		ho.end_time = ho.time + duration;

		// sliders get 2 + ticks combo (head, tail and ticks)
		// each repetition adds an extra combo and an extra set of ticks
		// the -.01 is there to prevent ceil from ceiling whole values
		// like 1.0 to 2.0 randomly
		u16 ticks = (u16)std::ceil(
				(num_beats - 0.01) / sl.repetitions * b.tick_rate);
		ticks--;
		ticks *= sl.repetitions;
		ticks += sl.repetitions + 1;
		//printf("%g x %ld %hd\n", num_beats, sl.repetitions, ticks);

		b.max_combo += ticks - 1; // -1 because we already did ++ earlier
	}
}
