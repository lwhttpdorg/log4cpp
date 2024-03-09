#pragma once

#include "main/log_output.h"

namespace log4cpp {
	class console_output : public log_output {
	public:
		console_output(const console_output &other) = delete;

		console_output &operator=(const console_output &other) = delete;

		console_output(console_output &&other) = delete;

		console_output &operator=(console_output &&other) = delete;

		void log(log_level level, const char *__restrict fmt, va_list args) override;

		void log(log_level level, const char *__restrict fmt, ...) override;

	public:
		class builder {
		public:
			builder &set_out_stream(const std::string &out_stream);

			std::shared_ptr<console_output> build();

			static builder new_builder();

		private:
			std::shared_ptr<console_output> instance{nullptr};
		};

	private:
		console_output() = default;

		explicit console_output(const std::string &out_stream);

	private:
		int file_no = -1;
	};

	class console_output_config {
	public:
		static std::shared_ptr<console_output> get_instance(const console_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, console_output_config const &obj);

		friend console_output_config
		tag_invoke(boost::json::value_to_tag<console_output_config>, boost::json::value const &json);

	private:
		std::string out_stream;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, console_output_config const &obj);

	console_output_config tag_invoke(boost::json::value_to_tag<console_output_config>, boost::json::value const &json);
}
