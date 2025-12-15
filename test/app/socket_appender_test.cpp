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

struct server_status {
    enum class state { AWAITING_STARTUP, RUNNING, FINISHED, FAILED };

    std::atomic<state> state{state::AWAITING_STARTUP};
    std::string error_message;
};

namespace log4cpp::common {
    inline int get_last_socket_error() {
#ifdef _WIN32
        return WSAGetLastError();
#else
        return errno;
#endif
    }
}

class socket_appender_test: public ::testing::Test {
protected:
    void SetUp() override {
#ifdef _WIN32
        WSADATA wsa_data{};
        (void)WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    }

    void TearDown() override {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

void set_reuse_addr_port(log4cpp::common::socket_fd fd) {
    int reuse = 1;
#ifdef _WIN32
    int result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&reuse), sizeof(reuse));
#else
    int result = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
#endif
    if (result < 0) {
#ifndef _WIN32
        log4cpp::common::log4c_debug(stderr, "[set_reuse_addr_port] setsockopt(SO_REUSEADDR) failed: %s\n",
                                     strerror(errno));
#else
        log4cpp::common::log4c_debug(stderr, "[set_reuse_addr_port] setsockopt(SO_REUSEADDR) failed (WinError: %d)\n",
                                     WSAGetLastError());
#endif
    }

#ifndef _WIN32
    result = setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    if (result < 0) {
        log4cpp::common::log4c_debug(stderr, "[set_reuse_addr_port] setsockopt(SO_REUSEPORT) failed: %s\n",
                                     strerror(errno));
    }
#endif
}

void set_socket_recv_timeout(log4cpp::common::socket_fd sockfd) {
#ifdef _WIN32
    unsigned int milliseconds = 1000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&milliseconds), sizeof(milliseconds))
        == SOCKET_ERROR) {
        log4cpp::common::log4c_debug(stderr, "[set_socket_recv_timeout] Set socket receive timeout failed: %u\n",
                                     WSAGetLastError());
    }
#else
    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        log4cpp::common::log4c_debug(stderr, "[set_socket_recv_timeout] Error setting receive timeout: %s(%d)",
                                     strerror(errno), errno);
    }
#endif
}

unsigned int tcp_log_server_loop(const std::shared_ptr<server_status> &status, log4cpp::common::prefer_stack prefer,
                                 unsigned short port) {
    log4cpp::common::socket_fd server_fd =
        socket(prefer == log4cpp::common::prefer_stack::IPv6 ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (log4cpp::common::INVALID_FD == server_fd) {
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] socket creation failed...\n");
        return 0;
    }

    sockaddr_storage local_addr{};
    socklen_t addr_len = 0;
#ifdef _DEBUG
    char addr_str[INET6_ADDRSTRLEN];
#endif
    if (prefer == log4cpp::common::prefer_stack::IPv6) {
        // NOLINTNEXTLINE
        sockaddr_in6 &local_addr6 = reinterpret_cast<sockaddr_in6 &>(local_addr);
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr = in6addr_any;
        local_addr6.sin6_port = htons(port);
        addr_len = sizeof(sockaddr_in6);
#ifdef _DEBUG
        inet_ntop(AF_INET6, &local_addr6.sin6_addr, addr_str, sizeof(addr_str));
#endif
    }
    else {
        // NOLINTNEXTLINE
        sockaddr_in &local_addr4 = reinterpret_cast<sockaddr_in &>(local_addr);
        local_addr4.sin_family = AF_INET;
        local_addr4.sin_addr.s_addr = INADDR_ANY;
        local_addr4.sin_port = htons(port);
        addr_len = sizeof(sockaddr_in);
#ifdef _DEBUG
        inet_ntop(AF_INET, &local_addr4.sin_addr, addr_str, sizeof(addr_str));
#endif
    }

#ifdef _DEBUG
    log4cpp::common::log4c_debug(stdout, "[tcp_socket_appender_test] bind %s@%u\n", addr_str, port);
#endif

    set_reuse_addr_port(server_fd);

    int val = bind(server_fd, reinterpret_cast<sockaddr *>(&local_addr), addr_len);
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] %s,L%d,bind failed! errno:%d\n", __func__,
                                     __LINE__, wsa_error);
#else
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] %s,L%d,bind failed! errno:%d,%s\n", __func__,
                                     __LINE__, errno, strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        status->error_message = "Server bind() failed.";
        status->state.store(server_status::state::FAILED);
        return 0;
    }
    val = listen(server_fd, 5);
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] %s,L%d,listen failed! errno:%d\n", __func__,
                                     __LINE__, wsa_error);
