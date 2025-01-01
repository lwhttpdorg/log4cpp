#pragma once

#include <string>
#include "log4cpp.hpp"

namespace log4cpp {
	constexpr unsigned short LOG_LINE_MAX = 512;

	class log_pattern {
	private:
		// The pattern to format the log message
		static std::string _pattern;
	public:
		static void set_pattern(const std::string &pattern);

		static size_t format(char *__restrict buf, size_t buf_len, log_level level, const char *fmt, va_list args);

		static size_t format(char *__restrict buf, size_t buf_len, log_level level, const char *fmt, ...);

	private:
		/**
		 * Format the log message
		 * @param level The log level
		 * @param msg The log message
		 * @return The length of the formatted message
		 */
		static size_t format_with_pattern(char *buf, size_t len, log4cpp::log_level level, const char *msg);
	};
}
