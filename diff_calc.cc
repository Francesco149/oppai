#include "diff_calc.h"
#include "common.h"

#include <math.h>
#include <algorithm>

#if NEEDS_TO_INSTALL_GENTOO
#include <functional> // std::greater
#endif

#include "utils.h"
#include "beatmap.h"

// based on tom94's osu!tp aimod
// TODO: reduce code redudnancy and rename variables to shorter names

namespace {
	const f64 decay_base[] = { 0.3, 0.15 };
	const f64 almost_diameter = 90;
	const f64 stream_spacing_treshold = 110;
	const f64 single_spacing_treshold = 125;
	const f64 spacing_weight_scaling[] = { 1400, 26.25 };
	const i64 lazy_slider_step = 10;
	const f64 circlesize_buff_treshold = 30;

	namespace diff {
		const u8 speed = 0, 
				 aim = 1;
	}

	// diffcalc hit object
	struct d_obj {
		hit_object* ho;
		f64 strains[2] = { 1, 1 };
		v2f norm_start_pos;
		v2f norm_end_pos;
		f64 lazy_slider_len_first = 0;
		f64 lazy_slider_len_subseq = 0;

		void init(hit_object* base_object, f64 radius) {
			this->ho = base_object;

			f64 scaling_factor = 52.0 / radius;

			// cs buff (based on osuElements, not accurate)
			if (radius < circlesize_buff_treshold) {
				scaling_factor *= std::min(1.1, 
					1 + (circlesize_buff_treshold - radius) * 0.02);
			}

			norm_start_pos = ho->pos * scaling_factor;
			
			// just a circle bro
			if (ho->type != obj::slider) {
				norm_end_pos = norm_start_pos;
				return;
			}

			f64 follow_circle_rad = radius * 3;

			auto& sl = ho->slider;
			i64 segment_len = (ho->end_time - ho->time) / sl.repetitions;
			i64 segment_end_time = ho->time + segment_len;
			v2f cursor = ho->pos;

			for (i64 t = ho->time + lazy_slider_step; t < segment_end_time; 
					t+= lazy_slider_step) {
				
				v2f d = ho->at(t - ho->time) - cursor;
				f64 dist = d.len();

				if (dist <= follow_circle_rad) {
					continue;
				}

				d.norm();
				dist -= follow_circle_rad;
				cursor += d * dist;
				lazy_slider_len_first += dist;
			}

			lazy_slider_len_first *= scaling_factor;
			lazy_slider_len_first = 0.0;
			// for some crazy reason the pp values match only if I entirely 
			// remove slider lengths, otherwise I get too much pp
			// TODO: ask tom about this, were slider length weights removed?

			if (sl.repetitions % 2 == 1) {
				norm_end_pos = cursor * scaling_factor;
			}
	
			if (sl.repetitions < 2) {
				return;
			}

			segment_end_time += segment_len;

			for (i64 t = segment_end_time - segment_len + lazy_slider_step; 
					t < segment_end_time; 
					t+= lazy_slider_step) {
				
				v2f d = ho->at(t - ho->time) - cursor;
				f64 dist = d.len();

				if (dist <= follow_circle_rad) {
					continue;
				}

				d.norm();
				dist -= follow_circle_rad;
				cursor += d * dist;
				lazy_slider_len_subseq += dist;
			}

			lazy_slider_len_subseq *= scaling_factor;
			lazy_slider_len_subseq = 0.0; // see above

			if (sl.repetitions % 2 == 1) {
				norm_end_pos = cursor * scaling_factor;
			}
		}

		void calculate_strains(d_obj& prev) {
			calculate_strain(prev, diff::speed);
			calculate_strain(prev, diff::aim);
		}

		void calculate_strain(d_obj& prev, u8 diff_type) {
			f64 res = 0;
			i64 time_elapsed = ho->time - prev.ho->time;
			f64 decay = pow(decay_base[diff_type], time_elapsed / 1000.0);
			f64 scaling = spacing_weight_scaling[diff_type];

			switch (ho->type) {
				case obj::circle:
					res = spacing_weight(distance(prev), diff_type) * scaling;
					break;

				case obj::slider:
					switch(diff_type) {
						case diff::speed:
#ifdef SLIDERFIX
							time_elapsed = ho->end_time - prev.ho->time;
							res	= spacing_weight(
								lazy_slider_len_first +
								lazy_slider_len_subseq * 
									(ho->slider.repetitions - 1) +
								distance(prev), diff_type) * scaling;
#else
							res	= spacing_weight(
								prev.lazy_slider_len_first +
								prev.lazy_slider_len_subseq * 
									(prev.ho->slider.repetitions - 1) +
								distance(prev), diff_type) * scaling;
#endif
							break;

						case diff::aim:
#ifdef SLIDERFIX
							time_elapsed = ho->end_time - prev.ho->time;
							res = (
								spacing_weight(
									lazy_slider_len_first, diff_type) +
								spacing_weight(
									lazy_slider_len_subseq, diff_type) *
										(ho->slider.repetitions - 1) +
								spacing_weight(distance(prev), diff_type)
							) * scaling;
#else
							res = (
								spacing_weight(
									prev.lazy_slider_len_first, diff_type) +
								spacing_weight(
									prev.lazy_slider_len_subseq, diff_type) *
										(prev.ho->slider.repetitions - 1) +
								spacing_weight(distance(prev), diff_type)
							) * scaling;
#endif
					}
					break;

				case obj::spinner:
				case obj::invalid:
					break;
			}

			res /= std::max(time_elapsed, (i64)50);
			strains[diff_type] = prev.strains[diff_type] * decay + res;
		}

