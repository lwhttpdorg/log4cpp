#pragma once

#include <string>
#include <unordered_map>
#include <cassert>

#include <boost/json.hpp>
#include <list>
#include <memory>

#if defined(_WIN32)

#if defined(ERROR)
#undef ERROR
#endif

#endif

namespace log4cpp {
	/**
	 * The log level.
	 */
	enum class log_level {
		FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5
	};

	/**
	 * Convert log level to string.
	 * @param level The log level.
	 * @return The string of log level.
	 */
	std::string to_string(log_level level);

	/**
	 * Convert string to log level.
	 * @param s The string of log level.
	 * @return The log level.
	 */
	log_level from_string(const std::string &s);

	class log_output;

	class logger final {
	public:
		logger();

		explicit logger(const std::string &log_name, log_level _level = log_level::WARN);

		logger(const logger &other);

		logger(logger &&other) noexcept;

		logger &operator=(const logger &other);

		logger &operator=(logger &&other) noexcept;

		/**
		 * write log message to output.
		 * @param _level The log level.
		 * @param fmt The format string.
		 * @param args The arguments.
		 */
		void log(log_level _level, const char *__restrict fmt, va_list args) const;

		/**
		 * write FATAL log message to output.
		 * @param fmt The format string.
		 * @param ... The arguments.
		 */
		void fatal(const char *__restrict fmt, ...) const;

		/**
		 * write ERROR log message to output.
		 * @param fmt The format string.
		 * @param ... The arguments.
		 */
		void error(const char *__restrict fmt, ...) const;

		/**
		 * write WARN log message to output.
		 * @param fmt The format string.
		 * @param ... The arguments.
		 */
		void warn(const char *__restrict fmt, ...) const;

		/**
		 * write INFO log message to output.
		 * @param fmt The format string.
		 * @param ... The arguments.
		 */
		void info(const char *__restrict fmt, ...) const;

		/**
		 * write DEBUG log message to output.
		 * @param fmt The format string.
		 * @param ... The arguments.
		 */
		void debug(const char *__restrict fmt, ...) const;

		/**
		 * write TRACE log message to output.
		 * @param fmt The format string.
		 * @param ... The arguments.
		 */
		void trace(const char *__restrict fmt, ...) const;

		~logger() = default;

		friend class logger_builder;

		friend class log4cpp_config;

	private:
		/* The logger name. */
		std::string name;
		/* The log level. */
		log_level level;
		/* The log outputs. */
		std::list<std::shared_ptr<log_output>> outputs;
	};

	/*********************** logger_manager ***********************/
	class log4cpp_config;

	class log_lock;

	class logger_manager final {
	public:
		/**
		 * Load log4cpp configuration from json file.
		 * @param json_filepath
		 */
		static void load_config(const std::string &json_filepath);

		/**
		 * Get logger by name.
		 * @param name The logger name.
		 * @return If the logger exists, return the logger, otherwise return rootLogger.
		 */
		static std::shared_ptr<logger> get_logger(const std::string &name);

	private:
		logger_manager() = default;

		~logger_manager() = default;

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
