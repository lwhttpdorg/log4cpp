#pragma once

#include <common/log_lock.hpp>

#include "appender/console_appender.hpp"
#include "appender/log_appender.hpp"
#include "config/appender.hpp"

namespace log4cpp::appender {
    class console_appender: public log_appender {
    public:
        explicit console_appender(const config::console_appender &cfg);
        console_appender(const console_appender &other) = delete;

        console_appender &operator=(const console_appender &other) = delete;

        console_appender(console_appender &&other) = delete;

        console_appender &operator=(console_appender &&other) = delete;

        void log(const char *msg, size_t msg_len) override;

    private:
        /* The fd of console */
        int file_no = -1;
        common::log_lock lock;
    };
}
