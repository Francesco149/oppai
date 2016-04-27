#include "beatmap.h"

#include <string.h>
#include <algorithm>

#include "utils.h"
#include "slider_calc.h"

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

v2f hit_object::at(i64 ms) {
	if (type != obj::slider) {
		puts("Warning: tried to call .at on a non-slider object");
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
	printf("%ld bytes\n\n", cb);

	puts("Parsing...\n");

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
		die("File format version not found");
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

		if (sscanf(tok, "StackLeniency: %f", &b.stack_leniency) == 1) {
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

		if (sscanf(tok, "HPDrainRate: %f", &b.hp) == 1) {
			continue;
		}

		else if (sscanf(tok, "CircleSize: %f", &b.cs) == 1) {
			continue;
		}

		else if (sscanf(tok, "OverallDifficulty: %f", &b.od) == 1) {
			continue;
		}

		else if (sscanf(tok, "ApproachRate: %f", &b.ar) == 1) {
			continue;
		}

		else if (sscanf(tok, "SliderMultiplier: %f", &b.sv) == 1) {
			continue;
		}
	}

	if (b.hp > 10.f) {
		die("Invalid or missing HP");
	}

	if (b.cs > 10.f) {
		die("Invalid or missing CS");
	}

	if (b.od > 10.f) {
		die("Invalid or missing OD");
	}

	if (b.ar > 10.f) {
		puts("warning: AR not found, defaulting to AR6");
		b.ar = 6.f;
	}

	if (b.sv > 10.f) { // not sure what max sv is
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
		if (sscanf(tok, "%ld,%lf,%d,%d,%d,%d,%hhd", 
				   &tp.time, &tp.ms_per_beat, 
				   &useless, &useless, &useless, &useless, 
				   &not_inherited) == 7) {

			tp.inherit = not_inherited == 0;
			goto parsed_timing_pt;
		}

		// older formats might not have inherit and the other info
		if (sscanf(tok, "%ld,%lf", &tp.time, &tp.ms_per_beat) != 2) {
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
		if (sscanf(tok, "%f,%f,%ld,%d,%d,%c", 
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
		else if (sscanf(tok, "%f,%f,%ld,%d,%d,%ld", 
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
		else if (sscanf(tok, "%f,%f,%ld,%d,%d", 
			&ho.pos.x, &ho.pos.y, &ho.time, &type_num, &useless) == 5) {

			ho.type = obj::circle;
			ho.end_time = ho.time;
		}

		else {
			die("Invalid hit object found");
		}

		b.num_objects++;

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
				      "%f:%f,%ld,%f", 
					  &pt.x, &pt.y, &sl.repetitions, &sl.length) == 4) {

				sl.num_points++;
				// end of point list
				break;
			}
			
			// curveX:curveY
			else if (sscanf(slider_tok, "%f:%f", &pt.x, &pt.y) != 2) {
				die("Invalid slider found");
			}

			sl.num_points++;
		}

		// find which timing section the slider belongs to
		auto tp = b.timing(ho.time);
		auto parent = b.parent_timing(tp);

		// calculate slider velocity multiplier for inherited sections
		f32 sv_multiplier = 1.f;
		if (tp->inherit) {
			sv_multiplier = (-100.f / tp->ms_per_beat);
		}

		// calculate slider end time
		f32 px_per_beat = b.sv * 100.f * sv_multiplier;
		f32 num_beats = (sl.length * sl.repetitions) / px_per_beat;
		f32 duration = std::ceil(num_beats * parent->ms_per_beat);
		ho.end_time = ho.time + duration;
	}
}
