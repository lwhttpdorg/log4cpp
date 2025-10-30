#pragma once

#include <string>
#include <unordered_map>

#include <cstdarg>
#include <list>
#include <memory>

#include <stdexcept>

#if defined(_WIN32)

#if defined(ERROR)
#undef ERROR
#endif

#endif

namespace log4cpp {
	/**
	 * The log level.
	 */
	enum class log_level { FATAL = 0, ERROR = 1, WARN = 2, INFO = 3, DEBUG = 4, TRACE = 5 };

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

	class logger {
	public:
		virtual ~logger() = default;

		[[nodiscard]] virtual std::string get_name() const = 0;

		[[nodiscard]] virtual log_level get_level() const = 0;

		/**
		 * write log message to output.
		 * @param _level: The log level.
		 * @param fmt: The format string.
		 * @param args: The arguments.
		 */
		virtual void log(log_level _level, const char *__restrict fmt, va_list args) const = 0;

		/**
		 * write FATAL log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		virtual void fatal(const char *__restrict fmt, ...) const = 0;

		/**
		 * write ERROR log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		virtual void error(const char *__restrict fmt, ...) const = 0;

		/**
		 * write WARN log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		virtual void warn(const char *__restrict fmt, ...) const = 0;

		/**
		 * write INFO log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		virtual void info(const char *__restrict fmt, ...) const = 0;

		/**
		 * write DEBUG log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		virtual void debug(const char *__restrict fmt, ...) const = 0;

		/**
		 * write TRACE log message to output.
		 * @param fmt: The format string.
		 * @param ... The arguments.
		 */
		virtual void trace(const char *__restrict fmt, ...) const = 0;
	};

	class layout: public logger {
	public:
		layout();

		explicit layout(const std::string &log_name, log_level _level = log_level::WARN);

		layout(const layout &other);

		layout(layout &&other) noexcept;

		layout &operator=(const layout &other);

		layout &operator=(layout &&other) noexcept;

		[[nodiscard]] std::string get_name() const override {
			return name;
		}

		[[nodiscard]] log_level get_level() const override {
			return level;
		}

		void log(log_level _level, const char *__restrict fmt, va_list args) const override;

		void fatal(const char *__restrict fmt, ...) const override;

		void error(const char *__restrict fmt, ...) const override;

		void warn(const char *__restrict fmt, ...) const override;

		void info(const char *__restrict fmt, ...) const override;

		void debug(const char *__restrict fmt, ...) const override;

		void trace(const char *__restrict fmt, ...) const override;

		~layout() override = default;

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

	class layout_proxy: public logger {
	public:
		explicit layout_proxy(std::shared_ptr<layout> target_logger) : real_logger(std::move(target_logger)) {
			if (!real_logger) {
				throw std::invalid_argument("logger_proxy: real_logger (delegated logger) must not be null");
			}
		}

		[[nodiscard]] std::string get_name() const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			return real_logger->get_name();
		}

		[[nodiscard]] log_level get_level() const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			return real_logger->get_level();
		}

		void log(log_level _level, const char *__restrict fmt, va_list args) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			real_logger->log(_level, fmt, args);
		}

		void fatal(const char *__restrict fmt, ...) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			va_list args;
			va_start(args, fmt);
			real_logger->log(log_level::FATAL, fmt, args);
			va_end(args);
		}

		void error(const char *__restrict fmt, ...) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			va_list args;
			va_start(args, fmt);
			real_logger->log(log_level::ERROR, fmt, args);
			va_end(args);
		}

		void warn(const char *__restrict fmt, ...) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			va_list args;
			va_start(args, fmt);
			real_logger->log(log_level::WARN, fmt, args);
			va_end(args);
		}

		void info(const char *__restrict fmt, ...) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			va_list args;
			va_start(args, fmt);
			real_logger->log(log_level::INFO, fmt, args);
			va_end(args);
		}

		void debug(const char *__restrict fmt, ...) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			va_list args;
			va_start(args, fmt);
			real_logger->log(log_level::DEBUG, fmt, args);
			va_end(args);
		}

		void trace(const char *__restrict fmt, ...) const override {
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			va_list args;
			va_start(args, fmt);
			real_logger->log(log_level::TRACE, fmt, args);
			va_end(args);
		}

		~layout_proxy() override = default;

	private:
		std::shared_ptr<logger> real_logger;
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
		 * @return If the logger exists, return the logger, otherwise return root_layout.
		 */
		static std::shared_ptr<logger> get_layout(const std::string &name);

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
		static std::unordered_map<std::string, std::shared_ptr<logger>> layouts;
		static std::shared_ptr<logger> root_layout;
	};
}
