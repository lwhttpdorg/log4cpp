#pragma once

#include <boost/json.hpp>
#include "main/log_appender.h"

namespace log4cpp {
	class file_appender;

	class file_appender_config {
	public:
		/**
		 * @brief Get a file_appender instance with the given configuration
		 * @param config The configuration of the file_appender
		 * @return File appender instance
		 */
		static std::shared_ptr<file_appender> get_instance(const file_appender_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_appender_config const &obj);

		friend file_appender_config
		tag_invoke(boost::json::value_to_tag<file_appender_config>, boost::json::value const &json);

	public:
		std::string file_path{};
		static log_lock instance_lock;
		static std::shared_ptr<file_appender> instance;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_appender_config const &obj);

	file_appender_config tag_invoke(boost::json::value_to_tag<file_appender_config>, boost::json::value const &json);

	class file_appender final : public log_appender {
	public:
		class builder {
		public:
			/**
			 * @brief Set the file path for file appender
			 * @param file The file path
			 * @return The builder
			 */
			builder &set_file(const std::string &file);

			/**
			 * @brief Build the file appender instance
			 * @return The file appender instance
			 */
			std::shared_ptr<file_appender> build();

			/**
			 * @brief Create a builder
			 * @return The builder
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			file_appender_config config{};
			std::shared_ptr<file_appender> instance{nullptr};
		};


		file_appender(const file_appender &other) = delete;

		file_appender(file_appender &&other) = delete;

		file_appender &operator=(const file_appender &other) = delete;

		file_appender &operator=(file_appender &&other) = delete;

		/**
		 * @brief Write a log message with the given log level
		 * @param level The log level
		 * @param fmt The format string
		 * @param args The arguments
		 */
		void log(log_level level, const char *__restrict fmt, va_list args) override;

		/**
		 * @brief Write a log message with the given log level
		 * @param level The log level
		 * @param fmt The format string
		 * @param ... The variable arguments
		 */
		void log(log_level level, const char *__restrict fmt, ...) override;

		~file_appender() override;

	private:
		file_appender() = default;

	private:
		/* The fd of the log file */
		int fd{-1};
		log_lock lock;
	};
}
