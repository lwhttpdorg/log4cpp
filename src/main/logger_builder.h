#pragma once

#include <string>

#include "log4cpp.hpp"
#include "log4cpp_config.h"

namespace log4cpp {
	class layout_builder {
	public:
		class builder {
		public:
			/**
			 * @brief Set the name of the layout.
			 * @param name: The name of the layout.
			 * @return builder& The builder itself.
			 */
			builder &set_name(const std::string &name);

			/**
			 * @brief Set the log level of the layout.
			 * @param level: The log level of the layout.
			 * @return builder& The builder itself.
			 */
			builder &set_log_level(log_level level);

			/**
			 * @brief Set the console appender of the layout.
			 * @param appender: The console appender.
			 * @return builder& The builder itself.
			 */
			builder &set_console_appender(const std::shared_ptr<log_appender> &appender);

			/**
			 * @brief Set the file appender of the layout.
			 * @param appender: The file appender.
			 * @return builder& The builder itself.
			 */
			builder &set_file_appender(const std::shared_ptr<log_appender> &appender);

			/**
			 * @brief Set the tcp appender of the layout.
			 * @param appender: The tcp appender.
			 * @return builder& The builder itself.
			 */
			builder &set_tcp_appender(const std::shared_ptr<log_appender> &appender);

			/**
			 * @brief Set the udp appender of the layout.
			 * @param appender: The udp appender.
			 * @return builder& The builder itself.
			 */
			builder &set_udp_appender(const std::shared_ptr<log_appender> &appender);

			/* Build the layout. */
			std::shared_ptr<layout> build();

			/* Create a layout builder. */
			static builder new_builder();

		private:
			builder() = default;

		private:
			std::shared_ptr<layout> _layout{nullptr};
		};

		friend class layout_manager;

	private:
		layout_builder() = default;
	};
}
