#include "v2f.h"

#include <stdio.h>
#include <math.h>

const char* v2f::str() const {
	static f32 old_x, old_y;
	static char buf[42] = {0};

	if (x != old_x || y != old_y || !buf[0]) {
		sprintf(buf, "(%g %g)", x, y);
	}

	return buf;
}

f32 v2f::len() const {
	return sqrt(x * x + y * y);
}

f32 v2f::dot(const v2f& other) const {
	return x * other.x + y * other.y;
}

v2f& v2f::norm() {
	auto l = len();
	x /= l;
	y /= l;
	return *this;
}

v2f v2f::rot(f32 rad) const {
	double cos_ = cos(rad);
	double sin_ = sin(rad);
	/* 
	 * 2D rotation matrix:
	 * 
	 * |x|   |cos(r) -sin(r) 0|   |x * cos(r) - y * sin(r) + w * 0|
	 * |y| * |sin(r)  cos(r) 0| = |x * sin(r) + y * cos(r) + w * 0|
	 * |w|   |     0       0 1|   |x * 0      + y * 0      + w * 1|
	 *
	 * this can also be represented as multiplying two complex numbers:
	 * y axis becomes the complex element (i) in a complex number and 
	 * then we can multiply the vector by a complex number that represents the 
	 * angle we want to rotate by.
	 *
	 *  base vec    angle (cos(r), sin(r))
	 * (y1i + x1) (y2i + x2) =
	 * -y1y2 + x2y1i + x1y2i + x1x2 =
	 * (x1x2 - y1y2) + (x1y2 + y1x2)i
	 */
	return v2f{(f32)(x * cos_ - y * sin_), (f32)(x * cos_ + y * sin_)};
}
