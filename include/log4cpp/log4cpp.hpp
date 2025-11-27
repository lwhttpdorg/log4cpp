#pragma once

#include <string>
#include <unordered_map>

#include <memory>

#include <atomic>
#include <csignal>
#include <mutex>
#include <shared_mutex>
#include <thread>

#if defined(_WIN32)

#if defined(ERROR)
#undef ERROR
#endif

#endif

namespace log4cpp {
    constexpr unsigned short LOG_LINE_MAX = 512;
    constexpr const char *DEFAULT_LOG_PATTERN = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss} [${8TN}] [${L}] -- ${W}";
    /**
     * The log level.
     */
    enum class log_level { FATAL, ERROR, WARN, INFO, DEBUG, TRACE };

    /**
     * Convert log level to string.
     * @param level: The log level.
     * @return The string of log level.
     */
    std::string level_to_string(log_level level);

    /**
     * Convert string to log level.
     * @param s: The string of log level.
     * @return The log level.
     */
    log_level level_from_string(const std::string &s);

    unsigned long get_thread_name_id(char *thread_name, size_t len);

    void set_thread_name(const char *name);

    namespace config {
        class log4cpp;
    }

    namespace appender {
        class log_appender;
    }

    namespace log {
        class logger {
        public:
            virtual ~logger() = default;

            [[nodiscard]] virtual std::string get_name() const = 0;
            virtual void set_name(const std::string &name) = 0;

            [[nodiscard]] virtual log_level get_level() const = 0;
            virtual void set_level(log_level level) = 0;

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
    }

    class logger_manager;

    class supervisor {
    public:
        static void sigusr2_handle(int sig_num);
        static bool enable_config_hot_loading(int sig = SIGHUP);
        static logger_manager &get_logger_manager();
        static std::string serialize(const config::log4cpp &cfg);
    };

    namespace log {
        class logger;
        class logger_proxy;
    }

    /*********************** logger_manager ***********************/

    class logger_manager final {
    public:
        /**
         * Load log4cpp configuration from json file.
         * @param file_path: json file path
         */
        void load_config(const std::string &file_path);

        /**
         * Get logger by name.
         * @param name: The logger name.
         * @return If the logger exists, return the logger, otherwise return root_logger.
         */
        static std::shared_ptr<log::logger> get_logger(const std::string &name);

        const config::log4cpp *get_config() const;

        logger_manager(const logger_manager &) = delete;

        logger_manager &operator=(const logger_manager &) = delete;

        logger_manager(logger_manager &&) = delete;

        logger_manager &operator=(logger_manager &&) = delete;
        friend class supervisor;

    private:
        logger_manager();

        ~logger_manager();

        void notify_config_hot_reload() const;
        void hot_reload_config();
        void start_hot_reload_thread();

        void event_loop();

        void auto_load_config();

        void set_log_pattern() const;

        void build_appender();

        void build_logger();

        std::shared_ptr<log::logger> find_logger(const std::string &name);

        mutable std::shared_mutex rw_lock;
        int evt_fd;
        std::atomic<bool> evt_loop_run{false};
        std::thread evt_loop_thread;
        static std::once_flag init_flag;
        static logger_manager instance;
        std::string config_file_path;
        std::unique_ptr<config::log4cpp> config;
        mutable std::shared_mutex appender_mtx;
        std::shared_ptr<appender::log_appender> console_appender_ptr;
        std::shared_ptr<appender::log_appender> file_appender_ptr;
        std::shared_ptr<appender::log_appender> socket_appender_ptr;

        mutable std::shared_mutex logger_map_mtx;
        std::unordered_map<std::string, std::shared_ptr<log::logger_proxy>> loggers;
        std::shared_ptr<log::logger_proxy> root_logger;
    };
}
