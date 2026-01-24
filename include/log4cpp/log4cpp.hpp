#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>

#ifndef _WIN32
#include <atomic>
#include <csignal> // for SIGHUP
#include <vector>
#endif

#ifdef _WIN32

#if defined(ERROR)
#undef ERROR
#endif

#endif

namespace log4cpp {
    /**
     * @brief Defines the maximum length of a single log message.
     */
    constexpr unsigned short LOG_LINE_MAX = 512;
    /**
     * @brief The default log pattern to use if no configuration file is provided.
     */
    constexpr const char *DEFAULT_LOG_PATTERN = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss} [${8TN}] [${L}] -- ${msg}";
    constexpr const char *FALLBACK_LOGGER_NAME = "root";

    /**
     * @enum log_level
     * @brief Defines the severity levels for log messages.
     */
    enum class log_level { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };

    /**
     * @brief Converts a log_level enum to its string representation.
     * @param level The log level to convert.
     * @param[out] str The output parameter to store the resulting string.
     */
    void to_string(log_level level, std::string &str);

    /**
     * @brief Converts a string to the corresponding log_level enum.
     * @param str The string to convert (case-insensitive, e.g., "INFO", "warn").
     * @param[out] level The output parameter to store the resulting enum value.
     */
    void from_string(const std::string &str, log_level &level);

    /**
     * @brief Gets the name and ID of the current thread.
     * @param[out] thread_name A buffer to store the thread name.
     * @param len The length of the buffer.
     * @return The thread ID.
     */
    unsigned long get_thread_name_id(char *thread_name, size_t len);

    /**
     * @brief Sets the name of the current thread.
     * @param name The name to set for the thread.
     */
    void set_thread_name(const char *name);

    // Forward declarations to avoid including full definitions in the header, reducing compile dependencies.
    namespace config {
        class logger;
        class log4cpp;
    } // namespace config

    namespace appender {
        class log_appender;
    }

    /**
     * @class logger
     * @brief The abstract base class (interface) for a logger.
     *
     * Defines the core functionality that all concrete logger implementations must provide.
     */
    class logger {
    public:
        virtual ~logger() = default;

        [[nodiscard]] virtual std::string get_name() const = 0;
        virtual void set_name(const std::string &name) = 0;

        [[nodiscard]] virtual log_level get_level() const = 0;
        virtual void set_level(log_level level) = 0;

        /**
         * @brief Logs a formatted message with a specific log level.
         * @param _level The log level.
         * @param fmt The C-style format string.
         * @param args The list of arguments matching the format string.
         */
        virtual void log(log_level _level, const char *__restrict fmt, va_list args) const = 0;

        /**
         * @brief Logs a message at the FATAL level.
         * @param fmt The C-style format string.
         * @param ... Variable arguments.
         */
        virtual void fatal(const char *__restrict fmt, ...) const = 0;

        /**
         * @brief Logs a message at the ERROR level.
         * @param fmt The C-style format string.
         * @param ... Variable arguments.
         */
        virtual void error(const char *__restrict fmt, ...) const = 0;

        /**
         * @brief Logs a message at the WARN level.
         * @param fmt The C-style format string.
         * @param ... Variable arguments.
         */
        virtual void warn(const char *__restrict fmt, ...) const = 0;

        /**
         * @brief Logs a message at the INFO level.
         * @param fmt The C-style format string.
         * @param ... Variable arguments.
         */
        virtual void info(const char *__restrict fmt, ...) const = 0;

        /**
         * @brief Logs a message at the DEBUG level.
         * @param fmt The C-style format string.
         * @param ... Variable arguments.
         */
        virtual void debug(const char *__restrict fmt, ...) const = 0;

        /**
         * @brief Logs a message at the TRACE level.
         * @param fmt The C-style format string.
         * @param ... Variable arguments.
         */
        virtual void trace(const char *__restrict fmt, ...) const = 0;
    };

    class logger_manager;
    /**
     * @class supervisor
     * @brief Provides a high-level, static API as the main entry point for interacting with the logging system.
     *
     * It simplifies access to the underlying logger_manager and provides advanced features like hot-reloading.
     */
    class supervisor {
    public:
#ifndef _WIN32
        /**
         * @brief (Non-Windows only) The signal handler for hot-reloading.
         * @param sig_num The signal number received.
         */
        static void sigusr2_handle(int sig_num);
        /**
         * @brief (Non-Windows only) Enables configuration hot-reloading for a specified signal (defaults to SIGHUP).
         * @param sig The signal to use for triggering the hot-reload.
         * @return True if the signal handler was set and the event loop thread was started successfully.
         */
        static bool enable_config_hot_loading(int sig = SIGHUP);
#endif
        /**
         * @brief Gets the singleton instance of the logger_manager.
         * @return A reference to the logger_manager singleton.
         */
        static logger_manager &get_logger_manager();
        /**
         * @brief Serializes the given configuration object into a JSON string.
         * @param cfg The configuration object to serialize.
         * @return A JSON string representing the configuration.
         */
        static std::string serialize(const config::log4cpp &cfg);
    };

    class logger;
    class logger_proxy;
    class logger_deleter;

    /**
     * @class logger_manager
     * @brief The core manager of the logging system, implemented as a singleton.
     *
     * It is responsible for loading and managing configurations, the lifecycle of Appenders,
     * the creation and distribution of Loggers, and the implementation of configuration hot-reloading.
     * This class is thread-safe.
     */
    class logger_manager final {
    public:
        /**
         * @brief Loads the logging configuration from a specified JSON file.
         *
         * This operation overwrites the existing configuration and may trigger the
         * reconstruction of Appenders and Loggers.
         * @param file_path The path to the configuration file.
         * @throw std::runtime_error if the file cannot be opened.
         * @throw std::filesystem::filesystem_error if the file does not exist.
         * @throw nlohmann::json::parse_error if the JSON is invalid.
         */
        void load_config(const std::string &file_path);

        /**
         * @brief Gets a logger by name. This is the primary static method for users to obtain a logger.
         *
         * If a logger with the specified name does not yet exist, a new one will be
         * created based on the configuration. If the name is not found in the
         * configuration, the "root" logger's configuration will be used.
         * @param name The name of the logger. Defaults to "root".
         * @return A shared pointer to the logger interface.
         */
        static std::shared_ptr<logger> get_logger(const std::string &name = FALLBACK_LOGGER_NAME);

        const config::log4cpp *get_config() const;

        logger_manager(const logger_manager &) = delete;

        logger_manager &operator=(const logger_manager &) = delete;

        logger_manager(logger_manager &&) = delete;

        logger_manager &operator=(logger_manager &&) = delete;
        friend class supervisor;
        friend class logger_deleter;

    private:
        logger_manager();

        ~logger_manager();
#ifndef _WIN32
        // @brief (Non-Windows only) Sends a configuration hot-reload notification to the event loop thread.
        void notify_config_hot_reload() const;
        // @brief (Non-Windows only) Executes the configuration hot-reloading logic.
        void hot_reload_config();
        // @brief (Non-Windows only) Creates and starts the event loop thread to listen for hot-reload signals.
        void start_hot_reload_thread();
        // @brief (Non-Windows only) The event loop that waits for and handles events from the signal handler.
        void event_loop();
        // @brief (Non-Windows only) After a hot-reload, updates all active loggers based on the diff between old and
        // new configs.
        void update_logger(const std::unordered_map<std::string, config::logger> &old_log_cfg, bool appender_chg);
#endif
        // @brief Automatically loads the config file from the default path, or uses built-in defaults on failure.
        void auto_load_config();

        // @brief Sets the global log pattern based on the current configuration.
        void set_log_pattern() const;

        // @brief Builds (or rebuilds) all required Appender instances based on the current config. Uses lazy
        // initialization.
        void build_appender();

        // @brief Builds a concrete logger instance based on the given logger configuration.
        std::shared_ptr<logger> build_logger(const config::logger &log_cfg) const;

        // @brief Gets or creates a logger if it doesn't exist. Uses a double-checked locking pattern for thread-safety
        // and efficiency.
        std::shared_ptr<logger_proxy> get_or_create_logger(const std::string &name);
#ifndef _WIN32
        // @brief (Non-Windows only) The event file descriptor for inter-thread communication.
        int evt_fd;
        // @brief (Non-Windows only) An atomic flag to control the event loop thread's execution.
        std::atomic<bool> evt_loop_run{false};
        // @brief (Non-Windows only) The event loop thread object.
        std::thread evt_loop_thread;
#endif
        // @brief Removes a logger from the logger map, typically called by the logger_deleter.
        void release_logger(const std::string &name);

        // A flag to ensure thread-safe initialization of the singleton.
        static std::once_flag init_flag;
        // The unique static instance of the logger_manager.
        static logger_manager instance;
        // The path to the current configuration file.
        std::string config_file_path;

        // A read-write lock to protect the configuration object (config).
        mutable std::shared_mutex config_rw_lock;
        // A unique pointer to the current configuration object.
        std::unique_ptr<config::log4cpp> config;

        // A read-write lock to protect the Appender pointers.
        mutable std::shared_mutex appender_rw_lock;

        // A shared pointer to the console appender.
        std::shared_ptr<appender::log_appender> console_appender_ptr;
        // A shared pointer to the file appender.
        std::shared_ptr<appender::log_appender> file_appender_ptr;
        // A shared pointer to the socket appender.
        std::shared_ptr<appender::log_appender> socket_appender_ptr;

        // A read-write lock to protect the logger map.
        mutable std::shared_mutex logger_rw_lock;
        // A map storing all active logger proxies (name -> weak_ptr of logger_proxy).
        std::unordered_map<std::string, std::weak_ptr<logger_proxy>> loggers;
    };
} // namespace log4cpp
