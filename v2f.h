#pragma once
#include "types.h"

class v2f {
public:
	f32 x = 0, y = 0;

	v2f() {}
	v2f(f32 x, f32 y) :
		x{x}, 
		y{y} {}

	v2f(f32 v) :
		v2f{v, v} {}

	const char* str();
	f32 len() const;
	v2f& norm/*ies*/();

#define do_op(o) \
	inline void operator o##= (const v2f& v) { x o##= v.x; y o##= v.y; } 	   \
	inline void operator o##= (f32 f)		 { x o##= f; y o##= f; }		   \
	v2f operator o (const v2f& v) const		 { return v2f{x o v.x, y o v.y}; } \
	v2f operator o (f32 f) const		 	 { return v2f{x o f, y o f}; }

	// bisqwit is god
	do_op(+)
	do_op(-)
	do_op(*)
	do_op(/)

#undef do_op

protected:
	// this is used for formatting with str() 
	// without having to pass copies of the string around
	char buf[42] = { 0 };
};
