#include <filesystem>
#include <thread>

#ifdef _WIN32
#include <WS2tcpip.h>
#include <windows.h>
#include <winsock2.h>
#endif

#ifdef __linux__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif

#if defined(_WIN32) || defined(_WIN64)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include <gtest/gtest.h>

#include "log4cpp/log4cpp.hpp"

#include "common/log_net.hpp"
#include "config/log4cpp.hpp"

int main(int argc, char **argv) {
    const std::string cur_path = argv[0];
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void set_socket_recv_timeout(log4cpp::common::socket_fd sockfd) {
#ifdef _WIN32
    unsigned int milliseconds = 1000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&milliseconds), sizeof(milliseconds))
        == SOCKET_ERROR) {
        fprintf(stderr, "[set_socket_recv_timeout] Set socket receive timeout failed: %u\n", WSAGetLastError());
        fflush(stderr);
    }
#else
    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("[set_socket_recv_timeout] Error setting receive timeout");
        fflush(stderr);
    }
#endif
}

void tcp_log_server_loop(std::atomic<bool> &srv_running, log4cpp::common::prefer_stack prefer, unsigned short port,
                         unsigned int expected_log_count) {
    std::string prefer_str;
    log4cpp::common::to_string(prefer, prefer_str);
    log4cpp::common::socket_fd server_fd =
        socket(prefer == log4cpp::common::prefer_stack::IPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (log4cpp::common::INVALID_FD == server_fd) {
        fprintf(stderr, "[tcp_socket_appender_test] socket creation failed...\n");
        fflush(stderr);
        return;
    }

    sockaddr_storage local_addr{};
    if (prefer == log4cpp::common::prefer_stack::IPv6) {
        // NOLINTNEXTLINE
        sockaddr_in6 &local_addr6 = reinterpret_cast<sockaddr_in6 &>(local_addr);
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr = in6addr_any;
        local_addr6.sin6_port = htons(port);
    }
    else {
        // NOLINTNEXTLINE
        sockaddr_in &local_addr4 = reinterpret_cast<sockaddr_in &>(local_addr);
        local_addr4.sin_family = AF_INET;
        local_addr4.sin_addr.s_addr = INADDR_ANY;
        local_addr4.sin_port = htons(port);
    }

    int val = bind(server_fd, reinterpret_cast<sockaddr *>(&local_addr),
                   prefer == log4cpp::common::prefer_stack::IPv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in));
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        fprintf(stderr, "[tcp_socket_appender_test] %s,L%d,bind failed! errno:%d\n", __func__, __LINE__, wsa_error);
#else
        fprintf(stderr, "[tcp_socket_appender_test] %s,L%d,bind failed! errno:%d,%s\n", __func__, __LINE__, errno,
                strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        fflush(stderr);
        return;
    }
    val = listen(server_fd, 5);
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        fprintf(stderr, "[tcp_socket_appender_test] %s,L%d,listen failed! errno:%d\n", __func__, __LINE__, wsa_error);
#else
        fprintf(stderr, "[tcp_socket_appender_test] %s,L%d,listen failed! errno:%d,%s\n", __func__, __LINE__, errno,
                strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        fflush(stderr);
        return;
    }

    sockaddr_storage client_addr{};
    socklen_t addr_len = 0;
    if (prefer == log4cpp::common::prefer_stack::IPv6) {
        addr_len = sizeof(sockaddr_in6);
    }
    else {
        addr_len = sizeof(sockaddr_in);
    }
    log4cpp::common::socket_fd client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &addr_len);
    if (log4cpp::common::INVALID_FD == client_fd) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        fprintf(stderr, "[tcp_socket_appender_test] %s,L%d,accept failed! errno:%d\n", __func__, __LINE__, wsa_error);
#else
        fprintf(stderr, "[tcp_socket_appender_test] %s,L%d,accept failed! errno:%d,%s\n", __func__, __LINE__, errno,
                strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        fflush(stderr);
        return;
    }
    char buffer[1024];
    unsigned int actual_log_count = 0;
    set_socket_recv_timeout(client_fd);
    srv_running.store(true);
    while (srv_running.load()) {
        ssize_t len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (len > 0) {
            buffer[len] = 0;
            ++actual_log_count;
            printf("[tcp_socket_appender_test] [%u]: %s", actual_log_count, buffer);
        }
        else if (len == -1) {
            if (errno != EINTR) {
                break;
            }
        }
        else {
            // Connection closed by peer
            break;
        }
    }
#ifdef _WIN32
    shutdown(client_fd, SD_BOTH);
#else
    shutdown(client_fd, SHUT_RDWR);
#endif
    log4cpp::common::close_socket(client_fd);
    log4cpp::common::close_socket(server_fd);
    ASSERT_GE(expected_log_count, actual_log_count);
    fflush(stdout);
}

