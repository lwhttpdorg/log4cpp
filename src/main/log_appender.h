#pragma once

#include "log4cpp.hpp"

namespace log4cpp {
	constexpr const char *CONSOLE_APPENDER_NAME = "consoleAppender";
	constexpr const char *FILE_APPENDER_NAME = "fileAppender";
	constexpr const char *TCP_APPENDER_NAME = "tcpAppender";
	constexpr const char *UDP_APPENDER_NAME = "udpAppender";

	constexpr unsigned char CONSOLE_APPENDER_FLAG = 0x01;
	constexpr unsigned char FILE_APPENDER_FLAG = 0x02;
	constexpr unsigned char TCP_APPENDER_FLAG = 0x04;
	constexpr unsigned char UDP_APPENDER_FLAG = 0x08;

	class log_appender {
	public:
		/**
		 * @brief Write log to appender
		 * @param level:log level
		 * @param fmt:format string
		 * @param args:arguments
		 */
		virtual void log(log_level level, const char *__restrict fmt, va_list args) = 0;

		/**
		 * @brief Write log to appender
		 * @param level:log level
		 * @param fmt:format string
		 * @param ... arguments
		 */
		virtual void log(log_level level, const char *__restrict fmt, ...) = 0;

		virtual ~log_appender() = default;
	};
}
