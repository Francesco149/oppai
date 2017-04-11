#pragma once

#define internal static

#if _WIN32 || _WIN64
#define NEEDS_TO_INSTALL_GENTOO 1
#define strtok_r strtok_s
#define __func__ __FUNCTION__
#endif

#if _DEBUG
#define print_caller_info() printf("%s:%d %s: ", __FILE__, __LINE__, __func__)
#define dbgputs(x) print_caller_info(); puts(x)
#define dbgprintf print_caller_info(); printf
#else
#define dbgputs(x)
internal inline void dbgprintf(char const* fmt, ...) {}
#endif

