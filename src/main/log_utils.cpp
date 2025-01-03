#include <cstdarg>
#include <cstdio>
#include <cstring>
#include "log_utils.h"

#if defined(_WIN32)

#include <winsock.h>

#endif

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
	int i = vsnprintf(buf, size, fmt, args);
	if (i > 0) {
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
		return i;
	}
	else {
		return 0;
	}
}

int replace(char *target, size_t length, const char *str_old, const char *str_new) {
	size_t old_len = strlen(str_old);
	size_t new_len = strlen(str_new);
	size_t cur_len = strlen(target);
	if (old_len == 0 || cur_len == 0) {
		return -1;
	}
	// if the existing string length + the increase length is greater than the target length, truncate the string
	size_t truncated_len = 0;
	if (cur_len + new_len - old_len > length) {
		truncated_len = cur_len + new_len - old_len - length;
	}
	char *pos = strstr(target, str_old);
	if (pos == nullptr) {
		return 0;
	}

	// If the new string is longer than the old string, move the characters after the old string
	if (pos + new_len > target + length - 1) {
		new_len = target + length - pos - 1;
		memcpy(pos, str_new, new_len);
		target[new_len] = '\0';
	}
	else {
		if (new_len != old_len) {
			memmove(pos + new_len, pos + old_len, cur_len - (pos - target) - old_len - truncated_len);
			target[cur_len - old_len + new_len] = '\0';
		}
		memcpy(pos, str_new, new_len);
	}

	return 0;
}

std::string replace(const std::string &target, const std::string &str_old, const std::string &str_new) {
	std::string result = target;
	size_t pos = target.find(str_old);
	if (pos != std::string::npos) {
		result.replace(pos, str_old.size(), str_new);
	}
	return result;
}
