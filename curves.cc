#include "curves.h"

#include <math.h>
#include <algorithm>
#include <string.h>

// ported from osu-web
// TODO: clean up this math mess

bool pt_in_circle(const v2f& pt, const v2f& center, f64 r) {
	return (pt - center).len() <= r;
}

v2f pt_on_line(const v2f& p1, const v2f& p2, f64 t) {
	t = std::min(1.0, std::max(0.0, t));
	f64 len = (p2 - p1).len();
	f64 n = len - len * t;
	return (p1 * n + p2 * len * t) / len;
}

namespace {
	struct circle {
		v2f c;
		f64 r;
	};

	void circum_circle(const v2f& p1, const v2f& p2, const v2f& p3, circle* c) {
		f64 D = 2.0 * (p1.x * (p2.y - p3.y) + p2.x * 
				     (p3.y - p1.y) + p3.x * (p1.y - p2.y));

		f64 Ux = ((p1.x * p1.x + p1.y * p1.y) * 
			(p2.y - p3.y) + (p2.x * p2.x + p2.y * p2.y) * 
			(p3.y - p1.y) + (p3.x * p3.x + p3.y * p3.y) * (p1.y - p2.y)) / D;

		f64 Uy = ((p1.x * p1.x + p1.y * p1.y) * 
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

	v2f rotate(const v2f& c, const v2f& p, f64 radians) {
		f64 _cos = cos(radians);
		f64 _sin = sin(radians);
		v2f d = p - c;
		return v2f{
			_cos * d.x - _sin * d.y + c.x, 
			_sin * d.x + _cos * d.y + c.y
		};
	}
}

v2f pt_on_circular_arc(
		const v2f& p1, const v2f& p2, const v2f& p3, f64 t, f64 len) {

	t = std::min(1.0, std::max(0.0, t));

	circle c;
	circum_circle(p1, p2, p3, &c);
	f64 radians = (t * len) / c.r;
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
	const f64 curve_step = 0.0005;
}

const size_t curve::max_points;

void curve::init(const v2f* pts, size_t npts) {
	npts = std::min(curve::max_points, npts);
	num_points = npts;
	memcpy(points, pts, npts * sizeof(v2f));
}

f64 curve::len() const {
	f64 res = 0;
	v2f prev = at(0);
	f64 step = curve_step / num_points;

	// TODO: cache length for better performance

	// we calculate length by simply iterating the curve in very small steps
	// and summing the distances between each step as if it was a series
	// of really small straight segments
	for (f64 i = 0; i < 1.0 + step; i += step) {
		v2f cur = at(i);
		res += (cur - prev).len();
		prev = cur;
	}

	return res;
}

void curve::compute(std::vector<v2f>* dst) {
	f64 step = curve_step / num_points;
	for (f64 i = 0; i < 1.f + step; i += step) {
		dst->push_back(at(i));
	}
}

v2f bezier::at(f64 t) const {
	v2f res{0};

	t = std::min(1.0, std::max(0.0, t));
	size_t n = num_points - 1;
	for (size_t i = 0; i < num_points; i++) {
		f64 multiplier = 
			binominial_coefficient(i, n) * 
			pow(1.0 - t, n - i) *
			pow(t, i);

		res += points[i] * multiplier;
	}

	return res;
}

v2f catmull::at(f64 t) const {
	f64 total_len = 0.0;
	v2f cur{0};
	f64 step = curve_step;

	t = std::min(1.0, std::max(0.0, t));
	t *= num_points;

	for (size_t i = 0; i < num_points - 1; i++) {
		for (f64 j = 0; j < 1.0 + step; j += step) {
			v2f v1 = (i >= 1 ? points[i-1] : points[i]);
			v2f v2 = points[i];
			v2f v3 = i + 1 < num_points
				? points[i + 1]
				: (v2 * 2.0 - v1);
			v2f v4 = i + 2 < num_points
				? points[i + 2]
				: (v3 * 2.0 - v2);

			cur = v2f{0};
			cur += (v1 * -1.0 + v2 * 3.0 - v3 * 3.0 + v4) * j * j * j;
			cur += (v1 *  2.0 - v2 * 5.0 + v3 * 4.0 - v4) * j * j;
			cur += (v1 * -1.0 + v3                      ) * j;
			cur +=  v2 *  2.0;
			cur /= 2.0;

			total_len += step;

			if (total_len >= t) {
				return cur;
			}
		}
	}

	return cur; // just return the last point
}

void catmull::compute(std::vector<v2f>* dst) {
	v2f cur{0};
	f64 step = curve_step;

	for (size_t i = 0; i < num_points - 1; i++) {
		for (f64 j = 0; j < 1.0 + step; j += step) {
			v2f v1 = (i >= 1 ? points[i-1] : points[i]);
			v2f v2 = points[i];
			v2f v3 = i + 1 < num_points
				? points[i + 1]
				: (v2 * 2.0 - v1);
			v2f v4 = i + 2 < num_points
				? points[i + 2]
				: (v3 * 2.0 - v2);

			cur = v2f{0};
			cur += (v1 * -1.0 + v2 * 3.0 - v3 * 3.0 + v4) * j * j * j;
			cur += (v1 *  2.0 - v2 * 5.0 + v3 * 4.0 - v4) * j * j;
			cur += (v1 * -1.0 + v3                      ) * j;
			cur +=  v2 *  2.0;
			cur /= 2.0;
			
			dst->push_back(cur);
		}
	}
}
