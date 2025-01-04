#pragma once

#include "log4cpp.hpp"
#include "log_lock.h"

#define CONSOLE_APPENDER  "consoleAppender"
#define FILE_APPENDER     "fileAppender"
#define TCP_APPENDER      "tcpAppender"
#define UDP_APPENDER      "udpAppender"

namespace log4cpp {
	class log_appender {
	public:
		/**
		 * @brief Write log to appender
		 * @param level log level
		 * @param fmt format string
		 * @param args arguments
		 */
		virtual void log(log_level level, const char *__restrict fmt, va_list args) = 0;

		/**
		 * @brief Write log to appender
		 * @param level log level
		 * @param fmt format string
		 * @param ... arguments
		 */
		virtual void log(log_level level, const char *__restrict fmt, ...) = 0;

		virtual ~log_appender() = default;
	};
}
