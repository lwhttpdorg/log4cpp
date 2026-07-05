#pragma once

#include <cstdint> // for uint_least32_t
#if defined(__APPLE__) && defined(__MACH__)
#include <array> // for std::array
#endif
#include <format>          // for std::format, std::format_string
#include <memory>          // for std::shared_ptr, std::unique_ptr
#include <mutex>           // for std::once_flag
#include <shared_mutex>    // for std::shared_mutex
#include <source_location> // for std::source_location
#include <string>          // for std::string
#include <string_view>     // for std::string_view
#include <thread>          // for std::thread
#include <unordered_map>   // for std::unordered_map
#include <utility>         // for std::forward
#ifndef _WIN32
#include <atomic>  // for std::atomic
#include <csignal> // for SIGHUP
#include <vector>  // for std::vector
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
    constexpr unsigned short LOG_LINE_MAX = 1024;
    /**
     * @brief The default log pattern to use if no configuration file is provided.
     */
    constexpr const char *DEFAULT_LOG_PATTERN = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss} [${8TN}] [${L}] -- ${msg}";
    constexpr const char *FALLBACK_LOGGER_NAME = "root";

    /**
     * @enum log_level
     * @brief Defines the severity levels for log messages.
     */
    enum class log_level : uint8_t { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };

    struct log_location {
        std::string_view file;
        std::string_view function;
        uint_least32_t line = 0;
    };

    class logger;
    class located_logger;

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

        virtual void log(log_level _level, std::string_view msg) const = 0;

        [[nodiscard]] located_logger at(const std::source_location &location = std::source_location::current()) const;

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void log(log_level _level, std::format_string<Args...> fmt, Args &&...args) const {
            log(_level, std::format(fmt, std::forward<Args>(args)...));
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void fatal(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::FATAL, fmt, std::forward<Args>(args)...);
        }

        void fatal(std::string_view msg) const {
            log(log_level::FATAL, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void error(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::ERROR, fmt, std::forward<Args>(args)...);
        }

        void error(std::string_view msg) const {
            log(log_level::ERROR, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void warn(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::WARN, fmt, std::forward<Args>(args)...);
        }

        void warn(std::string_view msg) const {
            log(log_level::WARN, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void info(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::INFO, fmt, std::forward<Args>(args)...);
        }

        void info(std::string_view msg) const {
            log(log_level::INFO, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void debug(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::DEBUG, fmt, std::forward<Args>(args)...);
        }

        void debug(std::string_view msg) const {
            log(log_level::DEBUG, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void trace(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::TRACE, fmt, std::forward<Args>(args)...);
        }

        void trace(std::string_view msg) const {
            log(log_level::TRACE, msg);
        }
    };

    class located_logger {
    public:
        located_logger(const logger &target, const std::source_location &location) :
            logger_(target), location_{location.file_name(), location.function_name(), location.line()} {
        }

        located_logger(const logger &target, log_location location) : logger_(target), location_(location) {
        }

        void log(log_level _level, std::string_view msg) const {
            logger_.log(_level, std::format("[{}:{}] {}", location_.file, location_.line, msg));
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void log(log_level _level, std::format_string<Args...> fmt, Args &&...args) const {
            log(_level, std::format(fmt, std::forward<Args>(args)...));
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void fatal(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::FATAL, fmt, std::forward<Args>(args)...);
        }

        void fatal(std::string_view msg) const {
            log(log_level::FATAL, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void error(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::ERROR, fmt, std::forward<Args>(args)...);
        }

        void error(std::string_view msg) const {
            log(log_level::ERROR, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void warn(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::WARN, fmt, std::forward<Args>(args)...);
        }

        void warn(std::string_view msg) const {
            log(log_level::WARN, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void info(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::INFO, fmt, std::forward<Args>(args)...);
        }

        void info(std::string_view msg) const {
            log(log_level::INFO, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void debug(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::DEBUG, fmt, std::forward<Args>(args)...);
        }

        void debug(std::string_view msg) const {
            log(log_level::DEBUG, msg);
        }

        template<class... Args>
            requires(sizeof...(Args) > 0)
        void trace(std::format_string<Args...> fmt, Args &&...args) const {
            log(log_level::TRACE, fmt, std::forward<Args>(args)...);
        }

        void trace(std::string_view msg) const {
            log(log_level::TRACE, msg);
        }

    private:
        const logger &logger_;
        log_location location_;
    };

    inline located_logger logger::at(const std::source_location &location) const {
        return {*this, location};
    }

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
         * @throw log4cpp::json::parse_error if the JSON is invalid.
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

#ifdef __linux__
        // @brief (Non-Windows only) The event file descriptor for inter-thread communication.
        int evt_fd;
#endif
#if defined(__APPLE__) && defined(__MACH__)
        // @brief (Non-Windows only) Pipe descriptors for inter-thread communication.
        std::array<int, 2> evt_fd;
#endif

#ifndef _WIN32
        // @brief (Non-Windows only) An atomic flag to control the event loop thread's execution.
        std::atomic<bool> evt_loop_run{false};
        // @brief (Non-Windows only) The event loop thread object.
        std::thread evt_loop_thread;
#endif
        // @brief Removes a logger from the logger map, typically called by the logger_deleter.
        void release_logger(const std::string &name);

        // A flag to ensure thread-safe initialization of the singleton.
        static std::once_flag init_flag;
        // Returns the unique static instance of the logger_manager.
        static logger_manager &get_instance();
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
