#include "v2f.h"

#include <stdio.h>
#include <cmath>

const char* v2f::str() {
	sprintf(buf, "(%g %g)", x, y);
	return buf;
}

f64 v2f::len() const {
	return sqrt((f64)x * (f64)x + (f64)y * (f64)y);
}

v2f& v2f::norm() {
	auto l = len();
	x /= l;
	y /= l;
	return *this;
}
