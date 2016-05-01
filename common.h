#pragma once

#if _WIN32
#define NEEDS_TO_INSTALL_GENTOO 1
#define strtok_r strtok_s
#define __func__ __FUNCTION__
#endif

#if _DEBUG
#define print_caller_info() printf("%s:%d %s: ", __FILE__, __LINE__, __func__)
#define dbgputs(x) print_caller_info(); puts(x)
#define dbgprintf(fmt, ...) print_caller_info(); printf(fmt, ##__VA_ARGS__)
#else
#define dbgputs(x)
#define dbgprintf(fmt, ...)
#endif

#if OPPAI_VERBOSE
#define vbputs puts
#define vbprintf printf
#else
#define vbputs(x)
#define vbprintf(fmt, ...)
#endif