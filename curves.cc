#include "curves.h"

#include <cmath>
#include <string.h>
#if NEEDS_TO_INSTALL_GENTOO
#include <algorithm>
#endif

#if _DEBUG
#include <stdio.h>
#endif

// ported from osu-web

v2f pt_on_line(const v2f& p1, const v2f& p2, f64 t) {
	t = std::min(1.0, std::max(0.0, t));
	f64 len = (p2 - p1).len();
	f64 n = len - len * t;
	return (p1 * n + p2 * len * t) / len;
}

// ---

namespace {
	const f64 curve_step = 0.0005;
}

void curve::init(const v2f* pts, size_t npts) {
	points.clear();
	points.insert(points.end(), pts, pts + npts);
}

void curve::compute(std::vector<v2f>* dst) const {
	f64 step = curve_step / points.size();
	for (f64 i = 0; i < 1.f + step; i += step) {
		dst->push_back(at(i));
	}
}

// ---

namespace {
	// calculates the circumscribed circle for a triangle.
	// p1, p2, p3: the triangle
	// c: pointer to a circle struct that will hold the results
	void circum_circle(const v2f& p1, const v2f& p2, const v2f& p3, circle* c) {
		f64 D = 2.0 * (p1.x * (p2.y - p3.y) + p2.x *
			(p3.y - p1.y) + p3.x * (p1.y - p2.y));

		// D can be zero, so we must prevent a division by zero!
		D += 0.00001;

		f64 Ux = ((p1.x * p1.x + p1.y * p1.y) *
			(p2.y - p3.y) + (p2.x * p2.x + p2.y * p2.y) *
			(p3.y - p1.y) + (p3.x * p3.x + p3.y * p3.y) * (p1.y - p2.y)) / D;

		f64 Uy = ((p1.x * p1.x + p1.y * p1.y) *
			(p3.x - p2.x) + (p2.x * p2.x + p2.y * p2.y) *
			(p1.x - p3.x) + (p3.x * p3.x + p3.y * p3.y) * (p2.x - p1.x)) / D;

		v2f radius{ (f32)(Ux - p1.x), (f32)(Uy - p1.y) };
		c->r = radius.len();
		c->c = v2f{ (f32)Ux, (f32)Uy };
	}

	bool is_counter_clockwise(const v2f& a, const v2f& b, const v2f& c) {
		v2f diff1 = b - a;
		v2f diff2 = c - a;
		return diff1.y * diff2.x > diff1.x * diff2.y;
	}

	// rotates point p around the center c
	v2f rotate(const v2f& c, const v2f& p, f64 radians) {
		f64 _cos = cos(radians);
		f64 _sin = sin(radians);
		v2f d = p - c;
		return v2f{
			(f32)(_cos * d.x - _sin * d.y + c.x),
			(f32)(_sin * d.x + _cos * d.y + c.y)
		};
	}
}

void circular_arc::init(const v2f* pts, f64 length) {
	curve::init(pts, 3);
	len = length;
	circum_circle(points[0], points[1], points[2], &c);
}

v2f circular_arc::at(f64 t) const {
	t = std::min(1.0, std::max(0.0, t));

	f64 radians = (t * len) / c.r;
	if (is_counter_clockwise(points[0], points[1], points[2])) {
		radians *= -1;
	}

	return rotate(c.c, points[0], radians);
}

// ---

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

v2f bezier::at(f64 t) const {
	v2f res{0};

	t = std::min(1.0, std::max(0.0, t));
	size_t n = points.size() - 1;
	for (size_t i = 0; i < points.size(); i++) {
		f64 multiplier = 
			binominial_coefficient(i, n) * 
			pow(1.0 - t, n - i) *
			pow(t, i);

		res += points[i] * (f32)multiplier;
	}

	return res;
}

// ---

v2f catmull::at(f64 t) const {
	t = std::min(1.0, std::max(0.0, t));
	t *= points.size();
	size_t i = (size_t)t;
	return compute_single_point(i, t - i);
}

void catmull::compute(std::vector<v2f>* dst) const {
	for (size_t i = 0; i < points.size() - 1; i++) {
		for (f64 t = 0; t < 1.0 + curve_step; t += curve_step) {
			dst->push_back(compute_single_point(i, t));
		}
	}
}

// computes a single point between i and i + 1
// t: how far into the segment we are (0.0 is i, and 1.0 is i+ 1)
v2f catmull::compute_single_point(size_t i, f64 t) const {
	t = std::min(1.0, std::max(0.0, t));

	v2f v1 = (i >= 1 ? points[i - 1] : points[i]);
	v2f v2 = points[i];
	v2f v3 = i + 1 < points.size()
		? points[i + 1]
		: (v2 * 2.0 - v1);
	v2f v4 = i + 2 < points.size()
		? points[i + 2]
		: (v3 * 2.0 - v2);

	v2f pt{ 0 };
	pt += (v1 * -1.0 + v2 * 3.0 - v3 * 3.0 + v4) * t * t * t;
	pt += (v1 *  2.0 - v2 * 5.0 + v3 * 4.0 - v4) * t * t;
	pt += (v1 * -1.0            + v3           ) * t;
	pt +=              v2 * 2.0;
	pt /= 2.0;

	return pt;
}
