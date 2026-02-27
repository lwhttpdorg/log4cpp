#pragma once

#include <set>
#include <string>

#include <log4cpp/log4cpp.hpp>
#include <log4cpp/logger.hpp>

namespace log4cpp::appender {
    class log_appender;
}

namespace log4cpp {
    class core_logger: public logger {
    public:
        core_logger();

        explicit core_logger(const std::string &log_name, log_level _level = log_level::WARN);

        core_logger(const core_logger &other);

        core_logger(core_logger &&other) noexcept;

        core_logger &operator=(const core_logger &other);

        core_logger &operator=(core_logger &&other) noexcept;

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

        void fatal(const char *__restrict fmt, ...) const override;

        void error(const char *__restrict fmt, ...) const override;

        void warn(const char *__restrict fmt, ...) const override;

        void info(const char *__restrict fmt, ...) const override;

        void debug(const char *__restrict fmt, ...) const override;

        void trace(const char *__restrict fmt, ...) const override;

        ~core_logger() override = default;

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
    };
} // namespace log4cpp
