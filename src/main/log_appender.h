#pragma once

#include "log4cpp.hpp"

namespace log4cpp {
	constexpr const char *CONSOLE_APPENDER_NAME = "console_appender";
	constexpr const char *FILE_APPENDER_NAME = "file_appender";
	constexpr const char *TCP_APPENDER_NAME = "tcp_appender";
	constexpr const char *UDP_APPENDER_NAME = "udp_appender";

	constexpr unsigned char CONSOLE_APPENDER_FLAG = 0x01;
	constexpr unsigned char FILE_APPENDER_FLAG = 0x02;
	constexpr unsigned char TCP_APPENDER_FLAG = 0x04;
	constexpr unsigned char UDP_APPENDER_FLAG = 0x08;

	class log_appender {
	public:
		/**
		 * @brief Write log to appender
		 * @param msg: log message
		 * @param msg_len: the length of message
		 */
		virtual void log(const char *msg, size_t msg_len) = 0;

		virtual ~log_appender() = default;
	};
}
