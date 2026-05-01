#include <cstdarg>
#include <stdexcept>

#include <log4cpp/log4cpp.hpp>
#include <log4cpp/logger.hpp>

namespace log4cpp {
    logger_proxy::logger_proxy(std::shared_ptr<logger> target_logger) : target_(std::move(target_logger)) {
        if (!target_) {
            throw std::invalid_argument("logger_proxy: target_ (delegated logger) must not be null");
        }
    }

    [[nodiscard]] std::string logger_proxy::get_name() const {
        std::shared_lock lock(mtx);
        if (!target_) {
            return {};
        }
        return target_->get_name();
    }

    void logger_proxy::set_name(const std::string &name) {
        std::unique_lock lock(mtx);
        if (!target_) {
            return;
        }
        target_->set_name(name);
    }

    [[nodiscard]] log_level logger_proxy::get_level() const {
        std::shared_lock lock(mtx);
        if (!target_) {
            return {};
        }
        return target_->get_level();
    }

    void logger_proxy::set_level(log_level level) {
        std::unique_lock lock(mtx);
        if (!target_) {
            return;
        }
        target_->set_level(level);
    }

    void logger_proxy::log(log_level _level, const char *__restrict fmt, va_list args) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            logger_ptr->log(_level, fmt, args);
        }
    }

    void logger_proxy::fatal(const char *__restrict fmt, ...) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            va_list args;
            va_start(args, fmt);
            logger_ptr->log(log_level::FATAL, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::error(const char *__restrict fmt, ...) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            va_list args;
            va_start(args, fmt);
            logger_ptr->log(log_level::ERROR, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::warn(const char *__restrict fmt, ...) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            va_list args;
            va_start(args, fmt);
            logger_ptr->log(log_level::WARN, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::info(const char *__restrict fmt, ...) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            va_list args;
            va_start(args, fmt);
            logger_ptr->log(log_level::INFO, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::debug(const char *__restrict fmt, ...) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            va_list args;
            va_start(args, fmt);
            logger_ptr->log(log_level::DEBUG, fmt, args);
            va_end(args);
        }
    }

    void logger_proxy::trace(const char *__restrict fmt, ...) const {
        std::shared_ptr<logger> logger_ptr;
        {
            std::shared_lock lock(mtx);
            logger_ptr = target_;
        }
        if (logger_ptr) {
            va_list args;
            va_start(args, fmt);
            logger_ptr->log(log_level::TRACE, fmt, args);
            va_end(args);
        }
    }

    std::shared_ptr<logger> logger_proxy::get_target() {
        std::shared_lock lock(mtx);
        return target_;
    }

    void logger_proxy::set_target(std::shared_ptr<logger> target) {
        std::unique_lock lock(mtx);
        target_ = std::move(target);
    }
} // namespace log4cpp
