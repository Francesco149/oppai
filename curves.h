#pragma once
#include "types.h"
#include "v2f.h"
#include <stdlib.h>
#include <vector>

// ported from osu-web

// gets coords at point t (0-1) on a line
v2f pt_on_line(const v2f& p1, const v2f& p2, f64 t);

// gets coords at point t (0-1) on a circular arc of length len of a circle
// that intersects p1, p2, p3, starting at p1.
v2f pt_on_circular_arc(
		const v2f& p1, const v2f& p2, const v2f& p3, f64 t, f64 len);

// checks if pt is inside a circle at center with radius r
bool pt_in_circle(const v2f& pt, const v2f& center, f64 r);

struct curve {
	static const size_t max_points = 0xFF;
	size_t num_points = 0;
	v2f points[max_points];

	void init(const v2f* pts, size_t npts);
	virtual v2f at(f64 t) const = 0;

	// approximates the length of the curve
	f64 len() const;

	// compute granular positions for the entire curve
	virtual void compute(std::vector<v2f>* dst);
};

struct bezier : curve {
	// gets coords at point t (0-1) inside the curve
	virtual v2f at(f64 t) const;
};

struct catmull : curve {
	virtual v2f at(f64 t) const;
	virtual void compute(std::vector<v2f>* dst);
};
