#include "v2f.h"

#include <stdio.h>
#include <math.h>

const char* v2f::str() {
	static f64 old_x, old_y;

	if (x != old_x || y != old_y || !buf[0]) {
		sprintf(buf, "(%g %g)", x, y);
	}

	return buf;
}

f64 v2f::len() const {
	return sqrt(x * x + y * y);
}

v2f& v2f::norm() {
	auto l = len();
	x /= l;
	y /= l;
	return *this;
}
