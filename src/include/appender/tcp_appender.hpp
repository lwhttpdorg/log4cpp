#pragma once

#include <thread>
#include <unordered_set>

#include "appender/log_appender.hpp"
#include "common/log_lock.hpp"
#include "common/log_net.hpp"
#include "config/appender.hpp"

namespace log4cpp::appender {
    class tcp_appender: public log_appender {
    public:
        explicit tcp_appender(const config::tcp_appender &cfg);
        ~tcp_appender() override;

        tcp_appender(const tcp_appender &other) = delete;

        tcp_appender(tcp_appender &&other) = delete;

        tcp_appender &operator=(const tcp_appender &other) = delete;

        tcp_appender &operator=(tcp_appender &&other) = delete;

        void log(const char *msg, size_t msg_len) override;

    private:
        /* TCP server listen socket */
        common::socket_fd fd;
        /* Socket fds of connected clients */
        std::unordered_set<log4cpp::common::socket_fd> clients;
        /* Thread for accepting client connections */
        std::thread accept_thread;
        common::log_lock lock;
    };
}
