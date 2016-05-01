#include "slider_calc.h"

#include "utils.h"
#include "beatmap.h"
#include "curves.h"

#include <algorithm>

void precompute_slider(slider_data& sl, std::vector<v2f>& positions, 
		f64 px_per_ms) {

	f64 total_distance = 0;

	for (size_t k = 1; k < positions.size(); k++) {
		total_distance += (positions[k] - positions[k - 1]).len();
		if (total_distance >= px_per_ms) {
			sl.pos_at_ms.push_back(positions[k]);
			total_distance -= px_per_ms;
		}
	}
}

v2f slider_at(hit_object& ho, i64 ms) {
	i64 duration = ho.end_time - ho.time;
	auto &sl = ho.slider;

	ms = std::max((i64)0, std::min(ms, duration));
	f32 t = (f32)ms / duration;

	i64 one_repetition = duration / sl.repetitions;
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
			if (sl.points.size() < 2) {
				die("Found linear slider with invalid point count");
				return v2f{0};
			}

			if (sl.points.size() > 2) {
				// too lazy to implement it using lines, just double each 
				// point and make it a bezier

				std::vector<v2f> tmp = sl.points;

				sl.points.clear();
				for (size_t i = 0; i < tmp.size() * 2; i++) {
					sl.points.push_back(tmp[i / 2]);
				}

				sl.points.pop_back();
				sl.type = 'B';

				goto do_precomputed_curve;
			}

			return pt_on_line(sl.points[0], sl.points[1], t);

		case 'P':
			if (sl.points.size() != 3) {
				die("Found pass-through slider with invalid point count");
				return v2f{0};
			}
			goto do_precomputed_curve;

		case 'C':
		case 'B':
		{
			if (sl.points.size() < 2) {
				die("Found catmull or bezier slider with less than 2 points");
				return v2f{0};
			}

do_precomputed_curve:
			if (sl.pos_at_ms.size()) {
				return sl.pos_at_ms[(size_t)ms];
			}

			// pre-calc and cache all positions in millisecond granularity
			// this uses a lot of memory but should boost performance over 
			// calculating positions on demand
			
			f64 px_per_ms = (sl.length * sl.repetitions) / (f64)duration;
			std::vector<v2f> positions;

			if (sl.type != 'B') {
				curve *c = nullptr;
				circular_arc arc;
				catmull cat;

				switch (sl.type) {
				case 'C':
					cat.init(&sl.points[0], sl.points.size());
					c = &cat;
					break;
				case 'P':
					arc.init(&sl.points[0], sl.length);
					c = &arc;
					break;
				default:
					die("How did you get here????????");
					return v2f{0};
				}

				c->compute(&positions);
				precompute_slider(sl, positions, px_per_ms);
				positions.clear();
			}

			else {
				// bezier sliders can have multiple segments and each segment is
				// a separate bezier. overlapped slider points mark the end of 
				// a segment.
				bezier bez;
				size_t last_segment = 0;

				for (size_t j = 0; j < sl.points.size(); j++) {
					if (j == 0) {
						continue;
					}

					// is this the last point in the slider?
					bool last = j == sl.points.size() - 1;

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

					// compute this segment
					bez.init(&sl.points[last_segment], j - last_segment);
					last_segment = j;
		
					bez.compute(&positions);
					precompute_slider(sl, positions, px_per_ms);
				}
			}

			// just in case we end up with less positions than the slider len
			while (sl.pos_at_ms.size() < (size_t)duration) {
				sl.pos_at_ms.push_back(sl.pos_at_ms.back());
			}

			return sl.pos_at_ms[(size_t)ms];
		}
	}

	die("Unsupported slider type");
	return v2f{0};
}
