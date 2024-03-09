#include <cstdarg>
#include <cstdio>
#include "log_utils.h"

#if defined(_WIN32)

#include <winsock.h>

#endif

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
	int i = vsnprintf(buf, size, fmt, args);
	if (i > 0) {
		buf[i] = '\0';
		return i;
	}
	else {
		return 0;
	}
}

size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...) {
	va_list args;
	int i;
	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	if (i > 0) {
		buf[i] = '\0';
		return i;
	}
	else {
		return 0;
	}
}
