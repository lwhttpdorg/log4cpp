#pragma once

#include <memory> // for std::shared_ptr
#include <set>    // for std::set
#include <string> // for std::string

#include "log4cpp/log4cpp.hpp"     // for logger
#include "log4cpp/logger.hpp"      // for logger_proxy
#include "pattern/log_pattern.hpp" // for log_pattern

namespace log4cpp::appender {
    class log_appender;
}

namespace log4cpp {
    class real_logger: public logger {
    public:
        real_logger();

        explicit real_logger(const std::string &log_name, log_level _level = log_level::WARN);

        real_logger(const std::string &log_name, log_level _level, const std::string &pattern);

        real_logger(const real_logger &other);

        real_logger(real_logger &&other) noexcept;

        real_logger &operator=(const real_logger &other);

        real_logger &operator=(real_logger &&other) noexcept;

        [[nodiscard]] std::string get_name() const override {
            return name_;
        }

        void set_name(const std::string &name) override {
            this->name_ = name;
        }

        [[nodiscard]] log_level get_level() const override {
            return level_;
        }

        void set_level(log_level level) override {
            this->level_ = level;
        }

        void add_appender(const std::shared_ptr<appender::log_appender> &appender);

        void log(log_level _level, const char *__restrict fmt, va_list args) const override;

        void fatal(const char *__restrict fmt, ...) const override __attribute__((format(printf, 2, 3)));

        void error(const char *__restrict fmt, ...) const override __attribute__((format(printf, 2, 3)));

        void warn(const char *__restrict fmt, ...) const override __attribute__((format(printf, 2, 3)));

        void info(const char *__restrict fmt, ...) const override __attribute__((format(printf, 2, 3)));

        void debug(const char *__restrict fmt, ...) const override __attribute__((format(printf, 2, 3)));

        void trace(const char *__restrict fmt, ...) const override __attribute__((format(printf, 2, 3)));

        ~real_logger() override = default;

        friend class logger_builder;

        friend class log4cpp_config;

    private:
        /* The logger name. */
        std::string name_;
        /* The log level. */
        log_level level_;
        mutable std::shared_mutex appenders_mtx;
        /* The log appenders. */
        std::set<std::shared_ptr<appender::log_appender>> appenders;
        /* The log pattern formatter. */
        pattern::log_pattern pattern_;
    };
} // namespace log4cpp
