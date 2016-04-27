#include <stdio.h>
#include <algorithm>
#include <vector>

#include "utils.h"
#include "v2f.h"
#include "beatmap.h"
#include "preview_window.h"

namespace {
	beatmap b;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: %s /path/to/difficulty.osu\n", *argv);
		return 0;
	}

	puts("\n---");
	puts(argv[1]);
	puts("---\n");

	beatmap::parse(argv[1], b);

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

	p_init(argc, argv);

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

				p_show(ho);
				break;
			}

			default:
				die("Invalid object type");
		}
	}

	return 0;
}
