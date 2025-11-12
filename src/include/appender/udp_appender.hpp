#pragma once

#include <thread>
#include <unordered_set>

#include "common/log_lock.hpp"
#include "common/log_net.hpp"
#include "log_appender.hpp"

#include <config/appender.hpp>

namespace log4cpp::appender {
    class udp_appender final: public log_appender {
    public:
        explicit udp_appender(const config::udp_appender &cfg);
        udp_appender(const udp_appender &other) = delete;

        udp_appender(udp_appender &&other) = delete;

        udp_appender &operator=(const udp_appender &other) = delete;

        udp_appender &operator=(udp_appender &&other) = delete;

        void log(const char *msg, size_t msg_len) override;

        ~udp_appender() override;

    private:
        /* UDP server socket fd */
        common::socket_fd fd;
        /* Address of clients */
        std::unordered_set<common::sock_addr> clients;
        /* UDP server accept thread */
        std::thread accept_thread;
        common::log_lock lock;
    };
}
