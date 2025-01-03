#pragma once

#include <boost/json.hpp>
#include "main/log_output.h"

namespace log4cpp {
	class file_output;

	class file_output_config {
	public:
		/**
		 * @brief Get a file_output instance with the given configuration
		 * @param config The configuration of the file_output
		 * @return File output instance
		 */
		static std::shared_ptr<file_output> get_instance(const file_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_output_config const &obj);

		friend file_output_config
		tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json);

	public:
		std::string file_path{};
		bool append{true};
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_output_config const &obj);

	file_output_config tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json);

	class file_output final : public log_output {
	public:
		class builder {
		public:
			/**
			 * @brief Set the file path of the file output
			 * @param file The file path
			 * @return The builder
			 */
			builder &set_file(const std::string &file);

			/**
			 * @brief Set the append mode of the file output
			 * @param append The append mode, true for append, false for truncate
			 * @return The builder
			 */
			builder &set_append(bool append);

			/**
			 * @brief Build the file output instance
			 * @return The file output instance
			 */
			std::shared_ptr<file_output> build();

			/**
			 * @brief Create a new builder
			 * @return The new builder
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			file_output_config config{};
			std::shared_ptr<file_output> instance{nullptr};
		};


		file_output(const file_output &other) = delete;

		file_output(file_output &&other) = delete;

		file_output &operator=(const file_output &other) = delete;

		file_output &operator=(file_output &&other) = delete;

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

		~file_output() override;

	private:
		file_output() = default;

	private:
		/* The fd of the log file */
		int fd{-1};
	};
}
