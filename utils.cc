#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

void die(const char* msg) {
	fputs(msg, stderr);
	fputs("\n", stderr);
	exit(1);
}
