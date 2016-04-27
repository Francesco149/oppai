#include "slider_calc.h"

#include "utils.h"
#include "beatmap.h"
#include "curves.h"

void precompute_slider(slider_data& sl, std::vector<v2f>& positions, 
		f32 px_per_ms) {

	f32 total_distance = 0;

	for (size_t k = 1; k < positions.size(); k++) {
		total_distance += (positions[k] - positions[k - 1]).len();
		if (total_distance >= px_per_ms) {
			sl.pos_at_ms.push_back(positions[k]);
			total_distance -= px_per_ms;
		}
	}
}

v2f slider_at(hit_object& ho, i64 ms) {
	auto duration = ho.end_time - ho.time;
	auto &sl = ho.slider;
	f32 t = (f32)ms / duration;

	ms = std::max((i64)0, std::min(ms, duration));

	auto one_repetition = duration / sl.repetitions;
	bool invert = false;
	while (ms > (i64)one_repetition) {
		ms -= one_repetition;
		invert ^= true;
	}
	if (invert) {
		ms = one_repetition - ms;
	}

	switch (sl.type) {
		case 'L':
			if (sl.num_points < 2) {
				die("Found linear slider with invalid point count");
			}
			if (sl.num_points > 2) {
				// too lazy to implement it using lines, just double each 
				// point and make it a bezier
				
				v2f tmp[0xFF];
				for (size_t i = 0; i < sl.num_points; i++) {
					tmp[i] = sl.points[i];
				}

				sl.num_points *= 2;
				for (size_t i = 0; i < sl.num_points; i++) {
					sl.points[i] = tmp[i / 2];
				}

				sl.num_points--;
				sl.type = 'B';

				goto do_bezier;
			}
			return pt_on_line(sl.points[0], sl.points[1], t);

		case 'P':
			if (sl.num_points != 3) {
				die("Found pass-through slider with invalid point count");
			}
			return pt_on_circular_arc(
				sl.points[0], sl.points[1], sl.points[2], t, sl.length);

		case 'B':
		{
			if (sl.num_points < 2) {
				die("Found bezier slider with less than 2 points");
			}

		case 'C':
do_bezier:
			if (sl.pos_at_ms.size()) {
				return sl.pos_at_ms[ms];
			}

			// pre-calc and cache all positions in millisecond granularity
			// this is lazy and uses a lot of memory but #yolo
			
			f32 px_per_ms = (sl.length * sl.repetitions) / (f32)duration;
			std::vector<v2f> positions;

			if (sl.type == 'C') {
				catmull cat;
				cat.init(sl.points, sl.num_points);
				cat.compute(&positions);
				precompute_slider(sl, positions, px_per_ms);
				positions.clear();
			}

			else {
				bezier bez;
				size_t last_segment = 0;

				for (size_t j = 0; j < sl.num_points; j++) {
					if (j == 0) {
						continue;
					}

					bool last = j == sl.num_points - 1;

					if ((sl.points[j] - sl.points[j-1]).len() 
							> 0.0001f && !last) {
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

					bez.init(&sl.points[last_segment], j - last_segment);
					last_segment = j;
		
					// first compute positions, then normalize them on time by
					// calculating how much distance should be travelled for 
					// each millisecond
					bez.compute(&positions);
					precompute_slider(sl, positions, px_per_ms);
					positions.clear(); 
					// leaves vector mem allocated for reusage
				}
			}

			while (sl.pos_at_ms.size() < (size_t)duration) {
				sl.pos_at_ms.push_back(sl.pos_at_ms.back());
			}

			return sl.pos_at_ms[ms];
		}

		default:
			die("Unsupported slider type");
	}

	die("how did you get here");
	return v2f{0};
}
