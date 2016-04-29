#pragma once
#include "types.h"

struct v2f {
	f64 x = 0, y = 0;
	char buf[42] = {0};

	v2f() {}
	v2f(f64 x, f64 y) :
		x{x}, 
		y{y} {}

	v2f(f64 v) :
		v2f{v, v} {}

	const char* str();
	f64 len() const;
	v2f& norm();

#define do_op(o) \
	inline void operator o##= (const v2f& v) { x o##= v.x; y o##= v.y; } 	   \
	inline void operator o##= (f64 f)		 { x o##= f; y o##= f; }		   \
	v2f operator o (const v2f& v) const		 { return v2f{x o v.x, y o v.y}; } \
	v2f operator o (f64 f) const		 	 { return v2f{x o f, y o f}; }

	// bisqwit is god
	do_op(+)
	do_op(-)
	do_op(*)
	do_op(/)

#undef do_op
};