		f64 spacing_weight(f64 distance, u8 diff_type) {
			switch (diff_type) {
				case diff::speed:
					if (distance > single_spacing_treshold) {
						return 2.5;
					}
					else if (distance > stream_spacing_treshold) {
						return 1.6 + 0.9 *
							(distance - stream_spacing_treshold) /
							(single_spacing_treshold - stream_spacing_treshold);
					}
					else if (distance > almost_diameter) {
						return 1.2 + 0.4 * (distance - almost_diameter)
							/ (stream_spacing_treshold - almost_diameter);
					}
					else if (distance > almost_diameter / 2.0) {
						return 0.95 + 0.25 * 
							(distance - (almost_diameter / 2.0)) /
							(almost_diameter / 2.0);
					}
					return 0.95;

				case diff::aim:
					return pow(distance, 0.99);

				default:
					return 0.0;
			}
		}

		f64 distance(d_obj& prev) {
			return (norm_start_pos - prev.norm_end_pos).len();
		}
	};

	const f64 star_scaling_factor = 0.0675;
	const f64 extreme_scaling_factor = 0.5;
	const f64 playfield_width = 512;

	const i64 strain_step = 400;
	const f64 decay_weight = 0.9;

	d_obj objects[beatmap::max_objects];
	size_t num_objects;

	f64 calculate_difficulty(u8 type) {
		std::vector<f64> highest_strains;
		i64 interval_end = strain_step;
		f64 max_strain = 0.0;

		d_obj* prev = nullptr;
		for (size_t i = 0; i < num_objects; i++) {
			auto& o = objects[i];

			while (o.ho->time > interval_end) {
				highest_strains.push_back(max_strain);

				if (!prev) {
					max_strain = 0.0;
				} else {
					f64 decay = pow(decay_base[type], 
						(interval_end - prev->ho->time) / 1000.0);
					max_strain = prev->strains[type] * decay;
				}

				interval_end += strain_step;
			}
		
			max_strain = std::max(max_strain, o.strains[type]);
			prev = &o;
		}

		f64 difficulty = 0;
		f64 weight = 1.0;

		std::sort(highest_strains.begin(), highest_strains.end(), 
			std::greater<f64>());

		for (const f64& strain : highest_strains) {
			difficulty += weight * strain;
			weight *= decay_weight;
		}

		return difficulty;
	}
}

f64 d_calc(beatmap& b, f64* aim, f64* speed) {
	dbgputs("\ndiff calc");

	f64 circle_radius = (playfield_width / 16.f) * (1.f - 0.7f *
			(b.cs - 5.f) / 5.f);

	dbgprintf("circle radius: %g\n", circle_radius);

	num_objects = b.num_objects;
	dbgputs("initializing objects");
	for (size_t i = 0; i < b.num_objects; i++) {
		objects[i].init(&b.objects[i], circle_radius);
	}

	auto* prev = &objects[0];
	for (size_t i = 1; i < b.num_objects; i++) {
		auto& o = objects[i];
		o.calculate_strains(*prev);
		dbgprintf("%" fi64 ": type %d, strains %g %g, slider lens %g %g, "
				"norm pos %s-%s, pos %s-%s\n", 
				o.ho->time, (int)o.ho->type, o.strains[0], o.strains[1], 
				o.lazy_slider_len_first, o.lazy_slider_len_subseq, 
				o.norm_start_pos.str(), o.norm_end_pos.str(), o.ho->pos.str(), 
				o.ho->at(o.ho->end_time - o.ho->time).str());
		prev = &o;
	}

	*aim = calculate_difficulty(diff::aim);
	*speed = calculate_difficulty(diff::speed);
	*aim = sqrt(*aim) * star_scaling_factor;
	*speed = sqrt(*speed) * star_scaling_factor;

	// round to 2 decimal places
	*aim = std::round(*aim * 100.0) / 100.0;
	*speed = std::round(*speed * 100.0) / 100.0;

	f64 stars = *aim + *speed + 
		std::abs(*speed - *aim) * extreme_scaling_factor;

	stars = std::round(stars * 100.0) / 100.0;

	return stars;
}
