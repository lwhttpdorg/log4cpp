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
			 * @param name The name of the logger.
			 * @return builder& The builder itself.
			 */
			builder &set_name(const std::string &name);

			/**
			 * @brief Set the log level of the logger.
			 * @param level The log level of the logger.
			 * @return builder& The builder itself.
			 */
			builder &set_log_level(log_level level);

			/**
			 * @brief Set the console output of the logger.
			 * @param output The console output.
			 * @return builder& The builder itself.
			 */
			builder &set_console_output(const std::shared_ptr<log_output> &output);

			/**
			 * @brief Set the file output of the logger.
			 * @param output The file output.
			 * @return builder& The builder itself.
			 */
			builder &set_file_output(const std::shared_ptr<log_output> &output);

			/**
			 * @brief Set the tcp output of the logger.
			 * @param output The tcp output.
			 * @return builder& The builder itself.
			 */
			builder &set_tcp_output(const std::shared_ptr<log_output> &output);

			/**
			 * @brief Set the udp output of the logger.
			 * @param output The udp output.
			 * @return builder& The builder itself.
			 */
			builder &set_udp_output(const std::shared_ptr<log_output> &output);

			/* Build the logger. */
			std::shared_ptr<logger> build();

			/* Create a logger builder. */
			static builder new_builder();

		private:
			builder() = default;

		private:
			std::shared_ptr<logger> log{nullptr};
		};

		friend class logger_manager;

	private:
		logger_builder() = default;
	};
}
