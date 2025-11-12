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

int tcp_appender_client(std::atomic_bool &running, std::atomic_bool &finished, unsigned int log_count, unsigned port) {
    log4cpp::common::socket_fd fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (log4cpp::common::INVALID_FD == fd) {
        printf("[log4cpp] socket creation failed...\n");
        return -1;
    }

    sockaddr_in remote_addr = {};
    remote_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &(remote_addr.sin_addr.s_addr));
    remote_addr.sin_port = htons(port);
    socklen_t socklen = sizeof(remote_addr);
    int val = connect(fd, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
    if (-1 == val) {
#ifdef _WIN32
        int wsa_error = WSAGetLastError();
        printf("[log4cpp] %s,L%d,connect failed! errno:%d\n", __func__, __LINE__, wsa_error);
#else
        printf("[log4cpp] %s,L%d,connect failed! errno:%d,%s\n", __func__, __LINE__, errno, strerror(errno));
#endif
        log4cpp::common::close_socket(fd);
        running.store(true);
        EXPECT_GE(val, 0);
        return -2;
    }
    running.store(true);
    char buffer[1024];
    unsigned int index = 0;
    while (!finished) {
        ssize_t len = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (len > 0) {
            buffer[len] = 0;
            printf("TCP[%u]: %s", index++, buffer);
        }
        else if (len == -1) {
            break;
        }
    }
    EXPECT_EQ(index, log_count);
#ifdef _WIN32
    shutdown(fd, SD_BOTH);
#else
    shutdown(fd, SHUT_RDWR);
#endif
    log4cpp::common::close_socket(fd);
    return 0;
}

TEST(tcp_appender_test, tcp_appender_test) {
    const std::string config_file = "tcp_udp_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    const log4cpp::config::tcp_appender &tcp_config = config->appenders.tcp.value();
    unsigned short port = tcp_config.port;

#ifdef _WIN32
    WSADATA wsa_data{};
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
    std::atomic_bool running(false);
    std::atomic_bool finished(false);

    const std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger("tcp");
    log4cpp::log_level max_level = log->get_level();
    unsigned int log_count = static_cast<int>(max_level) + 1; // enum start from 0

    std::thread tcp_appender_thread =
        std::thread(&tcp_appender_client, std::ref(running), std::ref(finished), log_count, port);

    while (!running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    log->trace("this is a trace");
    log->info("this is a info");
    log->debug("this is a debug");
    log->warn("this is a warning");
    log->error("this is an error");
    log->fatal("this is a fatal");

    finished.store(true);

    tcp_appender_thread.join();
#ifdef _WIN32
    WSACleanup();
#endif
}
