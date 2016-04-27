#include "curves.h"

#include <math.h>
#include <algorithm>
#include <string.h>

// ported from osu-web
// TODO: clean up this math mess

bool pt_in_circle(const v2f& pt, const v2f& center, f32 r) {
	return (pt - center).len() <= r;
}

v2f pt_on_line(const v2f& p1, const v2f& p2, f32 t) {
	f32 len = (p2 - p1).len();
	f32 n = len - len * t;
	return (p1 * n + p2 * len) / len;
}

namespace {
	struct circle {
		v2f c;
		f32 r;
	};

	void circum_circle(const v2f& p1, const v2f& p2, const v2f& p3, circle* c) {
		f32 D = 2.f * (p1.x * (p2.y - p3.y) + p2.x * 
				     (p3.y - p1.y) + p3.x * (p1.y - p2.y));

		f32 Ux = ((p1.x * p1.x + p1.y * p1.y) * 
			(p2.y - p3.y) + (p2.x * p2.x + p2.y * p2.y) * 
			(p3.y - p1.y) + (p3.x * p3.x + p3.y * p3.y) * (p1.y - p2.y)) / D;

		f32 Uy = ((p1.x * p1.x + p1.y * p1.y) * 
			(p3.x - p2.x) + (p2.x * p2.x + p2.y * p2.y) * 
			(p1.x - p3.x) + (p3.x * p3.x + p3.y * p3.y) * (p2.x - p1.x)) / D;

  		v2f radius{Ux - p1.x, Uy - p1.y};
  		c->r = radius.len();
		c->c = v2f{Ux, Uy};
	}

	bool is_left(const v2f& a, const v2f& b, const v2f& c) {
		v2f diff1 = b - a;
		v2f diff2 = c - a;
		return diff1.y * diff2.x > diff1.x * diff2.y;
	}

	v2f rotate(const v2f& c, const v2f& p, f32 radians) {
		f32 _cos = cos(radians);
		f32 _sin = sin(radians);
		v2f d = p - c;
		return v2f{
			_cos * d.x - _sin * d.y + c.x, 
			_sin * d.x - _cos * d.y + c.y
		};
	}
}

v2f pt_on_circular_arc(
		const v2f& p1, const v2f& p2, const v2f& p3, f32 t, f32 len) {

	circle c;
	circum_circle(p1, p2, p3, &c);
	f32 radians = (t * len) / c.r;
	if (is_left(p1, p2, p3)) {
		radians *= -1;
	}
	return rotate(c.c, p1, radians);
}

namespace {
	i64 binominial_coefficient(i64 p, i64 n) {
		if (p < 0 || p > n) {
			return 0;
		}

		p = std::min(p, n - p);
		i64 out = 1;
		for (i64 i = 1; i < p + 1; i++) {
			out = out * (n - p + i) / i;
		}

		return out;
	}
}

// ---
namespace {
	const float curve_step = 0.0005f;
}

const size_t curve::max_points;

void curve::init(const v2f* pts, size_t npts) {
	npts = std::min(curve::max_points, npts);
	num_points = npts;
	memcpy(points, pts, npts * sizeof(v2f));
}

f32 curve::len() const {
	f32 res = 0;
	v2f prev = at(0);
	f32 step = curve_step / num_points;

	// TODO: cache length for better performance

	// we calculate length by simply iterating the curve in very small steps
	// and summing the distances between each step as if it was a series
	// of really small straight segments
	for (f32 i = 0; i < 1.f + step; i += step) {
		v2f cur = at(i);
		res += (cur - prev).len();
		prev = cur;
	}

	return res;
}

void curve::compute(std::vector<v2f>* dst) {
	f32 step = curve_step / num_points;
	for (f32 i = 0; i < 1.f + step; i += step) {
		dst->push_back(at(i));
	}
	// TODO: implement it into each curve instead to increase performance
	// exponentially
}

v2f bezier::at(f32 t) const {
	v2f res{0};

	size_t n = num_points - 1;
	for (size_t i = 0; i < num_points; i++) {
		f32 multiplier = 
			binominial_coefficient(i, n) * 
			pow(1.f - t, n - i) *
			pow(t, i);

		res += points[i] * multiplier;
	}

	return res;
}

v2f catmull::at(f32 t) const {
	f32 total_len = 0.f;
	v2f cur{0};
	f32 step = curve_step;

	t *= num_points;

	for (size_t i = 0; i < num_points - 1; i++) {
		for (f32 j = 0; j < 1.f + step; j += step) {
			v2f v1 = (i >= 1 ? points[i-1] : points[i]);
			v2f v2 = points[i];
			v2f v3 = i + 1 < num_points
				? points[i + 1]
				: (v2 * 2.f - v1);
			v2f v4 = i + 2 < num_points
				? points[i + 2]
				: (v3 * 2.f - v2);

			cur = v2f{0};
			cur += (v1 * -1.f + v2 * 3.f - v3 * 3.f + v4) * j * j * j;
			cur += (v1 *  2.f - v2 * 5.f + v3 * 4.f - v4) * j * j;
			cur += (v1 * -1.f + v3                      ) * j;
			cur +=  v2 *  2.f;
			cur /= 2.f;

			total_len += step;

			if (total_len >= t) {
				return cur;
			}
		}
	}

	return cur; // just return the last point
}
