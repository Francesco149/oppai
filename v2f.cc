#include "v2f.h"

#include <stdio.h>
#include <cmath>

const char* v2f::str() {
	static const f64 epsilon = 0.00001;
	if (std::abs(x - old_x) > epsilon || std::abs(y - old_y) > epsilon || !buf[0]) {
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
