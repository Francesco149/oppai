#ifdef SLIDERTEST
#ifdef _WIN32
#define NEEDS_TO_INSTALL_GENTOO
#include <Windows.h>
#endif
#include <GL/gl.h>
#include <GL/freeglut.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

#include "v2f.h"
#include "curves.h"

// shit code ahead! I am way too lazy to write nice code for parsers, sorry
// disclaimer: this beatmap parser is meant purely for difficulty calculation 
//             and I don't support any malicious use of it.

namespace {
	struct slider_data {
		char type;

		static const size_t max_points = 0xFF;
		size_t num_points = 0;
		v2f points[max_points];

		u64 repetitions = 0;
		f32 length = 0;
	};

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

		static const size_t max_timing_points = 0xFFFF;
		size_t num_timing_points = 0;
		timing_point timing_points[max_timing_points];

		static const size_t max_objects = 0xFFFF;
		size_t num_objects = 0;
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

	beatmap b;

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

#ifdef SLIDERTEST
	// yes this is extremely bad and slow but I don't care, it's just for testin
	const i32 screen_w = 800, screen_h = 600;
	f32 pixels[screen_w * screen_h][3];

	void slider_display() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawPixels(screen_w, screen_h, GL_RGB, GL_FLOAT, pixels);
		glutSwapBuffers();
	}

	bool showing_slider;
	void slider_keyboard(u8 key, i32 x, i32 y) {
		showing_slider = false;
	}

	f32* get_px(const v2f& pos) {
		i32 int_x = (int)pos.x;
		i32 int_y = (int)pos.y;

		if (int_x < 0 || int_x >= screen_w || int_y < 0 || int_y >= screen_h) {
			return nullptr;
		}

		return (f32*)pixels[(screen_h - (i32)int_y - 1) 
			* screen_w + (i32)int_x];
	}

	void shigeZZZ(u64 ms) {
#ifdef NEEDS_TO_INSTALL_GENTOO
		puts("INSTALL GENTOO");
		Sleep(ms);
#else
		usleep(ms * 1000);
#endif
	}
#endif
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

#ifdef SLIDERTEST
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("slider");
	glutDisplayFunc(slider_display);
	glutIdleFunc(slider_display);
	glutKeyboardFunc(slider_keyboard);
					
	glClearColor(0.f, 0.f, 0.f, 1.f);
#endif

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

#ifdef SLIDERTEST
				bezier bez;
				catmull cat;
				curve *c = nullptr;
				switch (sl.type) {
					case 'B':
						c = &bez;
						break;

					case 'C':
						c = &cat;
						break;

					default:
						puts("unsupported slider type");
						continue;
				}

				puts("\n-- TEST SLIDER SIMULATION --");
				memset(pixels, 0, screen_w * screen_h * sizeof(float) * 3);

				auto duration = ho.end_time - ho.time;
				f32 step = 1.f / duration; // 1ms step

				size_t last_segment = 0;
				for (size_t j = 0; j < sl.num_points; j++) {
					if (j == 0) {
						continue;
					}

					bool last = j == sl.num_points - 1;

					if ((sl.points[j] - sl.points[j-1]).len() > 0.0001f && 
						!last) {
						
						// not the end of a segment and not the sliderend
						continue;
					}

					if (j == 1 && !last) {
						// old sliders have double points on the first
						// segment
						// we also have to check if this is the last point
						// so that 2-point sliders still work
						last_segment = 1;
						continue;
					}


					if (last) {
						j++;
					}

					c->init(&sl.points[last_segment], j - last_segment);
					last_segment = j;
					v2f pos;
					u64 steps = 0;
					for (f32 t = 0; t < 1.f + step; t += step) {
						pos = c->at(t);

						if (steps % 50 == 0) {
							printf("%ld ms - %s\n", 
								(i64)std::round(t * duration), 
								pos.str());
						}
						steps++;

						pos *= 1.5f;
						auto pix = get_px(pos);
						if (!pix) {
							continue;
						}
						pix[0] = 1.f;
					}
				}

				// draw slider points
				for (size_t j = 0; j < sl.num_points; j++) {
					v2f pt = sl.points[j];
					pt *= 1.5f;
					auto pix = get_px(pt);
					if (!pix) {
						continue;
					}
					pix[0] = 1.f;
					pix[1] = 1.f;
				}

				puts("Press any key in the slider window to continue...");
				showing_slider = true;
				while (showing_slider) {
					glutMainLoopEvent();
					glutPostRedisplay();
					shigeZZZ(50);
				}
				puts("----------------------------\n");
#endif
				break;
			}

			default:
				die("Invalid object type");
		}
	}

	return 0;
}
