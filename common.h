#pragma once

#if _WIN32
#define NEEDS_TO_INSTALL_GENTOO 1
#define strtok_r strtok_s
#endif