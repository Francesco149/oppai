#pragma once

#include "v2f.h"
#include <vector>

struct slider_data;
struct hit_object;

i64 slider_segment_count(hit_object& ho);
v2f slider_at(hit_object& ho, i64 ms);
