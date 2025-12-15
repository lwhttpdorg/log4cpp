#pragma once

#include <memory>
#include <string>

#include "log4cpp/log4cpp.hpp"

namespace log4cpp {
    /**
     * @class logger_proxy
     * @brief A proxy that forwards logging calls to a real logger implementation.
     *
     * This class implements the Proxy design pattern. It provides a stable interface
     * to the client, which holds a `shared_ptr` to this proxy. The `logger_manager`
     * can then atomically swap the underlying `real_logger` when the configuration
     * is hot-reloaded, without invalidating the client's logger instance.
     */
    class logger_proxy: public logger {
    public:
        /**
         * @brief Constructs a logger_proxy.
         * @param target_logger The initial concrete logger implementation to which calls will be forwarded.
         */
        explicit logger_proxy(std::shared_ptr<logger> target_logger);

        /**
         * @brief Gets the name of the logger.
         * @return The name of the underlying real logger.
         */
        [[nodiscard]] std::string get_name() const override;
        /**
         * @brief Sets the name of the logger.
         * @param name The new name for the underlying real logger.
         */
        void set_name(const std::string &name) override;

        /**
         * @brief Gets the current log level.
         * @return The log level of the underlying real logger.
         */
        [[nodiscard]] log_level get_level() const override;
        /**
         * @brief Sets the log level.
         * @param level The new log level for the underlying real logger.
         */
        void set_level(log_level level) override;

        /**
         * @brief Forwards a formatted log message to the real logger.
         * @param _level The log level.
         * @param fmt The C-style format string.
         * @param args The list of arguments matching the format string.
         */
        void log(log_level _level, const char *__restrict fmt, va_list args) const override;

        // The following methods are convenience wrappers that forward calls to the real logger.
        void fatal(const char *__restrict fmt, ...) const override;
        void error(const char *__restrict fmt, ...) const override;
        void warn(const char *__restrict fmt, ...) const override;
        void info(const char *__restrict fmt, ...) const override;
        void debug(const char *__restrict fmt, ...) const override;
        void trace(const char *__restrict fmt, ...) const override;

        ~logger_proxy() override = default;

        /**
         * @brief Gets the underlying real logger instance.
         * @return A shared pointer to the concrete logger implementation.
         */
        std::shared_ptr<logger> get_target();

        // Allows logger_manager to call private methods like set_target().
        friend class log4cpp::logger_manager; // DO NOT remove qualifier!

    private:
        /**
         * @brief Atomically swaps the underlying real logger.
         * This is the core mechanism for hot-reloading.
         * @param target The new logger implementation to use.
         */
        void set_target(std::shared_ptr<logger> target);

        /// @brief A flag to indicate that a hot-reload is in progress. (Currently unused).
        void set_hot_reload_flag();
        /// @brief Resets the hot-reload flag. (Currently unused).
        void reset_hot_reload_flag();
        /// @brief Checks if the hot-reload flag is set. (Currently unused).
        bool hot_reload_flag_is_set() const;

        /// @brief A read-write mutex to protect access to `real_logger`.
        /// Logging operations use a shared lock, while `set_target` uses a unique lock.
        mutable std::shared_mutex mtx;
        /// @brief A pointer to the actual logger implementation that does the work.
        std::shared_ptr<logger> real_logger;
        /// @brief A flag to signal the hot-reload state.
        unsigned int hot_reload_flag{0};
    };
}
