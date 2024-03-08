#include <cstdarg>
#include <cstdio>
#include "log_utils.h"

#if defined(_WIN32)

#include <winsock.h>

#endif

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
	int i = vsnprintf(buf, size, fmt, args);
	return (static_cast<size_t>(i) >= size) ? (size - 1) : i;
}

size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...) {
	va_list args;
	int i;
			va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
			va_end(args);
	return (static_cast<size_t>(i) >= size) ? (size - 1) : i;
}
