#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <ctype.h> // tolower/toupper

#include "utils.h"
#include "beatmap.h"
#include "preview_window.h"
#include "diff_calc.h"
#include "pp_calc.h"

namespace {
	beatmap b;
	void print_beatmap();

	const char* const mod_strs[] {
		"nomod", "nf", "ez", "hd", "hr", "dt", "ht", "nc", "fl", "so"
	};
	const u32 mod_masks[] {
		mods::nomod, mods::nf, mods::ez, mods::hd, mods::hr, mods::dt, mods::ht,
		mods::nc, mods::fl, mods::so
	};
	const size_t num_mods = sizeof(mod_masks) / sizeof(mod_masks[0]);

	void chk() {
		if (!err()) {
			return;
		}

		fputs(err(), stderr);
		fputs("\n", stderr);
		exit(1);
	}
}

int main(int argc, char* argv[]) {
	puts("o p p a i | v0.3.3");
	puts("s     d n | ");
	puts("u     v s | (looking for");
	puts("!     a p | cool ascii");
	puts("      n e | to put here)");
	puts("      c c | ");
	puts("      e t | ");
	puts("      d o | ");
	puts("        r |\n");

	if (argc < 2) {
		printf("Usage: %s /path/to/difficulty.osu "
				"{[acc]%% or [num_100s]x100 [num_50s]x50} +[mods] "
				"[combo]x [misses]m scorev[scoring_version]\n\n", *argv);
		puts("acc: the accuracy in percent (example: 99.99%)");
		puts("num_100s, num_50s: used to specify accuracy in 100 and 50 count");
		puts("mods: any combination of nomod, nf, ez, hd, hr, dt, ht"
				", nc, fl, so (example: +hddthr)");
		puts("combo: the highest combo (example: 1337x)");
		puts("misses: amount of misses (example: 1m)");
		puts("scoring_version: can only be 1 or 2 (example: scorev2)");
		puts("\narguments in [square brackets] are optional");
		puts("(the order of the optional arguments does not matter)");

		return 0;
	}

	beatmap::parse(argv[1], b);
	chk();

	char* mods_str = nullptr;
	f64 acc = 0;
	u32 mods = mods::nomod;
	u16 combo = b.max_combo;
	u16 misses = 0;
	u32 scoring = 1;
	u16 c100 = 0, c50 = 0;
	bool no_percent = false;

	dbgputs("\nparsing arguments");
	for (int i = 2; i < argc; i++) {
		char suff[64] = {0};
		auto a = argv[i];

		std::transform(a, a + strlen(a), a, tolower);

		// acc
		f64 tmp_acc;
		if (sscanf(a, "%lf%s", &tmp_acc, suff) == 2 && !strcmp(suff, "%")) {
			acc = tmp_acc;
			continue;
		}

		// 100s, 50s
		u16 tmp_c100 = 0, tmp_c50 = 0;
		if (sscanf(a, "%" fu16 "%s", &tmp_c100, suff) == 2 && 
			!strcmp(suff, "x100")) {

			c100 = tmp_c100;
			no_percent = true;
			continue;
		}

		if (sscanf(a, "%" fu16 "%s", &tmp_c50, suff) == 2 && 
			!strcmp(suff, "x50")) {

			c50 = tmp_c50;
			no_percent = true;
			continue;
		}

		// mods
		char* tmp_mods_str = nullptr;
		for (size_t j = 0; j < num_mods; j++) {
			if (strstr(a, mod_strs[j])) {
				tmp_mods_str = a;
				mods |= mod_masks[j];
			}
		}

		if (tmp_mods_str == a && *tmp_mods_str == '+') {
			// at least one mod found in the parameter and the prefix matches
			mods_str = tmp_mods_str;
			std::transform(mods_str, mods_str + strlen(mods_str), mods_str, 
					toupper);
			continue;
		}

		// combo
		u16 tmp_combo;
		if (sscanf(a, "%" fu16 "%s", &tmp_combo, suff) == 2 && 
			!strcmp(suff, "x")) {

			combo = tmp_combo;
			continue;
		}

		// misses
		u16 tmp_misses;
		if (sscanf(a, "%" fu16 "%s", &tmp_misses, suff) == 2 && 
				(!strcmp(suff, "xm") || !strcmp(suff, "xmiss") ||
				 !strcmp(suff, "m"))) {
			misses = tmp_misses;
			continue;
		}

		// scorev1 / scorev2
		u32 tmp_scoring;
		if (sscanf(a, "scorev%" fu32, &tmp_scoring) == 1) {
			scoring = tmp_scoring;
			continue;
		}

		printf(">%s\n", a);
		die("Invalid parameter");
		break;
	}

	chk();

	p_init(argc, argv);
	chk();

	print_beatmap();
	chk();

	printf("\n%s - %s [%s] (%s) %s\n", 
			b.artist, b.title, b.version, b.creator, mods_str ? mods_str : "");

	b.apply_mods(mods);
	chk();

	printf("od%g ar%g cs%g\n", b.od, b.ar, b.cs);
	printf("%" fu16 "/%" fu16 " combo\n", combo, b.max_combo);
	printf("%" fu16 " circles, %" fu16 " sliders %" fu16 " spinners\n", 
			b.num_circles, b.num_sliders, b.num_spinners);
	printf("%" fu16 "xmiss\n", misses);
	printf("scorev%" fu32"\n\n", scoring);

	f64 aim, speed;
	f64 stars = d_calc(b, &aim, &speed);
	chk();
	printf("\n%g stars\naim stars: %g, speed stars: %g\n", stars, aim, speed);

	f64 pp = no_percent ? 
		pp_calc(aim, speed, b, mods, combo, misses, 0xFFFF, c100, c50, scoring)
	  : pp_calc_acc(aim, speed, b, acc, mods, combo, misses, scoring);
	chk();
	printf("\n%gpp\n", pp);

	return 0;
}

