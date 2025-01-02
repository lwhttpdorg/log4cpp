#pragma once

#include "main/log_output.h"

namespace log4cpp {
	class file_output;

	class file_output_config {
	public:
		static std::shared_ptr<file_output> get_instance(const file_output_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_output_config const &obj);

		friend file_output_config
		tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json);

	public:
		std::string file_path;
		bool append;
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, file_output_config const &obj);

	file_output_config tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json);

	class file_output : public log_output {
	public:
		class builder {
		public:
			builder &set_file(const std::string &file);

			builder &set_append(bool append);

			std::shared_ptr<file_output> build();

			static builder new_builder();

		private:
			file_output_config config;
			std::shared_ptr<file_output> instance{nullptr};
		};


		file_output(const file_output &other) = delete;

		file_output(file_output &&other) = delete;

		file_output &operator=(const file_output &other) = delete;

		file_output &operator=(file_output &&other) = delete;

		void log(log_level level, const char *__restrict fmt, va_list args) override;

		void log(log_level level, const char *__restrict fmt, ...) override;

		~file_output() override;

	private:
		file_output() = default;

	private:
		int fd{-1};
	};
}