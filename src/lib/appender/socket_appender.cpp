#if defined(_WIN32)
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include <ws2tcpip.h>
#endif

#ifdef __linux__

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <atomic>
#include <mutex>

#include "appender/socket_appender.hpp"

#include <log4cpp/log4cpp.hpp>

#include "common/log_net.hpp"

namespace log4cpp::appender {
    void to_string(const connection_fsm_state state, std::string &str) {
        switch (state) {
            case connection_fsm_state::DISCONNECTED:
                str = "DISCONNECTED";
                break;
            case connection_fsm_state::IN_PROGRESS:
                str = "IN_PROGRESS";
                break;
            case connection_fsm_state::ESTABLISHED:
                str = "ESTABLISHED";
                break;
        }
    }

    void from_string(const std::string &str, connection_fsm_state &state) {
        if (str == "DISCONNECTED") {
            state = connection_fsm_state::DISCONNECTED;
        }
        else if (str == "IN_PROGRESS") {
            state = connection_fsm_state::IN_PROGRESS;
        }
        else if (str == "ESTABLISHED") {
            state = connection_fsm_state::ESTABLISHED;
        }
        else {
            throw std::invalid_argument("invalid connection_fsm_state \'" + str + "\'");
        }
    }

    std::optional<common::net_addr> socket_appender::resolve_host() const {
        // Resolve host
        std::optional<common::net_addr> addr;
        try {
            addr = common::net_addr::resolve(this->host);
        }
        catch (const common::host_resolve_exception &) {
            // Failed to resolve host, retry later
        }
        catch (const std::invalid_argument &) {
            // Invalid host string, rethrow exception
            throw;
        }
        return addr;
    }

    void set_fd_nonblock(common::socket_fd fd, bool nonblock) {
#ifdef _WIN32
        unsigned long mode = nonblock ? 1 : 0;
        ioctlsocket(fd, FIONBIO, &mode);
#else
        int flags = fcntl(fd, F_GETFL, 0);
        if (-1 == flags) {
            return;
        }
        if (nonblock) {
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        }
        else {
            fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
        }
#endif
    }

    void set_send_timeout(common::socket_fd fd, std::chrono::seconds timeout) {
#ifdef _WIN32
        unsigned int tv = static_cast<unsigned int>(timeout.count()) * 1000;
        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char *>(&tv), sizeof(tv)) == SOCKET_ERROR) {
            common::log4c_debug(stderr, "setsockopt SO_SNDTIMEO failed: %d\n", WSAGetLastError());
        }
#else
        timeval tv{};
        tv.tv_sec = timeout.count();
        tv.tv_usec = 0;
        if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            common::log4c_debug(stderr, "setsockopt SO_SNDTIMEO failed: %s(%d)\n", strerror(errno), errno);
        }
