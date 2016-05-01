#pragma once
#include "common.h"

void die_impl(const char* msg);

// sets the last error to msg if it's not already set
#define die(msg) dbgputs(msg); die_impl(msg)

// returns the last error, or nullptr if no error has occurred
const char* err();