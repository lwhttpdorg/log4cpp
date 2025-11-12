#pragma once

namespace log4cpp::log {
    class logger_proxy: public logger {
    public:
        explicit logger_proxy(std::shared_ptr<logger> target_logger);

        [[nodiscard]] std::string get_name() const override;

        [[nodiscard]] log_level get_level() const override;

        void log(log_level _level, const char *__restrict fmt, va_list args) const override;

        void fatal(const char *__restrict fmt, ...) const override;

        void error(const char *__restrict fmt, ...) const override;

        void warn(const char *__restrict fmt, ...) const override;

        void info(const char *__restrict fmt, ...) const override;

        void debug(const char *__restrict fmt, ...) const override;

        void trace(const char *__restrict fmt, ...) const override;

        ~logger_proxy() override = default;

        std::shared_ptr<logger> get_target();

        friend class log4cpp::logger_manager; // DO NOT remove qualifier!

    private:
        void set_target(std::shared_ptr<logger> target);
        void set_hot_reload_flag();
        void reset_hot_reload_flag();
        bool hot_reload_flag_is_set() const;

        mutable std::shared_mutex mtx;
        std::shared_ptr<logger> real_logger;
        unsigned int hot_reload_flag;
    };
}