#else
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] %s,L%d,listen failed! errno:%d,%s\n", __func__,
                                     __LINE__, errno, strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        status->error_message = "Server listen() failed.";
        status->state.store(server_status::state::FAILED);
        return 0;
    }

    sockaddr_storage remote_addr{};
    log4cpp::common::socket_fd client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&remote_addr), &addr_len);
    if (log4cpp::common::INVALID_FD == client_fd) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] %s,L%d,accept failed! errno:%d\n", __func__,
                                     __LINE__, wsa_error);
#else
        log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] %s,L%d,accept failed! errno:%d,%s\n", __func__,
                                     __LINE__, errno, strerror(errno));
#endif
        log4cpp::common::close_socket(server_fd);
        status->error_message = "Server accept() failed.";
        status->state.store(server_status::state::FAILED);
        return 0;
    }
    char buffer[1024];
    std::string pending;
    unsigned int actual_log_count = 0;
    set_socket_recv_timeout(client_fd);
    status->state.store(server_status::state::RUNNING);
    while (true) {
        ssize_t len = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (len > 0) {
            buffer[len] = 0;
            pending.append(buffer, static_cast<size_t>(len));

            size_t pos = 0;
            while (true) {
                size_t nl = pending.find('\n', pos);
                if (nl == std::string::npos) break;

                std::string line = pending.substr(pos, nl - pos);
                if (!line.empty() && line.back() == '\r') line.pop_back();

                ++actual_log_count;
                printf("[tcp_socket_appender_test] [%u]: %s\n", actual_log_count, line.c_str());

                pos = nl + 1;
            }
            pending.erase(0, pos);
        }
        else if (len == -1) {
            int error = log4cpp::common::get_last_socket_error();
            if (error != EINTR && error != EAGAIN && error != EWOULDBLOCK) {
#ifdef _DEBUG
                log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] recv failed! errno:%d,%s\n", error,
                                             strerror(error));
#endif
                break; // Real error
            }
            // Timeout or interrupt, break the loop to check if test is done
            break;
        }
        else {
            // Connection closed by peer
#ifdef _DEBUG
            log4cpp::common::log4c_debug(stderr, "[tcp_socket_appender_test] recv failed! errno:%d,%s\n", errno,
                                         strerror(errno));
#endif
            break;
        }
    }
    status->state.store(server_status::state::FINISHED);
    log4cpp::common::shutdown_socket(client_fd);
    log4cpp::common::close_socket(client_fd);
    log4cpp::common::close_socket(server_fd);
    fflush(stdout);
    return actual_log_count;
}

TEST_F(socket_appender_test, tcp_socket_appender_test) {
    const std::string config_file = "tcp_socket_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_TRUE(config->appenders.socket.has_value());
    const log4cpp::config::socket_appender &socker_appender_cfg = config->appenders.socket.value();
    unsigned short port = socker_appender_cfg.port;
    log4cpp::common::prefer_stack prefer = socker_appender_cfg.prefer;

    auto status = std::make_shared<server_status>();
    unsigned int received_count = 0;

    const std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger();
    log4cpp::log_level max_level = log->get_level();
    unsigned int expected_log_count = static_cast<int>(max_level) + 1; // enum is zero-indexed

    std::thread log_server_thread([&]() { received_count = tcp_log_server_loop(std::cref(status), prefer, port); });

    while (status->state.load() == server_status::state::AWAITING_STARTUP) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");

    log_server_thread.join();
    ASSERT_NE(status->state.load(), server_status::state::FAILED) << status->error_message;
    ASSERT_EQ(expected_log_count, received_count);
}

