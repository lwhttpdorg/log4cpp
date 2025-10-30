#pragma once

#include <array>
#include <string>
#include "log4cpp.hpp"

namespace log4cpp {
	constexpr unsigned short LOG_LINE_MAX = 512;
	constexpr std::array<const char*, 12> MONTH_ABBR_NAME = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
		"Sep", "Oct", "Nov", "Dec"
	};

	void format_day(char *buf, size_t len, const std::string &pattern, const tm &now_tm);

	void format_time(char *buf, size_t len, const std::string &pattern, const tm &now_tm, unsigned short ms);

	void format_daytime(char *buf, size_t len, const std::string &pattern, const tm &now_tm, unsigned short ms);

	class logger_pattern {
	public:
		static void set_pattern(const std::string &pattern);

		/**
		 * Format the log message
		 * @param buf: The buffer to store the formatted message
		 * @param buf_len: The length of the buffer
		 * @param name: The logger name
		 * @param level: The log level
		 * @param fmt: The format string
		 * @param args: The arguments
		 * @return The length of the formatted message
		 */
		static size_t format(char *__restrict buf, size_t buf_len, const char *name, log_level level, const char *fmt,
							va_list args);

		/**
		 * Format the log message
		 * @param buf: The buffer to store the formatted message
		 * @param buf_len: The length of the buffer
		 * @param name: The logger name
		 * @param level: The log level
		 * @param fmt: The format string
		 * @param ... The arguments
		 * @return The length of the formatted message
		 */
		static size_t format(char *__restrict buf, size_t buf_len, const char *name, log_level level, const char *fmt,
							...);

	private:
		// The pattern to format the log message
		static std::string _pattern;
		/**
		 * Format the log message
		 * @param buf: The buffer to store the formatted message
		 * @param len: The length of the buffer
		 * @param name: The logger name
		 * @param level: The log level
		 * @param msg: The log message
		 * @return The length of the formatted message
		 */
		static size_t format_with_pattern(char *buf, size_t len, const char *name, log_level level, const char *msg);
	};
}
