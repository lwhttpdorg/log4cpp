#pragma once

#include <string>

#include "log4cpp.hpp"
#include "log4cpp_config.h"

namespace log4cpp {
	class logger_builder {
	public:
		class builder {
		public:
			/**
			 * @brief Set the name of the logger.
			 * @param name: The name of the logger.
			 * @return builder& The builder itself.
			 */
			builder &set_name(const std::string &name);

			/**
			 * @brief Set the log level of the logger.
			 * @param level: The log level of the logger.
			 * @return builder& The builder itself.
			 */
			builder &set_log_level(log_level level);

			/**
			 * @brief Set the console appender of the logger.
			 * @param appender: The console appender.
			 * @return builder& The builder itself.
			 */
			builder &set_console_appender(const std::shared_ptr<log_appender> &appender);

			/**
			 * @brief Set the file appender of the logger.
			 * @param appender: The file appender.
			 * @return builder& The builder itself.
			 */
			builder &set_file_appender(const std::shared_ptr<log_appender> &appender);

			/**
			 * @brief Set the tcp appender of the logger.
			 * @param appender: The tcp appender.
			 * @return builder& The builder itself.
			 */
			builder &set_tcp_appender(const std::shared_ptr<log_appender> &appender);

			/**
			 * @brief Set the udp appender of the logger.
			 * @param appender: The udp appender.
			 * @return builder& The builder itself.
			 */
			builder &set_udp_appender(const std::shared_ptr<log_appender> &appender);

			/* Build the logger. */
			std::shared_ptr<logger> build();

			/* Create a logger builder. */
			static builder new_builder();

		private:
			builder() = default;

		private:
			std::shared_ptr<core_logger> _logger{nullptr};
		};

		friend class logger_manager;

	private:
		logger_builder() = default;
	};
}
