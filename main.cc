#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <algorithm>

// shit code ahead! I am way too lazy to write nice code for parsers, sorry
// disclaimer: this beatmap parser is meant purely for difficulty calculation 
//             and I don't support any malicious use of it.

namespace {
	typedef float f32;
	typedef double f64;
	typedef uint_least8_t u8;
	typedef uint_least32_t u32;
	typedef uint_least64_t u64;
	typedef int_least32_t i32;
	typedef int_least64_t i64;

	struct v2f {
		f32 x, y;
	};

	struct slider_data {
		char type;

		static const size_t max_points = 0xFF;
		size_t num_points;
		v2f points[max_points];

		u64 repetitions;
		f32 length;
	};

	enum class obj : u8 {
		invalid = 0,
		circle, 
		spinner, 
		slider, 
	};

	struct hit_object {
		v2f pos;
		i64 time;
		obj type;
		i64 end_time; // for spinners
		slider_data slider;
	};

	struct timing_point {
		i64 time;
		f64 ms_per_beat;
		bool inherit;
	};

	// note: values not required for diff calc will be omitted from this parser
	// at least for now
	struct beatmap {
		u32 format_version;

		// general
		f32 stack_leniency;
		u8 mode;

		// metadata
		char title[256];
		char artist[256];
		char creator[256];
		char version[256];

		// difficulty
		f32 hp;
		f32 cs;
		f32 od;
		f32 ar;
		f32 sv;

		static const size_t max_timing_points = 0xFFFF;
		size_t num_timing_points;
		timing_point timing_points[max_timing_points];

		static const size_t max_objects = 0xFFFF;
		size_t num_objects;
		hit_object objects[max_objects];
	};
}


namespace {
	void die(const char* msg) {
		fputs(msg, stderr);
		fputs("\n", stderr);
		exit(1);
	}

	// too fucking lazy to do proper buffering, I will just read the entire
	// file into memory
	const size_t bufsize = 2000000; // 2 mb
	u8 buf[bufsize];

	beatmap b{0};

	// returns the timing point at the given time
	timing_point* get_timing_point(i64 time) {
		for (size_t i = b.num_timing_points - 1; i >= 0; i--) {
			auto& cur = b.timing_points[i];

			if (cur.time <= time) {
				return &cur;
			}
		}

		return &b.timing_points[0];
	}

	// finds the parent timing point of a given timing point
	timing_point* get_parent(timing_point* t) {
		if (!t->inherit) {
			return t;
		}

		for (size_t i = b.num_timing_points - 1; i >= 0; i--) {
			auto& cur = b.timing_points[i];
			
			if (cur.time <= t->time && !cur.inherit) {
				return &cur;
			}
		}

		die("Orphan timing section");
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: %s /path/to/difficulty.osu\n", *argv);
		return 0;
	}

	puts("\n---");
	puts(argv[1]);
	puts("---\n");

	auto f = fopen(argv[1], "rb");
	if (!f) {
		die("Failed to open beatmap");
	}

	auto cb = fread(buf, 1, bufsize, f);
	if (cb == bufsize) {
		die("Beatmap is too big for the internal buffer because I am a lazy "
			"fuck who can't parse files the proper way");
	}

	buf[cb] = 0;
	printf("%ld bytes\n\n", cb);

	puts("Parsing...\n");

	char* tok = strtok((char*)buf, "\n");

	// ---

	while (!strstr(tok, "[General]")) {
		if (sscanf(tok, "osu file format v%d", 
				   &b.format_version) == 1) {
			break;
		}
		tok = strtok(nullptr, "\n");
	}

	if (!b.format_version) {
		die("File format version not found");
	}

	// it would be best to skip to [General] here but it's not necessary

	// ---
	
	// NOTE: I could just store all properties and map them by section and name
	// but I'd rather parse only the ones I need since I'd still need to parse
	// them one by one and check for format errors.

	// StackLeniency and Mode are not present in older formats
	for (; !strstr(tok, "[Metadata]"); tok = strtok(nullptr, "\n")) {

		if (sscanf(tok, "StackLeniency: %f", &b.stack_leniency) == 1) {
			continue;
		}

		else if (sscanf(tok, "Mode: %hhd", &b.mode) == 1) {
			continue;
		}
	}

	// it would be best to skip to [Metadata] here but it's not necessary

	// ---
	
	for (; !strstr(tok, "[Difficulty]"); tok = strtok(nullptr, "\n")) {

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

	b.hp = 1337.f;
	b.cs = 1337.f;
	b.od = 1337.f;
	b.ar = 1337.f;
	b.sv = 1337.f;

	for (; !strstr(tok, "[TimingPoints]"); tok = strtok(nullptr, "\n")) {

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
		die("Invalid or missing AR");
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
		tok = strtok(nullptr, "\n");

	// ---

	i32 useless;
	for (; !strstr(tok, "[Colours]"); tok = strtok(nullptr, "\n")) {

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
		tok = strtok(nullptr, "\n");

	// ---

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
		
		// circle, or spinner
		if (sscanf(tok, "%f,%f,%ld,%d,%d,%ld", 
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

		// slider
		else if (sscanf(tok, "%f,%f,%ld,%d,%d,%c", 
				   &ho.pos.x, &ho.pos.y, &ho.time, &useless, &useless, 
				   &ho.slider.type) == 6) {

			// x,y,time,type,hitSound,sliderType|curveX:curveY|...,repeat,
			// 		pixelLength,edgeHitsound,edgeAddition,addition
			ho.type = obj::slider;
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
		auto tp = get_timing_point(ho.time);
		auto parent = get_parent(tp);

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

	// ---

	fclose(f);

	printf(
		"Format version: %d\n"
		"Stack Leniency: %g\n"
		"Mode: %d\n"
		"Title: %s\n"
		"Artist: %s\n"
		"Version: %s\n"
		"HP%g CS%g OD%g AR%g SV%g\n\n"
		,
		b.format_version, 
		b.stack_leniency, 
		b.mode, 
		b.title, 
		b.artist, 
		b.version, 
		b.hp, b.cs, b.od, b.ar, b.sv
	);

	printf("> %ld timing points\n", b.num_timing_points);
	for (size_t i = 0; i < b.num_timing_points; i++) {
		auto& tp = b.timing_points[i];
		printf("%ld: ", tp.time);
		if (!tp.inherit) {
			printf("%g bpm\n", 60000.0 / tp.ms_per_beat);
		} else {
			printf("%gx\n", -100.0 / tp.ms_per_beat);
		}
	}

	printf("\n> %ld hit objects\n", b.num_objects);
	for (size_t i = 0; i < b.num_objects; i++) {

		auto& ho = b.objects[i];
		switch (ho.type) {
			case obj::circle:
				printf("%ld: Circle (%g, %g)\n", ho.time, ho.pos.x, ho.pos.y);
				break;

			case obj::spinner:
				printf("%ld-%ld: Spinner\n", ho.time, ho.end_time);
				break;

			case obj::slider:
			{
				auto& sl = ho.slider;

				printf("%ld-%ld: Slider [Type %c, Length %g, %ld Repetitions] ", 
					ho.time, ho.end_time, sl.type, sl.length, sl.repetitions);

				for (size_t j = 0; j < sl.num_points; j++) {
					auto& pt = sl.points[j];
					printf("(%g, %g) ", pt.x, pt.y);
				}

				puts("");
				break;
			}

			default:
				die("Invalid object type");
		}
	}

	return 0;
}
