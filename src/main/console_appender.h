#pragma once

#include <boost/json.hpp>

#include "log_lock.h"
#include "log_appender.h"

namespace log4cpp {
	class console_appender final : public log_appender {
	public:
		console_appender(const console_appender &other) = delete;

		console_appender &operator=(const console_appender &other) = delete;

		console_appender(console_appender &&other) = delete;

		console_appender &operator=(console_appender &&other) = delete;

		/**
		 * @brief Write log message with the given log level
		 * @param level: Log level
		 * @param fmt: Format string
		 * @param args: Arguments
		 */
		void log(log_level level, const char *__restrict fmt, va_list args) override;

		/**
		 * @brief Write log message with the given log level
		 * @param level: Log level
		 * @param fmt: Format string
		 * @param ... Arguments
		 */
		void log(log_level level, const char *__restrict fmt, ...) override;

		class builder {
		public:
			/**
			 * Set the out stream
			 * @param out_stream: The out stream, "stdout" or "stderr", default is "stdout"
			 * @return The builder
			 */
			builder &set_out_stream(const std::string &out_stream);

			/**
			 * Build the console appender
			 * @return The console appender
			 */
			std::shared_ptr<console_appender> build();

			/**
			 * Create a builder
			 * @return The builder
			 */
			static builder new_builder();

		private:
			builder() = default;

			std::shared_ptr<console_appender> instance{nullptr};
		};

	private:
		console_appender() = default;

		explicit console_appender(const std::string &out_stream);

		/* The fd of console */
		int file_no = -1;
		log_lock lock;
	};

	class console_appender_config {
	public:
		/**
		 * Get an instance of console appender with the specified configuration
		 * @return Console appender instance
		 */
		static std::shared_ptr<console_appender> get_instance(const console_appender_config &config);

		[[nodiscard]] std::string get_out_stream() const {
			return out_stream;
		}

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json,
								console_appender_config const &obj);

		friend console_appender_config tag_invoke(boost::json::value_to_tag<console_appender_config>,
												boost::json::value const &json);

	private:
		/* The out stream, "stdout" or "stderr" */
		std::string out_stream{};
		static log_lock instance_lock;
		static std::shared_ptr<console_appender> instance;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, console_appender_config const &obj);

	console_appender_config tag_invoke(boost::json::value_to_tag<console_appender_config>,
										boost::json::value const &json);
}
