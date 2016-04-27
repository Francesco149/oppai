#pragma once

#ifdef SLIDERTEST
#include "v2f.h"

struct hit_object;

void p_init(int& argc, char* argv[]);
void p_show(hit_object& ho);
#else
#define p_init(argc, argv)
#define p_show(ho)
#endif
