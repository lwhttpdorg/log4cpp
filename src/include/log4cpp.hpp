#pragma once

#include <string>
#include <unordered_map>

#include <cstdarg>
#include <list>
#include <memory>

#include <atomic>
#include <stdexcept>
#include <thread>
#include <utility>
#include <shared_mutex>
#include <mutex>

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
		OFF = 0, FATAL = 1, ERROR = 2, WARN = 3, INFO = 4, DEBUG = 5, TRACE = 6
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

	class core_logger : public logger {
	public:
		core_logger();

		explicit core_logger(const std::string &log_name, log_level _level = log_level::WARN);

		core_logger(const core_logger &other);

		core_logger(core_logger &&other) noexcept;

		core_logger &operator=(const core_logger &other);

		core_logger &operator=(core_logger &&other) noexcept;

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

		~core_logger() override = default;

		friend class logger_builder;

		friend class log4cpp_config;

	private:
		/* The logger name. */
		std::string name;
		/* The log level. */
		log_level level;
		/* The log appenders. */
		std::list<std::shared_ptr<log_appender>> appenders;
	};

	class logger_proxy : public logger {
	public:
		explicit logger_proxy(std::shared_ptr<logger> target_logger) : real_logger(std::move(target_logger)) {
			if (!real_logger) {
				throw std::invalid_argument("logger_proxy: real_logger (delegated logger) must not be null");
			}
		}

		[[nodiscard]] std::string get_name() const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			return real_logger->get_name();
		}

		[[nodiscard]] log_level get_level() const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (!real_logger) {
				throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
			}
			return real_logger->get_level();
		}

		void log(log_level _level, const char *__restrict fmt, va_list args) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				real_logger->log(_level, fmt, args);
			}
		}

		void fatal(const char *__restrict fmt, ...) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				va_list args;
				va_start(args, fmt);
				// Add argument real_logger->get_name()
				real_logger->log(log_level::FATAL, fmt, args);
				va_end(args);
			}
		}

		void error(const char *__restrict fmt, ...) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				va_list args;
				va_start(args, fmt);
				real_logger->log(log_level::ERROR, fmt, args);
				va_end(args);
			}
		}

		void warn(const char *__restrict fmt, ...) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				va_list args;
				va_start(args, fmt);
				real_logger->log(log_level::WARN, fmt, args);
				va_end(args);
			}
		}

		void info(const char *__restrict fmt, ...) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				va_list args;
				va_start(args, fmt);
				real_logger->log(log_level::INFO, fmt, args);
				va_end(args);
			}
		}

		void debug(const char *__restrict fmt, ...) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				va_list args;
				va_start(args, fmt);
				real_logger->log(log_level::DEBUG, fmt, args);
				va_end(args);
			}
		}

		void trace(const char *__restrict fmt, ...) const override {
			std::shared_lock<std::shared_mutex> lock(mtx);
			if (real_logger) {
				va_list args;
				va_start(args, fmt);
				real_logger->log(log_level::TRACE, fmt, args);
				va_end(args);
			}
		}

		~logger_proxy() override = default;

		std::shared_ptr<logger> get_target() {
			std::shared_lock<std::shared_mutex> lock(mtx);
			return real_logger;
		}

		void set_target(std::shared_ptr<logger> target) {
			std::unique_lock<std::shared_mutex> lock(mtx);
			real_logger = std::move(target);
		}

	private:
		mutable std::shared_mutex mtx;
		std::shared_ptr<logger> real_logger;
	};

	/*********************** logger_manager ***********************/
	class log4cpp_config;

	class log_lock;

	class logger_manager final {
	public:
		static logger_manager &instance() {
			static logger_manager instance;
			return instance;
		}

		bool enable_config_hot_loading();

		/**
		 * Load log4cpp configuration from json file.
		 * @param file_path: json file path
		 */
		void load_config(const std::string &file_path);

		static const log4cpp_config *get_config();

		/**
		 * Get logger by name.
		 * @param name: The logger name.
		 * @return If the logger exists, return the logger, otherwise return root_logger.
		 */
		std::shared_ptr<logger> get_logger(const std::string &name);

		logger_manager(const logger_manager &) = delete;

		logger_manager &operator=(const logger_manager &) = delete;

		logger_manager(logger_manager &&) = delete;

		logger_manager &operator=(logger_manager &&) = delete;

	private:
		logger_manager();

		~logger_manager();

		static void handle_sigusr2(int sig_num);

		void notify_config_hot_reload() const;

		void hot_reload_config();

		void event_loop();

		void build_appender();

		void build_logger();

		void build_root_logger();

		static log_lock lock;
		int evt_fd;
		std::atomic<bool> evt_loop_run{false};
		std::thread evt_loop_thread;
		bool initialized;
		std::string config_file_path;
		static log4cpp_config config;
		std::shared_ptr<log_appender> console_appender;
		std::shared_ptr<log_appender> file_appender;
		std::shared_ptr<log_appender> tcp_appender;
		std::shared_ptr<log_appender> udp_appender;
		std::unordered_map<std::string, std::shared_ptr<logger_proxy>> loggers;
		std::shared_ptr<logger_proxy> root_logger;
	};
}
