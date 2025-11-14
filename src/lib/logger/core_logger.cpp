#include <cstdarg>

#include "appender/log_appender.hpp"
#include "logger/core_logger.hpp"
#include "pattern/log_pattern.hpp"

namespace log4cpp::log {
    core_logger::core_logger() {
        this->level = log_level::WARN;
    }

    core_logger::core_logger(const std::string &log_name, log_level _level) {
        this->name = log_name;
        this->level = _level;
    }

    void core_logger::add_appender(const std::shared_ptr<appender::log_appender> &appender) {
        std::unique_lock<std::shared_mutex> lock(appenders_mtx);
        this->appenders.insert(appender);
    }

    void core_logger::log(log_level _level, const char *fmt, va_list args) const {
        if (this->level >= _level) {
            char buffer[LOG_LINE_MAX];
            buffer[0] = '\0';
            const size_t used_len =
                pattern::log_pattern::format(buffer, sizeof(buffer), this->name.c_str(), _level, fmt, args);
            std::shared_lock<std::shared_mutex> lock(appenders_mtx);
            for (auto &l: this->appenders) {
                l->log(buffer, used_len);
            }
        }
    }

    void core_logger::trace(const char *__restrict fmt, ...) const {
        if (this->level >= log_level::TRACE) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::TRACE, fmt, args);
            va_end(args);
        }
    }

    void core_logger::info(const char *__restrict fmt, ...) const {
        if (this->level >= log_level::INFO) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::INFO, fmt, args);
            va_end(args);
        }
    }

    void core_logger::debug(const char *__restrict fmt, ...) const {
        if (this->level >= log_level::DEBUG) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::DEBUG, fmt, args);
            va_end(args);
        }
    }

    void core_logger::warn(const char *__restrict fmt, ...) const {
        if (this->level >= log_level::WARN) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::WARN, fmt, args);
            va_end(args);
        }
    }

    void core_logger::error(const char *__restrict fmt, ...) const {
        if (this->level >= log_level::ERROR) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::ERROR, fmt, args);
            va_end(args);
        }
    }

    void core_logger::fatal(const char *__restrict fmt, ...) const {
        if (this->level >= log_level::FATAL) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::FATAL, fmt, args);
            va_end(args);
        }
    }

    core_logger::core_logger(const core_logger &other) {
        this->name = other.name;
        this->level = other.level;
        this->appenders = other.appenders;
    }

    core_logger::core_logger(core_logger &&other) noexcept {
        this->name = std::move(other.name);
        this->level = other.level;
        this->appenders = std::move(other.appenders);
    }

    core_logger &core_logger::operator=(const core_logger &other) {
        if (this != &other) {
            this->name = other.name;
            this->level = other.level;
            this->appenders = other.appenders;
        }
        return *this;
    }

    core_logger &core_logger::operator=(core_logger &&other) noexcept {
        if (this != &other) {
            this->name = std::move(other.name);
            this->level = other.level;
            this->appenders = std::move(other.appenders);
        }
        return *this;
    }
}
