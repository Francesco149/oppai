#include "utils.h"

namespace {
	const char* last_err = nullptr;
}

void die_impl(const char* msg) {
	if (last_err) {
		return;
	}

	last_err = msg;
}

const char* err() {
	return last_err;
}