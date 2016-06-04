#include "beatmap.h"
#include "common.h"

#include <string.h>
#include <cmath>
#include <string>
#if NEEDS_TO_INSTALL_GENTOO
#include <algorithm>
#endif

#include "utils.h"
#include "slider_calc.h"
#include "pp_calc.h" // TODO: move the mods namespace elsewhere?

namespace {
	// too fucking lazy to do proper buffering, I will just read the entire
	// file into memory
	const size_t bufsize = 2000000; // 2 mb
	u8 buf[bufsize];

	const f64	od0_ms = 79.5,
				od10_ms = 19.5,
				ar0_ms = 1800,
				ar5_ms = 1200,
				ar10_ms = 450;

	const f64	od_ms_step = 6,
				ar_ms_step1 = 120, // ar0-5
				ar_ms_step2 = 150; // ar5-10
}

timing_point* beatmap::timing(i64 time) {
	for (i64 i = (i64)num_timing_points - 1; i >= 0; i--) {
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

	for (i64 i = (i64)num_timing_points - 1; i >= 0; i--) {
		auto& cur = timing_points[i];
		
		if (cur.time <= t->time && !cur.inherit) {
			return &cur;
		}
	}

	die("Orphan timing section");
	return nullptr;
}

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
	f64 odms = od0_ms - std::ceil(od_ms_step * od);

	// ar
	f64 ar_multiplier = 1;

	if (mods & mods::hr) {
		ar_multiplier = 1.4;
	}

	if (mods & mods::ez) {
		ar_multiplier = 0.5;
	}

	ar *= ar_multiplier;

	// convert AR into its milliseconds value
	f64 arms = ar <= 5 
		? (ar0_ms - ar_ms_step1 *  ar     ) 
		: (ar5_ms - ar_ms_step2 * (ar - 5));

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
	odms = std::min(od0_ms, std::max(od10_ms, odms));
	arms = std::min(ar0_ms, std::max(ar10_ms, arms));

	// apply speed-changing mods
	odms /= speed;
	arms /= speed;

	// convert OD and AR back into their stat form
	//od = (-(odms - od0_ms)) / od_ms_step;
	od = (od0_ms - odms) / od_ms_step;
	ar = ar <= 5.0
		//? (      (-(arms - ar0_ms)) / ar_ms_step1)
		//: (5.0 + (-(arms - ar5_ms)) / ar_ms_step2);
		? (      (ar0_ms - arms) / ar_ms_step1)
		: (5.0 + (ar5_ms - arms) / ar_ms_step2);

	cs *= cs_multiplier;
	cs = std::max(0.0, std::min(10.0, cs));

	if ((mods & mods::speed_changing) == 0) {
		// not speed-modifying
		return;
	}

	// apply speed-changing mods

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

v2f hit_object::at(i64 ms) {
	if (type != obj::slider) {
		//vbputs("Warning: tried to call .at on a non-slider object");
		return pos;
	}

	return slider_at(*this, ms);
}

void beatmap::parse(const char* osu_file, beatmap& b) {
	auto f = fopen(osu_file, "rb");
	if (!f) {
		die("Failed to open beatmap");
		return;
	}

	auto cb = fread(buf, 1, bufsize, f);
	if (cb == bufsize) {
		die("Beatmap is too big for the internal buffer because I am a lazy "
			"fuck who can't parse files the proper way");
		return;
	}

	dbgprintf("%zd bytes\n", cb);
	fclose(f);

	// just to be safe
	buf[cb] = 0;

	// ---

	char* tok = strtok((char*)buf, "\n");

	auto fwd = [&tok]() { 
		// skips 1 line
		tok = strtok(nullptr, "\n"); 
	};

	auto find_fwd = [&tok, fwd](const char* str) -> bool {
		// skips forward until tok is the line right after 
		// the one that contains str
		dbgprintf("skipping until %s", str);
		while (tok) {
			if (strstr(tok, str)) {
				fwd();
				return true;
			}
			fwd();
		}
		return false;
	};

	auto not_section = [&tok]() -> bool {
		return tok && *tok != '[';
	};
	
	// ---

	while (not_section()) {
		if (sscanf(tok, "osu file format v%" fi32 "", 
				   &b.format_version) == 1) {
			break;
		}
		fwd();
	}

	if (!b.format_version) {
		vbputs("Warning: File format version not found");
	}

	// ---
	
	if (!find_fwd("[General]")) {
		die("Could not find General info");
		return;
	}

	// ---
	
	// NOTE: I could just store all properties and map them by section and name
	// but I'd rather parse only the ones I need since I'd still need to parse
	// them one by one and check for format errors.

	// StackLeniency and Mode are not present in older formats
	for (; not_section(); fwd()) {

		if (sscanf(tok, "StackLeniency: %lf", &b.stack_leniency) == 1) {
			continue;
		}

		if (sscanf(tok, "Mode: %" fu8 "", &b.mode) == 1) {
			continue;
		}
	}

	// ---

	if (!find_fwd("[Metadata]")) {
		die("Could not find Metadata");
		return;
	}

	// ---
	
	for (; not_section(); fwd()) {

		// %[^\r\n] means accept all characters except \r\n
		// which means that it'll grab the string until it finds \r or \n
		if (sscanf(tok, "Title: %[^\r\n]", b.title) == 1) {
			continue;
		}

		if (sscanf(tok, "Artist: %[^\r\n]", b.artist) == 1) {
			continue;
		}

		if (sscanf(tok, "Creator: %[^\r\n]", b.creator) == 1) {
			continue;
		}

		if (sscanf(tok, "Version: %[^\r\n]", b.version) == 1) {
			continue;
		}
	}

	if (!strlen(b.title)) {
		die("Missing title in metadata");
		return;
	}

	if (!strlen(b.artist)) {
		die("Missing artist in metadata");
		return;
	}

	if (!strlen(b.creator)) {
		die("Missing creator in metadata");
		return;
	}

	if (!strlen(b.version)) {
		die("Missing version in metadata");
		return;
	}

	// ---
	
	if (!find_fwd("[Difficulty]")) {
		die("Could not find Difficulty");
		return;
	}

	// ---
	
	for (; not_section(); fwd()) {

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
		return;
	}

	if (b.cs > 10) {
		die("Invalid or missing CS");
		return;
	}

	if (b.od > 10) {
		die("Invalid or missing OD");
		return;
	}

	if (b.ar > 10) {
		vbputs("Warning: AR not found, assuming old map and setting AR=OD");
		b.ar = b.od;
	}

	if (b.sv > 10) { // not sure what max sv is
		die("Invalid or missing SV");
		return;
	}

	// ---
	
	if (!find_fwd("[TimingPoints]")) {
		die("Could not find TimingPoints");
		return;
	}

	// ---

	i32 useless;

	// TODO: move this func elsewhere
	auto whitespace = [](const char* s) -> bool {
		while (*s) {
			if (!isspace(*s)) {
				return false;
			}
			s++;
		}
		return true;
	};

	for (; not_section(); fwd()) {

		if (whitespace(tok)) {
			dbgputs("skipping whitespace line");
			continue;
		}

		if (b.num_timing_points >= beatmap::max_timing_points) {
			die("Too many timing points for the internal buffer");
			return;
		}

		auto& tp = b.timing_points[b.num_timing_points];

		u8 not_inherited = 0;
		f64 time_tmp; // I'm rounding times to milliseconds. 
					  // not sure if making them floats will matter for diffcalc

		if (sscanf(tok, "%lf,%lf,%" fi32 ",%" fi32 ",%" fi32 ",%" fi32 ",%" fu8, 
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
			return;
		}

parsed_timing_pt:
		b.num_timing_points++;
	}

	// ---
	
	if (!find_fwd("[HitObjects]")) {
		die("Could not find HitObjects");
		return;
	}

	// ---

	for (; tok; fwd()) {

		if (whitespace(tok)) {
			dbgputs("skipping whitespace line");
			continue;
		}

		if (b.num_objects >= beatmap::max_objects) {
			die("Too many hit objects for the internal buffer");
			return;
		}

		auto& ho = b.objects[b.num_objects];

		i32 type_num;

		// slider
		if (sscanf(tok, "%f,%f,%" fi64 ",%" fi32 ",%" fi32 ",%c", 
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
		else if (sscanf(tok, "%f,%f,%" fi64 ",%" fi32 ",%" fi32 ",%" fi64,
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
		else if (sscanf(tok, "%f,%f,%" fi64 ",%" fi32 ",%" fi32 "", 
			&ho.pos.x, &ho.pos.y, &ho.time, &type_num, &useless) == 5) {

			ho.type = obj::circle;
			ho.end_time = ho.time;
		}

		else {
			die("Invalid hit object found");
			return;
		}

		b.num_objects++;
		dbgprintf("\n\nobject %zd\n", b.num_objects);

		// increase max combo and circle/slider count
		b.max_combo++; // slider ticks are calculated later
		switch (ho.type) {
			case obj::circle:
				dbgputs("it's a circle!");
				b.num_circles++;
				break;

			case obj::slider:
				dbgputs("it's a slider!");
				b.num_sliders++;
				break;

			case obj::spinner:
				dbgputs("it's a spinner!");
				b.num_spinners++;
				break;

			case obj::invalid:
				die("How did you get here????????");
				return;
		}

		// slider points are separated by |
		if (!strstr(tok, "|")) {
			
			// expected slider but no points found
			if (ho.type == obj::slider) {
				die("Slider is missing points");
				return;
			}

			dbgputs("no slider points, we're done here");

			continue;
		}

		// not a slider yet slider points were found
		if (ho.type != obj::slider) {
			die("Invalid slider found");
			return;
		}

		auto& sl = ho.slider;

		// gotta make a copy of the line to tokenize sliders without affecting
		// the current line-by-line tokenization
		std::string sline{tok};
		char* line = &sline[0];

		char* saveptr = nullptr;
		char* slider_tok = strtok_r(line, "|", &saveptr);
		slider_tok = strtok_r(nullptr, "|", &saveptr); // skip first token

		// TODO: prevent useless copying here and in other vector usages
		sl.points.push_back(ho.pos);
		dbgputs("first slider point");

		for (; slider_tok; slider_tok = strtok_r(nullptr, "|", &saveptr)) {
			sl.points.push_back(v2f{});
			auto& pt = sl.points[sl.points.size() - 1];

			// lastcurveX:lastcurveY,repeat,pixelLength,
			// 		edgeHitsound,edgeAddition,addition
			if (sscanf(slider_tok, 
				      "%f:%f,%" fu16 ",%lf", 
					  &pt.x, &pt.y, &sl.repetitions, &sl.length) == 4) {

				dbgputs("last slider point");
				// end of point list
				break;
			}
			
			// curveX:curveY
			else if (sscanf(slider_tok, "%f:%f", &pt.x, &pt.y) != 2) {
				die("Invalid slider found");
				return;
			}

			dbgprintf("slider point %zd\n", sl.points.size());
		}

		// find which timing section the slider belongs to
		auto tp = b.timing(ho.time);
		auto parent = b.parent_timing(tp);
		if (err()) {
			return;
		}

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

		// calculate the number of slider ticks for one repetition
		// ---
		// example: a 3.75 beats slider at 1x tick rate will go:
		// beat0 (head), beat1 (tick), beat2(tick), beat3(tick), beat3.75(tail)
		// so all we have to do is ceil the number of beats and subtract 1 to
		// take out the tail
		// ---
		// the -.01 is there to prevent ceil from ceiling whole values
		// like 1.0 to 2.0 randomly
		u16 ticks = (u16)std::ceil(
				(num_beats - 0.01) / sl.repetitions * b.tick_rate);
		ticks--;

		ticks *= sl.repetitions; // multiply slider ticks by repetitions
		ticks += sl.repetitions + 1; // add heads and tails

		dbgprintf("%g beats x %" fu16 " = %" fu16 " combo\n", 
			num_beats, sl.repetitions, ticks);

		b.max_combo += ticks - 1; // -1 because we already did ++ earlier
	}

	dbgputs("\nparsing done");
}
