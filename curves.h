#pragma once
#include "types.h"
#include "v2f.h"
#include <stdlib.h>

// ported from osu-web

// gets coords at point t (0-1) on a line
v2f pt_on_line(const v2f& p1, const v2f& p2, f32 t);

// gets coords at point t (0-1) on the edge of the circle that intersects
// p1, p2, p3
v2f pt_on_circle(const v2f& p1, const v2f& p2, const v2f& p3, f32 t);

// checks if pt is inside a circle at center with radius r
bool pt_in_circle(const v2f& pt, const v2f& center, f32 r);

// TODO: just cache xy in small steps and look up in the cached coords for
// better performance (not really necessary for diff calc but whetever)
	
struct curve {
	static const size_t max_points = 0xFF;
	size_t num_points = 0;
	v2f points[max_points];

	void init(const v2f* pts, size_t npts);
	virtual v2f at(f32 t) const = 0;

	// approximates the length of the curve
	f32 len() const;
};

struct bezier : curve {
	// gets coords at point t (0-1) inside the curve
	v2f at(f32 t) const;
};

struct catmull : curve {
	v2f at(f32 t) const;
};
