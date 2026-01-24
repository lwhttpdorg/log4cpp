#include <cstdarg>

#include "appender/log_appender.hpp"
#include "logger/core_logger.hpp"
#include "pattern/log_pattern.hpp"

namespace log4cpp {
    core_logger::core_logger() {
        this->level_ = log_level::WARN;
    }

    core_logger::core_logger(const std::string &log_name, log_level _level) {
        this->name_ = log_name;
        this->level_ = _level;
    }

    void core_logger::add_appender(const std::shared_ptr<appender::log_appender> &appender) {
        std::unique_lock lock(appenders_mtx);
        this->appenders.insert(appender);
    }

    void core_logger::log(log_level _level, const char *fmt, va_list args) const {
        if (this->level_ >= _level) {
            char buffer[LOG_LINE_MAX];
            buffer[0] = '\0';
            const size_t used_len =
                pattern::log_pattern::format(buffer, sizeof(buffer), this->name_.c_str(), _level, fmt, args);
            std::shared_lock lock(appenders_mtx);
            for (auto &l: this->appenders) {
                l->log(buffer, used_len);
            }
        }
    }

    void core_logger::trace(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::TRACE) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::TRACE, fmt, args);
            va_end(args);
        }
    }

    void core_logger::info(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::INFO) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::INFO, fmt, args);
            va_end(args);
        }
    }

    void core_logger::debug(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::DEBUG) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::DEBUG, fmt, args);
            va_end(args);
        }
    }

    void core_logger::warn(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::WARN) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::WARN, fmt, args);
            va_end(args);
        }
    }

    void core_logger::error(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::ERROR) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::ERROR, fmt, args);
            va_end(args);
        }
    }

    void core_logger::fatal(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::FATAL) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::FATAL, fmt, args);
            va_end(args);
        }
    }

    core_logger::core_logger(const core_logger &other) : name_(other.name_), level_(other.level_) {
        std::shared_lock lock(other.appenders_mtx);
        this->appenders = other.appenders;
    }

    core_logger::core_logger(core_logger &&other) noexcept :
        name_(std::move(other.name_)), level_(other.level_), appenders(std::move(other.appenders)) {
    }

    core_logger &core_logger::operator=(const core_logger &other) {
        if (this != &other) {
            // Copy-and-Swap
            core_logger temp(other);
            std::scoped_lock lock(appenders_mtx, temp.appenders_mtx);
            std::swap(name_, temp.name_);
            std::swap(level_, temp.level_);
            std::swap(appenders, temp.appenders);
        }
        return *this;
    }

    core_logger &core_logger::operator=(core_logger &&other) noexcept {
        if (this != &other) {
            std::unique_lock lock(appenders_mtx);
            this->name_ = std::move(other.name_);
            this->level_ = other.level_;
            this->appenders = std::move(other.appenders);
        }
        return *this;
    }
} // namespace log4cpp