unsigned int udp_log_server_loop(const std::shared_ptr<server_status> &status, log4cpp::common::prefer_stack prefer,
                                 unsigned short port, unsigned int expected_count) {
    log4cpp::common::socket_fd server_fd =
        socket(prefer == log4cpp::common::prefer_stack::IPv6 ? AF_INET6 : AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (log4cpp::common::INVALID_FD == server_fd) {
        log4cpp::common::log4c_debug(stderr, "[udp_socket_appender_test] socket creation failed...\n");
        return 0;
    }

    sockaddr_storage local_addr{};
    socklen_t addr_len = 0;
#ifdef _DEBUG
    char addr_str[INET6_ADDRSTRLEN];
#endif
    if (prefer == log4cpp::common::prefer_stack::IPv6) {
        // NOLINTNEXTLINE
        sockaddr_in6 &local_addr6 = reinterpret_cast<sockaddr_in6 &>(local_addr);
        local_addr6.sin6_family = AF_INET6;
        local_addr6.sin6_addr = in6addr_any;
        local_addr6.sin6_port = htons(port);
        addr_len = sizeof(sockaddr_in6);
#ifdef _DEBUG
        inet_ntop(AF_INET6, &local_addr6.sin6_addr, addr_str, sizeof(addr_str));
#endif
    }
    else {
        // NOLINTNEXTLINE
        sockaddr_in &local_addr4 = reinterpret_cast<sockaddr_in &>(local_addr);
        local_addr4.sin_family = AF_INET;
        local_addr4.sin_addr.s_addr = INADDR_ANY;
        local_addr4.sin_port = htons(port);
        addr_len = sizeof(sockaddr_in);
#ifdef _DEBUG
        inet_ntop(AF_INET, &local_addr4.sin_addr, addr_str, sizeof(addr_str));
#endif
    }
#ifdef _DEBUG
    log4cpp::common::log4c_debug(stdout, "[udp_socket_appender_test] bind %s@%u\n", addr_str, port);
#endif

    set_reuse_addr_port(server_fd);

    int val = bind(server_fd, reinterpret_cast<sockaddr *>(&local_addr), addr_len);
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        log4cpp::common::log4c_debug(stderr, "[udp_socket_appender_test] %s,L%d,bind failed! errno:%d\n", __func__,
                                     __LINE__, wsa_error);
#else
        log4cpp::common::log4c_debug(stderr, "[udp_socket_appender_test] %s,L%d,bind failed! errno:%d,%s\n", __func__,
                                     __LINE__, errno, strerror(errno));
#endif
        status->error_message = "Server bind() failed.";
        status->state.store(server_status::state::FAILED);
        return 0;
    }

    log4cpp::common::log4c_debug(stdout, "[udp_socket_appender_test] UDP log server is running...\n");

    set_socket_recv_timeout(server_fd);
    char buffer[1024];
    unsigned int actual_log_count = 0;
    sockaddr_storage remote_addr{};
    socklen_t remote_addr_len = sizeof(remote_addr);
    status->state.store(server_status::state::RUNNING);
    while (actual_log_count < expected_count) {
        ssize_t len = recvfrom(server_fd, buffer, sizeof(buffer) - 1, 0,
                               reinterpret_cast<struct sockaddr *>(&remote_addr), &remote_addr_len);
        if (len > 0) {
            buffer[len] = 0;
            ++actual_log_count;
            printf("[udp_socket_appender_test] [%u]: %s", actual_log_count, buffer);
        }
        else if (len == -1) {
            int error = log4cpp::common::get_last_socket_error();
            if (error != EINTR && error != EAGAIN && error != EWOULDBLOCK) {
#ifdef _DEBUG
                log4cpp::common::log4c_debug(stderr, "[udp_socket_appender_test] recvfrom failed! errno:%d,%s\n", error,
                                             strerror(error));
#endif
                break; // Real error
            }
            // Timeout or interrupt, break the loop to allow test to finish
            break;
        }
        // Since UDP is connectionless, a recvfrom return value of 0 signifies the successful receipt of a zero-length
        // datagram, and the program should continue its loop
    }
    status->state.store(server_status::state::FINISHED);
    log4cpp::common::shutdown_socket(server_fd);
    log4cpp::common::close_socket(server_fd);
    fflush(stdout);
    return actual_log_count;
}

TEST_F(socket_appender_test, udp_socket_appender_test) {
    const std::string config_file = "udp_socket_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_TRUE(config->appenders.socket.has_value());
    const log4cpp::config::socket_appender &socker_appender_cfg = config->appenders.socket.value();
    unsigned short port = socker_appender_cfg.port;
    log4cpp::common::prefer_stack prefer = socker_appender_cfg.prefer;

    auto status = std::make_shared<server_status>();
    unsigned int received_count = 0;

    const std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger();
    log4cpp::log_level max_level = log->get_level();
    unsigned int expected_log_count = static_cast<int>(max_level) + 1; // enum is zero-indexed

    std::thread log_server_thread(
        [&]() { received_count = udp_log_server_loop(std::cref(status), prefer, port, expected_log_count); });

    while (status->state.load() == server_status::state::AWAITING_STARTUP) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    ASSERT_NE(status->state.load(), server_status::state::FAILED) << status->error_message;

    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");

    log_server_thread.join();
    ASSERT_NE(status->state.load(), server_status::state::FAILED) << status->error_message;
    ASSERT_EQ(expected_log_count, received_count);
}
