#pragma once

#include <boost/json.hpp>

#include "main/log_output.h"

namespace log4cpp {
	class console_output final : public log_output {
	public:
		console_output(const console_output &other) = delete;

		console_output &operator=(const console_output &other) = delete;

		console_output(console_output &&other) = delete;

		console_output &operator=(console_output &&other) = delete;

		/**
		 * @brief Write log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param args Arguments
		 */
		void log(log_level level, const char *__restrict fmt, va_list args) override;

		/**
		 * @brief Write log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param ... Arguments
		 */
		void log(log_level level, const char *__restrict fmt, ...) override;

	public:
		class builder {
		public:
			/**
			 * Set the output stream
			 * @param out_stream The output stream, "stdout" or "stderr", default is "stdout"
			 * @return The builder
			 */
			builder &set_out_stream(const std::string &out_stream);

			/**
			 * Build the console output
			 * @return The console output
			 */
			std::shared_ptr<console_output> build();

			/**
			 * Create a new console output builder
			 * @return The console output builder
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			std::shared_ptr<console_output> instance{nullptr};
		};

	private:
		console_output() = default;

		explicit console_output(const std::string &out_stream);

	private:
		/* The fd of console */
		int file_no = -1;
	};

	class console_output_config {
	public:
		/**
		 * Get an instance of console output with the specified configuration
		 * @return Console output instance
		 */
		static std::shared_ptr<console_output> get_instance(const console_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, console_output_config const &obj);

		friend console_output_config
		tag_invoke(boost::json::value_to_tag<console_output_config>, boost::json::value const &json);

	private:
		/* The output stream, "stdout" or "stderr" */
		std::string out_stream{};
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, console_output_config const &obj);

	console_output_config tag_invoke(boost::json::value_to_tag<console_output_config>, boost::json::value const &json);
}
