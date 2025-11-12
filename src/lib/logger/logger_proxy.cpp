#include <cstdarg>
#include <stdexcept>

#include <log4cpp/log4cpp.hpp>
#include <log4cpp/logger.hpp>

namespace log4cpp::log {
    constexpr unsigned int PROXY_HOT_RELOADED = 1;

    logger_proxy::logger_proxy(std::shared_ptr<logger> target_logger) :
        real_logger(std::move(target_logger)), hot_reload_flag(0) {
        if (!real_logger) {
            throw std::invalid_argument("logger_proxy: real_logger (delegated logger) must not be null");
        }
    }

    [[nodiscard]] std::string logger_proxy::get_name() const {
        std::shared_lock lock(mtx);
        if (!real_logger) {
            throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
        }
        return real_logger->get_name();
    }

    [[nodiscard]] log_level logger_proxy::get_level() const {
        std::shared_lock lock(mtx);
        if (!real_logger) {
            throw std::runtime_error("logger_proxy: real_logger (delegated logger) is null");
        }
        return real_logger->get_level();
    }

    void logger_proxy::log(log_level _level, const char *__restrict fmt, va_list args) const {
        std::shared_lock lock(mtx);
        if (real_logger) {
            real_logger->log(_level, fmt, args);
        }
    }

    void logger_proxy::fatal(const char *__restrict fmt, ...) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (real_logger) {
            va_list args;
            va_start(args, fmt);
            real_logger->log(log_level::FATAL, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::error(const char *__restrict fmt, ...) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (real_logger) {
            va_list args;
            va_start(args, fmt);
            real_logger->log(log_level::ERROR, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::warn(const char *__restrict fmt, ...) const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (real_logger) {
            va_list args;
            va_start(args, fmt);
            real_logger->log(log_level::WARN, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::info(const char *__restrict fmt, ...) const {
        std::shared_lock lock(mtx);
        if (real_logger) {
            va_list args;
            va_start(args, fmt);
            real_logger->log(log_level::INFO, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::debug(const char *__restrict fmt, ...) const {
        std::shared_lock lock(mtx);
        if (real_logger) {
            va_list args;
            va_start(args, fmt);
            real_logger->log(log_level::DEBUG, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::trace(const char *__restrict fmt, ...) const {
        std::shared_lock lock(mtx);
        if (real_logger) {
            va_list args;
            va_start(args, fmt);
            real_logger->log(log_level::TRACE, fmt, args);
            va_end(args);
        }
    }

    std::shared_ptr<logger> logger_proxy::get_target() {
        std::shared_lock lock(mtx);
        return real_logger;
    }

    void logger_proxy::set_target(std::shared_ptr<logger> target) {
        std::unique_lock lock(mtx);
        real_logger = std::move(target);
        hot_reload_flag = PROXY_HOT_RELOADED;
    }

    void logger_proxy::set_hot_reload_flag() {
        hot_reload_flag |= PROXY_HOT_RELOADED;
    }

    void logger_proxy::reset_hot_reload_flag() {
        hot_reload_flag &= ~PROXY_HOT_RELOADED;
    }

    bool logger_proxy::hot_reload_flag_is_set() const {
        return (hot_reload_flag & PROXY_HOT_RELOADED) != 0;
    }
}
