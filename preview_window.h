#pragma once

#ifdef SLIDERTEST
#include "v2f.h"

void p_init(int& argc, char* argv[]);
void p_cls();
u8* p_get_px(const v2f& pos);
void p_put_px(const v2f& pos, u8 r, u8 g, u8 b);
void p_show();
#endif
