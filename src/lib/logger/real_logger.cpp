#include <cstdarg>

#include "appender/log_appender.hpp"
#include "logger/real_logger.hpp"
#include "pattern/log_pattern.hpp"

namespace log4cpp {
    real_logger::real_logger() {
        this->level_ = log_level::WARN;
    }

    real_logger::real_logger(const std::string &log_name, log_level _level) {
        this->name_ = log_name;
        this->level_ = _level;
    }

    real_logger::real_logger(const std::string &log_name, log_level _level, const std::string &pattern) :
        name_(log_name), level_(_level), pattern_(pattern) {
    }

    void real_logger::add_appender(const std::shared_ptr<appender::log_appender> &appender) {
        std::unique_lock lock(appenders_mtx);
        this->appenders.insert(appender);
    }

    void real_logger::log(log_level _level, const char *fmt, va_list args) const {
        if (this->level_ >= _level) {
            char buffer[LOG_LINE_MAX];
            buffer[0] = '\0';
            const size_t used_len = pattern_.format(buffer, sizeof(buffer), this->name_.c_str(), _level, fmt, args);
            std::shared_lock lock(appenders_mtx);
            for (auto &l: this->appenders) {
                l->log(buffer, used_len);
            }
        }
    }

    void real_logger::trace(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::TRACE) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::TRACE, fmt, args);
            va_end(args);
        }
    }

    void real_logger::info(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::INFO) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::INFO, fmt, args);
            va_end(args);
        }
    }

    void real_logger::debug(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::DEBUG) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::DEBUG, fmt, args);
            va_end(args);
        }
    }

    void real_logger::warn(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::WARN) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::WARN, fmt, args);
            va_end(args);
        }
    }

    void real_logger::error(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::ERROR) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::ERROR, fmt, args);
            va_end(args);
        }
    }

    void real_logger::fatal(const char *__restrict fmt, ...) const {
        if (this->level_ >= log_level::FATAL) {
            va_list args;
            va_start(args, fmt);
            this->log(log_level::FATAL, fmt, args);
            va_end(args);
        }
    }

    real_logger::real_logger(const real_logger &other) :
        name_(other.name_), level_(other.level_), pattern_(other.pattern_) {
        std::shared_lock lock(other.appenders_mtx);
        this->appenders = other.appenders;
    }

    real_logger::real_logger(real_logger &&other) noexcept :
        name_(std::move(other.name_)), level_(other.level_), appenders(std::move(other.appenders)),
        pattern_(std::move(other.pattern_)) {
    }

    real_logger &real_logger::operator=(const real_logger &other) {
        if (this != &other) {
            // Copy-and-Swap
            real_logger temp(other);
            std::scoped_lock lock(appenders_mtx, temp.appenders_mtx);
            std::swap(name_, temp.name_);
            std::swap(level_, temp.level_);
            std::swap(appenders, temp.appenders);
            std::swap(pattern_, temp.pattern_);
        }
        return *this;
    }

    real_logger &real_logger::operator=(real_logger &&other) noexcept {
        if (this != &other) {
            std::unique_lock lock(appenders_mtx);
            this->name_ = std::move(other.name_);
            this->level_ = other.level_;
            this->appenders = std::move(other.appenders);
            this->pattern_ = std::move(other.pattern_);
        }
        return *this;
    }
} // namespace log4cpp
