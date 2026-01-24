#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <thread>

#include "appender/log_appender.hpp"
#include "common/log_net.hpp"
#include "config/appender.hpp"

namespace log4cpp::appender {
    constexpr std::chrono::seconds send_timeout = std::chrono::seconds(1);

    enum class connection_fsm_state { DISCONNECTED, IN_PROGRESS, ESTABLISHED };

    void to_string(connection_fsm_state state, std::string &str);
    void from_string(const std::string &str, connection_fsm_state &state);

    struct connect_result {
        common::socket_fd fd = common::INVALID_FD;
        connection_fsm_state state = connection_fsm_state::DISCONNECTED;
    };

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
        common::prefer_stack ip_stack;

        std::shared_mutex connection_rw_lock;
        common::socket_fd sock_fd; // Socket file descriptor
        connection_fsm_state connection_state;

        std::mutex reconnect_mutex;              // Mutex for reconnect condition variable
        std::condition_variable reconnect_cv;    // Condition variable for reconnecting
        std::atomic<bool> stop_reconnect{false}; // Flag to stop reconnect thread
        void reconnect_thread_routine();         // Reconnect thread function
        std::thread reconnect_thread;            // Reconnect thread
        // Current delay for reconnection
        std::chrono::seconds reconnect_delay{0};
        // Initial delay for reconnection
        static constexpr std::chrono::seconds RECONNECT_INITIAL_DELAY{1}; // 1 seconds
        // Maximum delay for reconnection
        static constexpr std::chrono::hours RECONNECT_MAX_DELAY{24}; // 24 hours

        [[nodiscard]] std::optional<common::net_addr> resolve_host() const;
        [[nodiscard]] connect_result connect_to_server(const common::sock_addr &saddr) const;
        void udp_init();
        void try_connect();
        void check_conn_status();
        void wait_for_reconnect_or_stop();
        void schedule_backoff();
        void reset_backoff();
    };
} // namespace log4cpp::appender
