#if defined(_WIN32)
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on
#include <ws2tcpip.h>
#endif

#ifdef __linux__

#include <netinet/in.h>
#include <sys/socket.h>

#endif

#include <atomic>
#include <mutex>

#include <log4cpp/log4cpp.hpp>

#include "appender/udp_appender.hpp"

namespace log4cpp::appender {
    const char *UDP_OUTPUT_HELLO = "hello";
    const char *UDP_OUTPUT_GOODBYE = "bye";

    /* UDP server accept thread running flag */
    static std::atomic<bool> running{true};
    common::socket_fd create_udp_socket(const common::sock_addr &saddr) {
        common::socket_fd fd;
        if (saddr.addr.family == common::net_family::NET_IPv4) {
            fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        }
        else {
            fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        }
        if (fd == common::INVALID_FD) {
            return common::INVALID_FD;
        }
        int opt = 1;
#ifdef _WIN32
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&opt), sizeof(opt));
#endif
#ifdef __linux__
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
#endif
        if (saddr.addr.family == common::net_family::NET_IPv4) {
            sockaddr_in server_addr{};
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(saddr.port);
            server_addr.sin_addr.s_addr = htonl(saddr.addr.ip.addr4);
            if (bind(fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
                common::close_socket(fd);
                return common::INVALID_FD;
            }
        }
        else {
            sockaddr_in6 server_addr{};
            server_addr.sin6_family = AF_INET6;
            server_addr.sin6_port = htons(saddr.port);
            server_addr.sin6_addr = in6addr_any;
            if (bind(fd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
                common::close_socket(fd);
                return common::INVALID_FD;
            }
        }
        return fd;
    }

    void accept_worker(common::socket_fd listen_fd, common::log_lock &lock,
                       std::unordered_set<common::sock_addr> &clients) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(listen_fd, &read_fds);
        timeval timeout{};
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        while (running.load()) {
            fd_set tmp_fds = read_fds;
            const int ret = select(listen_fd + 1, &tmp_fds, nullptr, nullptr, &timeout);
            if (ret == -1) {
                break;
            }
            if (ret == 0) {
                continue;
            }
            if (FD_ISSET(listen_fd, &tmp_fds)) {
                char buffer[LOG_LINE_MAX];
                buffer[0] = '\0';
                sockaddr_storage client_addr{};
                socklen_t client_addr_len = sizeof(client_addr);
                ssize_t len = recvfrom(listen_fd, buffer, sizeof(buffer) - 1, 0,
                                       reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);
                if (len < 0) {
                    continue;
                }

                common::sock_addr saddr{};
                if (client_addr.ss_family == AF_INET) {
                    const sockaddr_in *client_addr_in = reinterpret_cast<struct sockaddr_in *>(&client_addr);
                    saddr.addr.family = common::net_family::NET_IPv4;
                    saddr.addr.ip.addr4 = ntohl(client_addr_in->sin_addr.s_addr);
                    saddr.port = ntohs(client_addr_in->sin_port);
                }
                else {
                    const sockaddr_in6 *client_addr_in6 = reinterpret_cast<struct sockaddr_in6 *>(&client_addr);
                    saddr.addr.family = common::net_family::NET_IPv6;
                    memcpy(saddr.addr.ip.addr6, &client_addr_in6->sin6_addr, sizeof(saddr.addr.ip.addr6));
                    saddr.port = ntohs(client_addr_in6->sin6_port);
                }
#ifdef _DEBUG
                printf("New UDP client: %s\n", saddr.to_string().c_str());
#endif
                buffer[len] = '\0';
                if (strcmp(buffer, UDP_OUTPUT_HELLO) == 0) {
                    std::lock_guard lock_guard(lock);
                    clients.insert(saddr);
                }
                else if (strcmp(buffer, UDP_OUTPUT_GOODBYE) == 0) {
                    std::lock_guard lock_guard(lock);
                    clients.erase(saddr);
                }
            }
        }
    }

    udp_appender::udp_appender(const config::udp_appender &cfg) {
        this->fd = common::INVALID_FD;

        common::sock_addr saddr;
        saddr.addr = cfg.local_addr;
        saddr.port = cfg.port;
        common::socket_fd server_fd = create_udp_socket(saddr);
        if (server_fd == common::INVALID_FD) {
            throw std::runtime_error("Can not create tcp socket");
        }
        this->fd = server_fd;
        running.store(true);
        this->accept_thread = std::thread(accept_worker, server_fd, std::ref(this->lock), std::ref(this->clients));
    }

    udp_appender::~udp_appender() {
        running.store(false);
        this->accept_thread.join();
        if (this->fd != common::INVALID_FD) {
            common::close_socket(this->fd);
        }
    }

    void udp_appender::log(const char *msg, size_t msg_len) {
        std::lock_guard lock_guard(this->lock);
        for (auto &client: this->clients) {
            if (client.addr.family == common::net_family::NET_IPv4) {
                sockaddr_in client_addr{};
                client_addr.sin_family = AF_INET;
                client_addr.sin_port = htons(client.port);
                client_addr.sin_addr.s_addr = htonl(client.addr.ip.addr4);
                (void)sendto(this->fd, msg, msg_len, 0, reinterpret_cast<sockaddr *>(&client_addr),
                             sizeof(client_addr));
            }
            else {
                sockaddr_in6 client_addr{};
                client_addr.sin6_family = AF_INET6;
                client_addr.sin6_port = htons(client.port);
                client_addr.sin6_addr = in6addr_any;
                (void)sendto(this->fd, msg, msg_len, 0, reinterpret_cast<sockaddr *>(&client_addr),
                             sizeof(client_addr));
            }
        }
    }
}
