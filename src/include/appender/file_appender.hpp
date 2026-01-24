#pragma once

#include "appender/log_appender.hpp"
#include "common/log_lock.hpp"
#include "config/appender.hpp"

namespace log4cpp::appender {
    class file_appender: public log_appender {
    public:
        explicit file_appender(const config::file_appender &cfg);
        file_appender(const file_appender &other) = delete;

        file_appender(file_appender &&other) = delete;

        file_appender &operator=(const file_appender &other) = delete;

        file_appender &operator=(file_appender &&other) = delete;

        void log(const char *msg, size_t msg_len) override;

        ~file_appender() override;

    private:
        /* The fd of the log file */
        int fd{-1};
        common::log_lock lock;
    };
} // namespace log4cpp::appender
