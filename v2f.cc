#include "v2f.h"

#include <stdio.h>
#include <cmath>

const char* v2f::str() {
	sprintf(buf, "(%g %g)", x, y);
	return buf;
}

f32 v2f::len() const {
	return sqrt(x * x + y * y);
}

v2f& v2f::norm() {
	auto l = len();
	x /= l;
	y /= l;
	return *this;
}
