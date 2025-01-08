#pragma once

#include <string>
#include <unordered_map>

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
		FATAL = 0,
		ERROR = 1,
		WARN = 2,
		INFO = 3,
		DEBUG = 4,
		TRACE = 5
	};

	/**
	 * Convert log level to string.
	 * @param level: The log level.
	 * @return The string of log level.
	 */
	std::string to_string(log_level level);

	/**
	 * Convert string to log level.
	 * @param s: The string of log level.
	 * @return The log level.
	 */
	log_level from_string(const std::string &s);

	unsigned long get_thread_name_id(char *thread_name, size_t len);

	void set_thread_name(const char *name);

	class log_appender;

	class layout final {
	public:
		layout();

		explicit layout(const std::string &log_name, log_level _level = log_level::WARN);

		layout(const layout &other);

		layout(layout &&other) noexcept;

		layout &operator=(const layout &other);

		layout &operator=(layout &&other) noexcept;

		[[nodiscard]] std::string get_name() const {
			return name;
		}

		[[nodiscard]] log_level get_level() const {
			return level;
		}

		/**
		 * write log message to output.
		 * @param _level: The log level.
		 * @param fmt: The format string.
		 * @param args: The arguments.
		 */
		void log(log_level _level, const char *__restrict fmt, va_list args) const;

		/**
		 * write FATAL log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		void fatal(const char *__restrict fmt, ...) const;

		/**
		 * write ERROR log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		void error(const char *__restrict fmt, ...) const;

		/**
		 * write WARN log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		void warn(const char *__restrict fmt, ...) const;

		/**
		 * write INFO log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		void info(const char *__restrict fmt, ...) const;

		/**
		 * write DEBUG log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		void debug(const char *__restrict fmt, ...) const;

		/**
		 * write TRACE log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		void trace(const char *__restrict fmt, ...) const;

		~layout() = default;

		friend class layout_builder;

		friend class log4cpp_config;

	private:
		/* The logger name. */
		std::string name;
		/* The log level. */
		log_level level;
		/* The log appenders. */
		std::list<std::shared_ptr<log_appender>> appenders;
	};

	/*********************** layout_manager ***********************/
	class log4cpp_config;

	class log_lock;

	class layout_manager final {
	public:
		/**
		 * Load log4cpp configuration from json file.
		 * @param json_filepath: json file path
		 */
		static void load_config(const std::string &json_filepath);

		static const log4cpp_config *get_config();

		/**
		 * Get logger by name.
		 * @param name: The logger name.
		 * @return If the logger exists, return the logger, otherwise return rootLayout.
		 */
		static std::shared_ptr<layout> get_layout(const std::string &name);

	private:
		layout_manager() = default;

		~layout_manager() = default;

		static void build_appender();

		static void build_layout();

		static void build_root_layout();

		static log_lock lock;
		static bool initialized;
		static log4cpp_config config;
		static std::shared_ptr<log_appender> console_appender;
		static std::shared_ptr<log_appender> file_appender;
		static std::shared_ptr<log_appender> tcp_appender;
		static std::shared_ptr<log_appender> udp_appender;
		static std::unordered_map<std::string, std::shared_ptr<layout>> layouts;
		static std::shared_ptr<layout> root_layout;
	};
}
