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

#include "appender/tcp_appender.hpp"

#include <log4cpp/log4cpp.hpp>

namespace log4cpp::appender {
    /* TCP server accept thread running flag */
    static std::atomic_bool running{true};

    common::socket_fd create_tcp_socket(const common::sock_addr &saddr) {
        common::socket_fd fd;
        if (saddr.addr.family == common::net_family::NET_IPv4) {
            fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        }
        else {
            fd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
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
            if (bind(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
                common::close_socket(fd);
                return common::INVALID_FD;
            }
        }
        else {
            sockaddr_in6 server_addr{};
            server_addr.sin6_family = AF_INET6;
            server_addr.sin6_port = htons(saddr.port);
            server_addr.sin6_addr = in6addr_any;
            if (bind(fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
                common::close_socket(fd);
                return common::INVALID_FD;
            }
        }
        if (listen(fd, 5) == -1) {
            common::close_socket(fd);
            return common::INVALID_FD;
        }
        return fd;
    }

    void accept_worker(common::socket_fd listen_fd, common::log_lock &lock,
                       std::unordered_set<common::socket_fd> &clients) {
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
                sockaddr_storage client_addr{};
                socklen_t client_addr_len = sizeof(client_addr);
                common::socket_fd client_fd =
                    accept(listen_fd, reinterpret_cast<struct sockaddr *>(&client_addr), &client_addr_len);
                if (common::INVALID_FD != client_fd) {
#ifdef _DEBUG
                    char client_ip[INET6_ADDRSTRLEN];
                    unsigned short port;
                    if (AF_INET == client_addr.ss_family) {
                        const sockaddr_in *client_addr4 = reinterpret_cast<struct sockaddr_in *>(&client_addr);
                        port = ntohs(client_addr4->sin_port);
                        inet_ntop(client_addr4->sin_family, &client_addr4->sin_addr, client_ip, sizeof(client_ip));
                    }
                    else {
                        const sockaddr_in6 *client_addr6 = reinterpret_cast<struct sockaddr_in6 *>(&client_addr);
                        port = ntohs(client_addr6->sin6_port);
                        inet_ntop(client_addr6->sin6_family, &client_addr6->sin6_addr, client_ip, sizeof(client_ip));
                    }
                    printf("New TCP client: %s@%hu\n", client_ip, port);
#endif
                    std::lock_guard lock_guard(lock);
                    clients.insert(client_fd);
                    FD_SET(client_fd, &read_fds);
                }
            }
            std::lock_guard lock_guard(lock);
            for (auto it = clients.begin(); it != clients.end();) {
                common::socket_fd fd = *it;
                if (FD_ISSET(fd, &tmp_fds)) {
                    char buffer[LOG_LINE_MAX];
                    buffer[0] = '\0';
                    ssize_t len = recv(fd, buffer, sizeof(buffer), 0);
                    if (len < 0) {
#ifdef _WIN32
                        shutdown(fd, SD_BOTH);
#else
                        shutdown(fd, SHUT_RDWR);
#endif
                        common::close_socket(fd);
                        it = clients.erase(it);
                        FD_CLR(fd, &read_fds);
                    }
                    else {
                        ++it;
                    }
                }
                else {
                    ++it;
                }
            }
        }
    }

    tcp_appender::tcp_appender(const config::tcp_appender &cfg) {
        this->fd = common::INVALID_FD;

        common::sock_addr saddr;
        saddr.addr = cfg.local_addr;
        saddr.port = cfg.port;
        common::socket_fd server_fd = create_tcp_socket(saddr);
        if (common::INVALID_FD == server_fd) {
            throw std::runtime_error("Can not create tcp socket");
        }
        this->fd = server_fd;
        this->accept_thread = std::thread(accept_worker, server_fd, std::ref(this->lock), std::ref(this->clients));
    }

    tcp_appender::~tcp_appender() {
        running.store(false);
        this->accept_thread.join();
        if (common::INVALID_FD != this->fd) {
            common::close_socket(this->fd);
        }
        for (auto &client: this->clients) {
#ifdef _WIN32
            shutdown(client, SD_BOTH);
#else
            shutdown(client, SHUT_RDWR);
#endif
            common::close_socket(client);
        }
    }

    void tcp_appender::log(const char *msg, size_t msg_len) {
        std::lock_guard lock_guard(this->lock);
        for (auto &client: this->clients) {
            (void)send(client, msg, msg_len, 0);
        }
    }
}
