#pragma once

#include <string>
#include "log4cpp.hpp"
#include "log4cpp_config.h"

namespace log4cpp {
	class logger_builder {
	public:
		class builder {
			friend class logger_builder;

		public:
			builder &set_name(const std::string &name);

			builder &set_log_level(log_level level);

			builder &set_console_output(const std::shared_ptr<log_output> &output);

			builder &set_file_output(const std::shared_ptr<log_output> &output);

			builder &set_tcp_output(const std::shared_ptr<log_output> &output);

			builder &set_udp_output(const std::shared_ptr<log_output> &output);

			logger *build();

		private:
			builder();

		private:
			logger *log;
		};

		friend class logger_manager;

	public:
		static builder new_builder();

	private:
		logger_builder() = default;
	};
}