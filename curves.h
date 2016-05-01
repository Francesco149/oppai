#pragma once
#include "types.h"
#include "v2f.h"
#include <stdlib.h>
#include <vector>

// ported from osu-web

// gets a point on a line.
//
// p1, p2: the two points that define the line
// t: how far into the line we are (0.0 would be p1 and 1.0 would be p2)
v2f pt_on_line(const v2f& p1, const v2f& p2, f64 t);

// generic curve code that can be reused in all curves
class curve {
public:
	// copies pts to the curve's points
	void init(const v2f* pts, size_t npts);

	// calculates a point on the curve.
	// t: how far into the curve we are (0.0-1.0)
	virtual v2f at(f64 t) const = 0;

	// compute points for the entire curve at a fixed internal granularity.
	// (calls .at to do this by default)
	// dst: the vector that will store the points.
	virtual void compute(std::vector<v2f>* dst) const;

protected:
	std::vector<v2f> points;
};

struct circle {
	v2f c; // center
	f64 r; // radius
};

class circular_arc : public curve {
public:
	// copies 3 points from pts to the curve's points and sets length.
	// len is the length of the circular arc, in the same units and scale as the
	// points.
	void init(const v2f* pts, f64 len);
	virtual v2f at(f64 t) const;

protected:
	f64 len;
	circle c;
};

class bezier : public curve {
public:
	virtual v2f at(f64 t) const;
};

class catmull : public curve {
public:
	virtual v2f at(f64 t) const;
	virtual void compute(std::vector<v2f>* dst) const;

protected:
	v2f compute_single_point(size_t i, f64 t) const;
};
