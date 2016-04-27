#pragma once

#include "v2f.h"
#include <vector>

struct slider_data;
struct hit_object;

v2f slider_at(hit_object& ho, i64 ms);
