#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>

#include "log_utils.h"

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args) {
	int i = vsnprintf(buf, size, fmt, args);
	if (i > 0) {
		return i;
	}
	return 0;
}

size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	if (i > 0) {
		return i;
	}
	return 0;
}

size_t replace(char *original, size_t length, const char *target, const char *replace) {
	size_t target_len = strlen(target);
	size_t replace_len = strlen(replace);
	size_t origin_len = strlen(original);
	if (target_len == 0 || origin_len == length) {
		return origin_len;
	}

	char *pos = strstr(original, target);
	if (nullptr == pos) {
		return origin_len;
	}

	size_t new_len;
	// There is not enough size after the replacement position to accommodate the string to be replaced.
	if (pos + replace_len > original + length - 1) {
		size_t copy_len = original + length - pos - 1;
		memcpy(pos, replace, copy_len);
		original[length - 1] = '\0';
		new_len = length - 1;
	}
	else {
		// The lengths of target and replace are not equal, so the subsequent string needs to be moved
		if (replace_len != target_len) {
			size_t move_len;
			// If the subsequent length is insufficient, the string after target will be truncated
			if (origin_len + replace_len - target_len > length) {
				move_len = origin_len - (pos - original) - replace_len;
			}
			else {
				move_len = origin_len - (pos - original) - target_len;
			}
			memmove(pos + replace_len, pos + target_len, move_len);
			new_len = pos - original + replace_len + move_len;
			original[new_len] = '\0';
		}
		else {
			new_len = origin_len;
		}
		memcpy(pos, replace, replace_len);
	}
	return new_len;
}

std::string replace(const std::string &original, const std::string &target, const std::string &replace) {
	std::string result = original;
	size_t pos = original.find(target);
	if (pos != std::string::npos) {
		result.replace(pos, target.size(), replace);
	}
	return result;
}

void get_time_now(tm &now_tm, unsigned short &ms) {
	const auto now = std::chrono::system_clock::now();
	const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
#ifdef _WIN32
	localtime_s(&now_tm, &now_time_t);
#else
	localtime_r(&now_time_t, &now_tm);
#endif
	ms = static_cast<unsigned short>(now_ms.count());
}
