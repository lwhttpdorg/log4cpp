#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>

#include "appender/log_appender.hpp"
#include "common/log_lock.hpp"
#include "common/log_net.hpp"
#include "config/appender.hpp"

namespace log4cpp::appender {
    class socket_appender: public log_appender {
    public:
        explicit socket_appender(const config::socket_appender &cfg);
        ~socket_appender() override;
        socket_appender(const socket_appender &other) = delete;
        socket_appender(socket_appender &&other) = delete;
        socket_appender &operator=(const socket_appender &other) = delete;
        socket_appender &operator=(socket_appender &&other) = delete;
        void log(const char *msg, size_t msg_len) override;

    private:
        std::string host;
        unsigned short port{0};
        config::socket_appender::protocol proto;

        common::socket_fd sock_fd; // Socket file descriptor
        std::mutex reconnect_mutex; // Mutex for reconnect condition variable
        std::condition_variable reconnect_cv; // Condition variable for reconnecting
        std::atomic<bool> stop_reconnect{false}; // Flag to stop reconnect thread
        void reconnect_worker_loop(); // Reconnect thread function
        std::thread reconnect_thread; // Reconnect thread
        // Current delay for reconnection
        std::chrono::seconds reconnect_delay{0};
        // Initial delay for reconnection
        static constexpr std::chrono::seconds RECONNECT_INITIAL_DELAY{10}; // 10 seconds
        // Maximum delay for reconnection
        static constexpr std::chrono::hours RECONNECT_MAX_DELAY{24}; // 24 hours

        [[nodiscard]] std::optional<common::net_addr> resolve_host() const;
        [[nodiscard]] common::socket_fd connect_to_server(const common::sock_addr &saddr) const;
        void schedule_backoff();
        void reset_backoff();

        [[nodiscard]] bool is_connected() const {
            return common::INVALID_FD != this->sock_fd;
        }
    };
}
