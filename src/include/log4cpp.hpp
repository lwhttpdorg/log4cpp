#pragma once

#include <string>
#include <unordered_map>
#include <cassert>

#include <boost/json.hpp>
#include <list>
#include <vector>
#include <memory>

#if defined(_WIN32)

#include <winsock.h>

#if defined(ERROR)
#undef ERROR
#endif

#endif

namespace log4cpp {
	enum class log_level {
		FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
	};

	std::string to_string(log_level level);

	log_level from_string(const std::string &s);

	class log_output;

	class logger {
	public:
		logger();

		explicit logger(const std::string &log_name, log_level _level = log_level::WARN);

		logger(const logger &other);

		logger(logger &&other) noexcept;

		logger &operator=(const logger &other);

		logger &operator=(logger &&other) noexcept;

		void log(log_level _level, const char *__restrict fmt, va_list args);

		void fatal(const char *__restrict fmt, ...);

		void error(const char *__restrict fmt, ...);

		void warn(const char *__restrict fmt, ...);

		void info(const char *__restrict fmt, ...);

		void debug(const char *__restrict fmt, ...);

		void trace(const char *__restrict fmt, ...);

		virtual ~logger() = default;

		friend class logger_builder;

		friend class log4cpp_config;

	private:
		std::string name;
		log_level level;
		std::list<std::shared_ptr<log_output>> outputs;
	};

	/*********************** logger_manager ***********************/
	class log4cpp_config;

	class log_lock;

	class logger_manager {
	public:
		static void load_config(const std::string &json_filepath);

		static std::shared_ptr<logger> get_logger(const std::string &name);


	private:
		logger_manager() = default;

		virtual ~logger_manager() = default;

		static void build_output();

		static void build_logger();

		static void build_root_logger();

	private:
		static bool initialized;
		static log4cpp_config config;
		static std::shared_ptr<log_output> console_out;
		static std::shared_ptr<log_output> file_out;
		static std::shared_ptr<log_output> tcp_out;
		static std::shared_ptr<log_output> udp_out;
		static std::unordered_map<std::string, std::shared_ptr<logger>> loggers;
		static std::shared_ptr<logger> root_logger;
	};
}