#endif
    }

    connect_result socket_appender::connect_to_server(const common::sock_addr &saddr) const {
        connect_result result{common::INVALID_FD, connection_fsm_state::DISCONNECTED};
        int family = saddr.addr.family == common::net_family::NET_IPv4 ? AF_INET : AF_INET6;
        int socket_type;
        int ip_proto;
        if (config::socket_appender::protocol::TCP == this->proto) {
            socket_type = SOCK_STREAM;
            ip_proto = IPPROTO_TCP;
        }
        else {
            socket_type = SOCK_DGRAM;
            ip_proto = IPPROTO_UDP;
        }

        common::socket_fd fd = socket(family, socket_type, ip_proto);
        if (fd == common::INVALID_FD) {
            return result;
        }
        sockaddr_storage server_addr{};
        socklen_t addr_len = 0;
        if (saddr.addr.family == common::net_family::NET_IPv4) {
            // NOLINTNEXTLINE
            sockaddr_in *addr4 = reinterpret_cast<sockaddr_in *>(&server_addr);
            addr4->sin_family = AF_INET;
            addr4->sin_port = htons(saddr.port);
            addr4->sin_addr.s_addr = saddr.addr.ip.addr4;
            addr_len = sizeof(sockaddr_in);
        }
        else {
            // NOLINTNEXTLINE
            sockaddr_in6 *addr6 = reinterpret_cast<sockaddr_in6 *>(&server_addr);
            addr6->sin6_family = AF_INET6;
            addr6->sin6_port = htons(saddr.port);
            std::memcpy(&addr6->sin6_addr.s6_addr, saddr.addr.ip.addr6, sizeof(saddr.addr.ip.addr6));
            addr_len = sizeof(sockaddr_in6);
        }

        set_fd_nonblock(fd, true);

        if (connect(fd, reinterpret_cast<sockaddr *>(&server_addr), addr_len) == 0) {
            result.fd = fd;
            result.state = connection_fsm_state::ESTABLISHED;
        }
        else {
#ifdef _WIN32
            if (common::IN_PROGRESS == WSAGetLastError()) {
#else
            if (common::IN_PROGRESS == errno) {
#endif
                result.fd = fd;
                result.state = connection_fsm_state::IN_PROGRESS;
            }
            else {
                common::close_socket(fd);
            }
        }

        return result;
    }

    void socket_appender::udp_init() {
        const auto addr = resolve_host();
        if (!addr.has_value()) {
            return;
        }
        common::sock_addr saddr{};
        saddr.addr = addr.value();
        saddr.port = this->port;
        connect_result result = connect_to_server(saddr);
#ifdef _DEBUG
        const auto addr_str = saddr.to_string();
        std::string state_str;
        to_string(result.state, state_str);
        common::log4c_debug(stdout, "UDP connect to %s, %s\n", addr_str.c_str(), state_str.c_str());
#endif
        if (connection_fsm_state::ESTABLISHED == result.state) {
            this->sock_fd = result.fd;
        }
    }

    socket_appender::socket_appender(const config::socket_appender &cfg) :
        host(cfg.host), port(cfg.port), proto(cfg.proto), sock_fd(common::INVALID_FD),
        connection_state(connection_fsm_state::DISCONNECTED) {
        // For TCP, start reconnect thread
        if (config::socket_appender::protocol::TCP == this->proto) {
            this->reconnect_thread = std::thread(&socket_appender::reconnect_thread_routine, this);
        }
        else {
            udp_init();
        }
    }

    socket_appender::~socket_appender() {
        if (config::socket_appender::protocol::TCP == this->proto) {
            {
                std::lock_guard lock_guard(this->reconnect_mutex);
                this->stop_reconnect.store(true);
                reconnect_cv.notify_one();
#ifdef _DEBUG
                common::log4c_debug(stdout, "[socket_appender] notify cv...\n");
#endif
            }
            {
                std::unique_lock w_lock(this->connection_rw_lock);
                if (common::INVALID_FD != this->sock_fd) {
                    common::shutdown_socket(this->sock_fd);
                    common::close_socket(this->sock_fd);
                    this->connection_state = connection_fsm_state::DISCONNECTED;
                }
            }
            if (this->reconnect_thread.joinable()) {
                this->reconnect_thread.join();
            }
        }
        else {
            common::close_socket(this->sock_fd);
        }
    }

    void socket_appender::schedule_backoff() {
        if (this->reconnect_delay.count() == 0) {
            this->reconnect_delay = RECONNECT_INITIAL_DELAY;
        }
        else {
            this->reconnect_delay *= 2;
            if (this->reconnect_delay > RECONNECT_MAX_DELAY) {
                this->reconnect_delay = RECONNECT_MAX_DELAY;
            }
        }
    }

    void socket_appender::reset_backoff() {
        this->reconnect_delay = std::chrono::seconds{0};
    }

    void socket_appender::try_connect() {
        // backoff
        if (this->reconnect_delay.count() > 0) {
#ifdef _DEBUG
            common::log4c_debug(stdout, "[socket_appender] exponential reconnection backoff...\n");
#endif
            // Use wait_for instead of sleep_for to allow early wakeup
            std::unique_lock lock(this->reconnect_mutex);
            this->reconnect_cv.wait_for(lock, this->reconnect_delay, [this] { return this->stop_reconnect.load(); });
            // Check if you need to stop
            if (this->stop_reconnect.load()) {
#ifdef _DEBUG
                common::log4c_debug(stdout, "[socket_appender] stop reconnect...break out of the loop\n");
#endif
                return;
            }
        }
        // Reconnect
        const auto addr = resolve_host();
        if (!addr.has_value()) {
            schedule_backoff();
            return;
        }
        common::sock_addr saddr{};
        saddr.addr = addr.value();
        saddr.port = this->port;
#ifdef _DEBUG
        auto addr_str = saddr.to_string();
        common::log4c_debug(stdout, "[socket_appender] try connecting to the server %s...\n", addr_str.c_str());
#endif
        connect_result result = connect_to_server(saddr);
        if (connection_fsm_state::DISCONNECTED != result.state) {
            std::unique_lock w_lock(this->connection_rw_lock);
            this->sock_fd = result.fd;
            this->connection_state = result.state;
            if (connection_fsm_state::ESTABLISHED == result.state) {
                set_fd_nonblock(this->sock_fd, false);
                set_send_timeout(this->sock_fd, send_timeout);
            }
        }
    }

    void socket_appender::check_conn_status() {
        std::unique_lock w_lock(this->connection_rw_lock);

        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(this->sock_fd, &writefds);

        timeval tv{};
        tv.tv_sec = 0;
        tv.tv_usec = 0;

#ifdef _WIN32
        int rc = select(0, nullptr, &writefds, nullptr, &tv);
#else
        int rc = select(sock_fd + 1, nullptr, &writefds, nullptr, &tv);
#endif
        if (0 == rc) {
            return;
        }
        if (0 > rc) {
            common::close_socket(this->sock_fd);
            this->sock_fd = common::INVALID_FD;
            this->connection_state = connection_fsm_state::DISCONNECTED;
            schedule_backoff();
            return;
        }

        int err = 0;
        socklen_t len = sizeof(err);
#ifdef _WIN32
        getsockopt(this->sock_fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&err), &len);
#else
        getsockopt(this->sock_fd, SOL_SOCKET, SO_ERROR, &err, &len);
#endif
        if (0 == err) {
            set_fd_nonblock(this->sock_fd, false);
            set_send_timeout(this->sock_fd, send_timeout);
            this->connection_state = connection_fsm_state::ESTABLISHED;
            reset_backoff();
#ifdef _DEBUG
            common::log4c_debug(stdout, "[socket_appender] connection established...\n");
#endif
        }
        else {
            common::close_socket(this->sock_fd);
            this->sock_fd = common::INVALID_FD;
            this->connection_state = connection_fsm_state::DISCONNECTED;
            schedule_backoff();
#ifdef _DEBUG
            common::log4c_debug(stdout, "[socket_appender] connection failed...will retry with backoff\n");
#endif
        }
    }

    void socket_appender::wait_for_reconnect_or_stop() {
#ifdef _DEBUG
        common::log4c_debug(
            stdout,
            "[socket_appender] connection is ESTABLISHED, waiting for a stop signal or the next reconnection...\n");
#endif
        // If connection is normal, wait for notification
        std::unique_lock lock(this->reconnect_mutex);
        this->reconnect_cv.wait(lock, [this] {
            return this->stop_reconnect.load() || connection_fsm_state::ESTABLISHED != this->connection_state;
        });
#ifdef _DEBUG
        std::string state_str;
        to_string(this->connection_state, state_str);
        common::log4c_debug(
            stdout, "[socket_appender] condition variable is awakened...connection state {%s}, stop connection {%s}\n",
            state_str.c_str(), this->stop_reconnect.load() ? "true" : "false");
#endif
    }

    void socket_appender::reconnect_thread_routine() {
        set_thread_name("reconnect_worker");
#ifdef _DEBUG
        common::log4c_debug(stdout, "[socket_appender] reconnect thread is running...\n");
#endif
        while (!this->stop_reconnect.load()) {
            connection_fsm_state current_state;
            {
                std::shared_lock r_lock(this->connection_rw_lock);
                current_state = this->connection_state;
            }
            switch (current_state) {
                case connection_fsm_state::DISCONNECTED:
                    try_connect();
                    break;
                case connection_fsm_state::IN_PROGRESS:
                    check_conn_status();
                    bool in_progress;
                    {
                        std::shared_lock r_lock(this->connection_rw_lock);
                        in_progress = connection_fsm_state::IN_PROGRESS == this->connection_state;
                    }
                    if (in_progress) {
                        std::unique_lock lock(this->reconnect_mutex);
                        this->reconnect_cv.wait_for(lock, std::chrono::seconds(1),
                                                    [this] { return this->stop_reconnect.load(); });
                    }
                    break;
                case connection_fsm_state::ESTABLISHED:
                    wait_for_reconnect_or_stop();
                    break;
            }
        }
#ifdef _DEBUG
        common::log4c_debug(stdout, "[socket_appender] reconnect thread is stop...\n");
#endif
    }

    void socket_appender::log(const char *msg, size_t msg_len) {
        if (config::socket_appender::protocol::TCP == this->proto) {
            std::shared_lock r_lock(this->connection_rw_lock);
            if (connection_fsm_state::ESTABLISHED != this->connection_state) {
                return;
            }
            ssize_t sent = send(this->sock_fd, msg, static_cast<int>(msg_len), 0);
            if (sent < 0) {
                // Connection lost, notify reconnect thread
                r_lock.unlock();
                std::unique_lock w_lock(this->connection_rw_lock);
                // NOLINTNEXTLINE (double check)
                if (connection_fsm_state::ESTABLISHED == this->connection_state) {
#ifdef _WIN32
                    auto wsaerr = WSAGetLastError();
                    if (WSAECONNRESET == wsaerr || WSAESHUTDOWN == wsaerr || WSAENOTCONN == wsaerr) {
#else
                    if (errno == EPIPE || errno == ECONNRESET) {
#endif
                        // Connection lost, mark as disconnected
                        common::close_socket(this->sock_fd);
                        this->sock_fd = common::INVALID_FD;
                        this->connection_state = connection_fsm_state::DISCONNECTED;
                        // Notify reconnect thread
                        this->reconnect_cv.notify_one();
                    }
                    // For other errors, just ignore
                }
            }
        }
        else {
            // For UDP, just ignore the error
            (void)send(this->sock_fd, msg, static_cast<int>(msg_len), 0);
        }
    }
}
