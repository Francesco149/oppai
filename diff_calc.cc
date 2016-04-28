#include "diff_calc.h"

#include "beatmap.h"
#include <math.h>
#include <algorithm>

// based on tom94's osu!tp aimod
// TODO: reduce code redudnancy and rename variables to shorter names

namespace {
	const f32 decay_base[] = { 0.3f, 0.15f };
	const f32 almost_diameter = 90.f;
	const f32 stream_spacing_treshold = 110.f;
	const f32 single_spacing_treshold = 125.f;
	const f32 spacing_weight_scaling[] = { 1400.f, 26.25f };
	const i64 lazy_slider_step = 10;

	namespace diff {
		const u8 speed = 0, 
				 aim = 1;
	}

	// diffcalc hit object
	struct d_obj {
		hit_object* ho;
		f32 strains[2] = { 1.f, 1.f };
		v2f norm_start_pos;
		v2f norm_end_pos;
		f32 lazy_slider_len_first = 0.f;
		f32 lazy_slider_len_subseq = 0.f;

		void init(hit_object* ho, f32 radius) {
			this->ho = ho;

			f32 scaling_factor = (52.f / radius);
			norm_start_pos = ho->pos * scaling_factor;
			
			// just a circle bro
			if (ho->type != obj::slider) {
				norm_end_pos = norm_start_pos;
				return;
			}

			f32 follow_circle_rad = radius * 3.f;
			i64 segment_len = (ho->end_time - ho->time) / ho->num_segments();
			i64 segment_end_time = ho->time + segment_len;

			v2f cursor = ho->pos;

			for (i64 t = ho->time + lazy_slider_step; t < segment_end_time; 
					t+= lazy_slider_step) {
				
				v2f d = ho->at(t - ho->time) - cursor;
				f32 dist = d.len();

				if (dist <= follow_circle_rad) {
					continue;
				}

				d.norm();
				dist -= follow_circle_rad;
				cursor += d * dist;
				lazy_slider_len_first += dist;
			}

			lazy_slider_len_first *= scaling_factor;

			if (ho->num_segments() % 2 == 1) {
				norm_end_pos = cursor * scaling_factor;
			}
	
			if (ho->num_segments() < 2) {
				return;
			}

			segment_end_time += segment_len;

			for (i64 t = segment_end_time - segment_len + lazy_slider_step; 
					t < segment_end_time; 
					t+= lazy_slider_step) {
				
				v2f d = ho->at(t - ho->time) - cursor;
				f32 dist = d.len();

				if (dist <= follow_circle_rad) {
					continue;
				}

				d.norm();
				dist -= follow_circle_rad;
				cursor += d * dist;
				lazy_slider_len_subseq += dist;
			}

			lazy_slider_len_subseq *= scaling_factor;

			if (ho->num_segments() % 2 == 1) {
				norm_end_pos = cursor * scaling_factor;
			}
		}

		void calculate_strains(d_obj& prev) {
			calculate_strain(prev, diff::speed);
			calculate_strain(prev, diff::aim);
		}

		void calculate_strain(d_obj& prev, u8 diff_type) {
			f32 res = 0.f;
			i64 time_elapsed = ho->time - prev.ho->time;
			f32 decay = pow(decay_base[diff_type], time_elapsed / 1000.f);
			f32 scaling = spacing_weight_scaling[diff_type];

			switch (ho->type) {
				case obj::circle:
					res = spacing_weight(distance(prev), diff_type) * scaling;
					break;

				case obj::slider:
					switch(diff_type) {
						case diff::speed:
							res	= spacing_weight(
								prev.lazy_slider_len_first +
								prev.lazy_slider_len_subseq * 
									(prev.ho->num_segments() - 1) +
								distance(prev), diff_type) * scaling;
							break;

						case diff::aim:
							res = (
								spacing_weight(
									prev.lazy_slider_len_first, diff_type) +
								spacing_weight(
									prev.lazy_slider_len_subseq, diff_type) *
										(prev.ho->num_segments() - 1) +
								spacing_weight(distance(prev), diff_type)
							) * scaling;
					}
					break;

				case obj::spinner:
				case obj::invalid:
					break;
			}

			res /= std::max(time_elapsed, (i64)50);
			strains[diff_type] = prev.strains[diff_type] * decay + res;
		}