namespace {
	void print_beatmap() {
#ifdef SHOW_BEATMAP
		printf(
			"Format version: %" fi32 "\n"
			"Stack Leniency: %g\n"
			"Mode: %" fi32 "\n"
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

		printf("> %zd timing points\n", b.num_timing_points);
		for (size_t i = 0; i < b.num_timing_points; i++) {
			auto& tp = b.timing_points[i];
			printf("%" fi64 ": ", tp.time);
			if (!tp.inherit) {
				printf("%g bpm\n", 60000.0 / tp.ms_per_beat);
			} else {
				printf("%gx\n", -100.0 / tp.ms_per_beat);
			}
		}

		printf("\n> %zd hit objects\n", b.num_objects);
#endif
		for (size_t i = 0; i < b.num_objects; i++) {

			auto& ho = b.objects[i];
			switch (ho.type) {
#ifdef SHOW_BEATMAP
				case obj::circle:
					printf("%" fi64 ": Circle (%g, %g)\n", 
						ho.time, ho.pos.x, ho.pos.y);
					break;

				case obj::spinner:
					printf("%" fi64 "-%" fi64 ": Spinner\n", 
							ho.time, ho.end_time);
					break;
#else
			case obj::circle:
			case obj::spinner:
				break;
#endif

				case obj::slider:
				{
#ifdef SHOW_BEATMAP
					auto& sl = ho.slider;

					printf(
						"%" fi64 "-%" fi64 ": Slider "
						"[Type %c, Length %g, %" fu16 " Repetitions] ", 
						ho.time, ho.end_time, sl.type, 
						sl.length, sl.repetitions);

					for (size_t j = 0; j < sl.points.size(); j++) {
						auto& pt = sl.points[j];
						printf("(%g, %g) ", pt.x, pt.y);
					}

					puts("");
#endif

					p_show(ho);
					break;
				}

				default:
					die("Invalid object type");
					return;
			}
		}
	}
}