void udp_log_server_loop(std::atomic<bool> &srv_running, log4cpp::common::prefer_stack prefer, unsigned short port,
                         unsigned int expected_log_count) {
    log4cpp::common::socket_fd server_fd =
        socket(prefer == log4cpp::common::prefer_stack::IPv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (log4cpp::common::INVALID_FD == server_fd) {
        fprintf(stderr, "[udp_socket_appender_test] socket creation failed...\n");
        fflush(stderr);
        return;
    }

    sockaddr_storage local_addr{};
    socklen_t addr_len = 0;
    if (prefer == log4cpp::common::prefer_stack::IPv6) {
        // NOLINTNEXTLINE
        sockaddr_in6 &local_addr6 = reinterpret_cast<sockaddr_in6 &>(local_addr);
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr = in6addr_any;
        local_addr6.sin6_port = htons(port);
        addr_len = sizeof(sockaddr_in6);
    }
    else {
        // NOLINTNEXTLINE
        sockaddr_in &local_addr4 = reinterpret_cast<sockaddr_in &>(local_addr);
        local_addr4.sin_family = AF_INET;
        local_addr4.sin_addr.s_addr = INADDR_ANY;
        local_addr4.sin_port = htons(port);
        addr_len = sizeof(sockaddr_in);
    }

    int reuse = 1;
    int result = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));
    if (result < 0) {
#ifndef _WIN32
        fprintf(stderr, "[udp_socket_appender_test] setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
#else
        fprintf(stderr, "[udp_socket_appender_test] setsockopt(SO_REUSEADDR) failed (WinError: %d)\n",
                WSAGetLastError());
#endif
        fflush(stderr);
    }

#ifndef _WIN32
    result = setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    if (result < 0) {
        fprintf(stderr, "[udp_socket_appender_test] setsockopt(SO_REUSEPORT) failed: %s\n", strerror(errno));
        fflush(stderr);
    }
#endif

    int val = bind(server_fd, reinterpret_cast<sockaddr *>(&local_addr), addr_len);
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        fprintf(stderr, "[udp_socket_appender_test] %s,L%d,bind failed! errno:%d\n", __func__, __LINE__, wsa_error);
#else
        fprintf(stderr, "[udp_socket_appender_test] %s,L%d,bind failed! errno:%d,%s\n", __func__, __LINE__, errno,
                strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        fflush(stderr);
        return;
    }

    set_socket_recv_timeout(server_fd);
    char buffer[1024];
    unsigned int actual_log_count = 0;
    srv_running.store(true);
    while (srv_running.load()) {
        ssize_t len = recvfrom(server_fd, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
        if (len > 0) {
            buffer[len] = 0;
            ++actual_log_count;
            printf("[udp_socket_appender_test] [%u]: %s", actual_log_count, buffer);
        }
        else if (len == -1) {
            if (errno != EINTR) {
                break;
            }
        }
        // Since UDP is connectionless, a recvfrom return value of 0 signifies the successful receipt of a zero-length
        // datagram, and the program should continue its loop
    }
#ifdef _WIN32
    shutdown(server_fd, SD_BOTH);
#else
    shutdown(server_fd, SHUT_RDWR);
#endif
    log4cpp::common::close_socket(server_fd);
    ASSERT_GE(expected_log_count, actual_log_count);
}

TEST(socket_appender_test, tcp_socket_appender_test) {
    const std::string config_file = "tcp_socket_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    const log4cpp::config::socket_appender &socker_appender_cfg = config->appenders.socket.value();
    unsigned short port = socker_appender_cfg.port;
    log4cpp::common::prefer_stack prefer = socker_appender_cfg.prefer;

#ifdef _WIN32
    WSADATA wsa_data{};
    (void)WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    std::atomic<bool> running(false);

    const std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger();
    log4cpp::log_level max_level = log->get_level();
    unsigned int expected_log_count = static_cast<int>(max_level) + 1; // enum is zero-indexed

    std::thread log_server_thread =
        std::thread(&tcp_log_server_loop, std::ref(running), prefer, port, expected_log_count);

    while (!running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    running.store(false);
    log_server_thread.join();
#ifdef _WIN32
    WSACleanup();
#endif
}

TEST(socket_appender_test, udp_socket_appender_test) {
    const std::string config_file = "udp_socket_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    const log4cpp::config::socket_appender &socker_appender_cfg = config->appenders.socket.value();
    unsigned short port = socker_appender_cfg.port;
    log4cpp::common::prefer_stack prefer = socker_appender_cfg.prefer;

#ifdef _WIN32
    WSADATA wsa_data{};
    (void)WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    std::atomic<bool> running(false);
    const std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger();
    log4cpp::log_level max_level = log->get_level();
    unsigned int expected_log_count = static_cast<int>(max_level) + 1; // enum is zero-indexed

    std::thread log_server_thread =
        std::thread(&udp_log_server_loop, std::ref(running), prefer, port, expected_log_count);
    while (!running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");
    running.store(false);
    log_server_thread.join();
#ifdef _WIN32
    WSACleanup();
#endif
}
