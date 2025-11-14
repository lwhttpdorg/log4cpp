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

#include <gtest/gtest.h>

#include "log4cpp/log4cpp.hpp"

#include "common/log_net.hpp"
#include "config/log4cpp.hpp"

class TestEnvironment: public testing::Environment {
public:
    explicit TestEnvironment(const std::string &cur_path) {
        size_t end = cur_path.find_last_of('\\');
        if (end == std::string::npos) {
            end = cur_path.find_last_of('/');
        }
        const std::string work_dir = cur_path.substr(0, end);
        std::filesystem::current_path(work_dir);
    }
};

int main(int argc, char **argv) {
    const std::string cur_path = argv[0];
    testing::InitGoogleTest(&argc, argv);
    AddGlobalTestEnvironment(new TestEnvironment(cur_path));
    return RUN_ALL_TESTS();
}

int udp_appender_client(std::atomic<bool> &running, unsigned int log_count, unsigned port) {
    log4cpp::common::socket_fd fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (log4cpp::common::INVALID_FD == fd) {
        printf("socket creation failed...\n");
        running.store(true);
        return -1;
    }
    const char *hello = "hello";
    const char *bye = "bye";

    sockaddr_in remote_addr = {};
    remote_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(remote_addr.sin_addr.s_addr));
    remote_addr.sin_port = htons(port);

    socklen_t socklen = sizeof(remote_addr);
    if (connect(fd, reinterpret_cast<sockaddr *>(&remote_addr), socklen) < 0) {
        perror("connect failed");
        running.store(true);
        return -1;
    }

    // sendto(fd, hello, strlen(hello), 0, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
    send(fd, hello, strlen(hello), 0);
    running.store(true);

    struct timeval tv{};
    tv.tv_sec = 10; // 2s
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    char buffer[1024];
    unsigned int index = 0;
    for (index = 0; index < log_count; ++index) {
        ssize_t len = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (len > 0) {
            buffer[len] = 0;
            printf("UDP[%u]: %s", index, buffer);
        }
        else {
            break;
        }
    }
    // sendto(fd, bye, strlen(bye), 0, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
    send(fd, bye, strlen(bye), 0);
    log4cpp::common::close_socket(fd);
    EXPECT_EQ(index, log_count);
    return 0;
}

TEST(udp_appender_test, udp_appender_test) {
    const std::string config_file = "tcp_udp_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    const log4cpp::config::udp_appender &udp_config = config->appenders.udp.value();
    unsigned short port = udp_config.port;

#ifdef _WIN32
    WSADATA wsa_data{};
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    std::atomic<bool> running(false);

    const std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger("udp");
    log4cpp::log_level max_level = log->get_level();
    unsigned int log_count = static_cast<int>(max_level);

    std::thread client_thread = std::thread(&udp_appender_client, std::ref(running), log_count, port);

    while (!running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");

    client_thread.join();
#ifdef _WIN32
    WSACleanup();
#endif
}
