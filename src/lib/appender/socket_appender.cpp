#if defined(_WIN32)
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include <ws2tcpip.h>
#endif

#ifdef __linux__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif

#include <atomic>
#include <mutex>

#include "appender/socket_appender.hpp"

#include <log4cpp/log4cpp.hpp>

#include "common/log_net.hpp"

namespace log4cpp::appender {
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

    common::socket_fd socket_appender::connect_to_server(const common::sock_addr &saddr) const {
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
            return common::INVALID_FD;
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
        if (connect(fd, reinterpret_cast<sockaddr *>(&server_addr), addr_len) < 0) {
            common::close_socket(fd);
            return common::INVALID_FD;
        }
        return fd;
    }

    socket_appender::socket_appender(const config::socket_appender &cfg) :
        host(cfg.host), port(cfg.port), proto(cfg.proto), sock_fd(common::INVALID_FD) {
        auto addr = resolve_host();
        // If host is resolved, try to connect
        if (addr.has_value()) {
            common::sock_addr saddr;
            saddr.addr = addr.value();
            saddr.port = this->port;
            this->sock_fd = connect_to_server(saddr);
        }
        // For TCP, start reconnect thread if not connected
        if (true != this->is_connected() && config::socket_appender::protocol::TCP == this->proto) {
            this->reconnect_thread = std::thread(&socket_appender::reconnect_worker_loop, this);
        }
    }

    socket_appender::~socket_appender() {
        {
            std::lock_guard lock_guard(this->reconnect_mutex);
            this->stop_reconnect.store(true);
        }
        if (this->reconnect_thread.joinable()) {
            reconnect_cv.notify_one();
            this->reconnect_thread.join();
        }
        if (common::INVALID_FD != this->sock_fd) {
#ifdef _WIN32
            shutdown(this->sock_fd, SD_BOTH);
#else
            shutdown(this->sock_fd, SHUT_RDWR);
#endif
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

    void socket_appender::reconnect_worker_loop() {
        while (!this->stop_reconnect.load()) {
            // If connection is normal, wait for notification
            if (this->is_connected()) {
                std::unique_lock lock(this->reconnect_mutex);
                this->reconnect_cv.wait(lock, [this] { return this->stop_reconnect.load() || !this->is_connected(); });
            }
            // Check if you need to stop
            if (this->stop_reconnect.load()) {
                break;
            }
            // Try to reconnect
            if (this->reconnect_delay.count() > 0) {
                // Use wait_for instead of sleep_for to allow early wakeup
                std::unique_lock lock(this->reconnect_mutex);
                this->reconnect_cv.wait_for(lock, this->reconnect_delay,
                                            [this] { return this->stop_reconnect.load(); });
                // Check if you need to stop
                if (this->stop_reconnect.load()) {
                    break;
                }
            }
            // Reconnect
            auto addr = resolve_host();
            if (!addr.has_value()) {
                schedule_backoff();
                continue;
            }
            common::sock_addr saddr;
            saddr.addr = addr.value();
            saddr.port = this->port;
            auto addr_str = saddr.to_string();
            common::socket_fd fd = connect_to_server(saddr);
            if (common::INVALID_FD != fd) {
                this->sock_fd = fd;
                reset_backoff();
            }
            else {
                schedule_backoff();
            }
        }
    }

    void socket_appender::log(const char *msg, size_t msg_len) {
        if (!this->is_connected()) {
            return;
        }
        auto sent = send(this->sock_fd, msg, static_cast<int>(msg_len), 0);
        if (sent < 0) {
            // Connection lost, if protocol is TCP, notify reconnect thread
            if (config::socket_appender::protocol::TCP == this->proto) {
                if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN) {
                    // Connection lost, mark as disconnected
                    common::close_socket(this->sock_fd);
                    this->sock_fd = common::INVALID_FD;
                    // Notify reconnect thread
                    this->reconnect_cv.notify_one();
                }
                // For other errors, just ignore
            }
            // For UDP, just ignore the error
        }
    }
}