		f32 spacing_weight(f32 distance, u8 diff_type) {
			switch (diff_type) {
				case diff::speed:
					if (distance > single_spacing_treshold) {
						return 2.5f;
					}
					else if (distance > stream_spacing_treshold) {
						return 1.6f + 0.9f *
							(distance - stream_spacing_treshold) /
							(single_spacing_treshold - stream_spacing_treshold);
					}
					else if (distance > almost_diameter) {
						return 1.2f + 0.4f * (distance - almost_diameter)
							/ (stream_spacing_treshold - almost_diameter);
					}
					else if (distance > almost_diameter / 2.f) {
						return 0.95f + 0.25f * 
							(distance - (almost_diameter / 2.f)) /
							(almost_diameter / 2.f);
					}
					return 0.95f;

				case diff::aim:
					return pow(distance, 0.99f);

				default:
					return 0.f;
			}
		}

		f32 distance(d_obj& prev) {
			return (norm_start_pos - prev.norm_end_pos).len();
		}
	};

	const f32 star_scaling_factor = 0.0675f;
	const f32 extreme_scaling_factor = 0.5f;
	const f32 playfield_width = 512.f;

	const i64 strain_step = 400;
	const f32 decay_weight = 0.9f;

	d_obj objects[beatmap::max_objects];
	size_t num_objects;

	f32 calculate_difficulty(u8 type) {
		std::vector<f32> highest_strains;
		i64 interval_end = strain_step;
		f32 max_strain = 0.f;

		d_obj* prev = nullptr;
		for (size_t i = 0; i < num_objects; i++) {
			auto& o = objects[i];

			while (o.ho->time > interval_end) {
				highest_strains.push_back(max_strain);

				if (!prev) {
					max_strain = 0.f;
				} else {
					f32 decay = pow(decay_base[type], 
						(interval_end - prev->ho->time) / 1000.f);
					max_strain = prev->strains[type] * decay;
				}

				interval_end += strain_step;
			}
		
			max_strain = std::max(max_strain, o.strains[type]);
			prev = &o;
		}

		f32 difficulty = 0;
		f32 weight = 1.f;

		std::sort(highest_strains.begin(), highest_strains.end(), 
			std::greater<f32>());

		for (const f32& strain : highest_strains) {
			difficulty += weight * strain;
			weight *= decay_weight;
		}

		return difficulty;
	}
}

f32 d_calc(beatmap& b, f32* aim, f32* speed) {
	f32 circle_radius = (playfield_width / 16.f) * (1.f - 0.7f *
			(b.cs - 5.f) / 5.f);

	num_objects = b.num_objects;
	for (size_t i = 0; i < b.num_objects; i++) {
		objects[i].init(&b.objects[i], circle_radius);
	}

	auto* prev = &objects[0];
	for (size_t i = 1; i < b.num_objects; i++) {
		auto& o = objects[i];
		o.calculate_strains(*prev);
		/*printf("%ld %d %g %g %g %g %s %s %s %s\n", 
				o.ho->time, (int)o.ho->type, o.strains[0], o.strains[1], 
				o.lazy_slider_len_first, o.lazy_slider_len_subseq, 
				o.norm_start_pos.str(), o.norm_end_pos.str(), o.ho->pos.str(), 
				o.ho->at(o.ho->end_time - o.ho->time).str());*/
		//getchar();
		prev = &o;
	}

	*aim = calculate_difficulty(diff::aim);
	*speed = calculate_difficulty(diff::speed);

	*aim = sqrtf(*aim) * star_scaling_factor;
	*speed = sqrtf(*speed) * star_scaling_factor;
	return *aim + *speed + std::abs(*speed - *aim) * extreme_scaling_factor;
}
