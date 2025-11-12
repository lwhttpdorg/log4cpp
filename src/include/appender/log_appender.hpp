#pragma once

#include <cstddef>

namespace log4cpp::appender {
    class log_appender {
    public:
        /**
         * @brief Write log to appender
         * @param msg: log message
         * @param msg_len: the length of message
         */
        virtual void log(const char *msg, size_t msg_len) = 0;

        virtual ~log_appender() = default;
    };
}
