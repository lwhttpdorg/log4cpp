#include <thread>
#include <filesystem>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <WS2tcpip.h>
#endif

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "gtest/gtest.h"

#include "log4cpp.hpp"
#include "main/log4cpp_config.h"
#include "main/udp_appender.h"

class TestEnvironment : public testing::Environment {
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

int udp_appender_client(std::atomic_bool &running, unsigned int log_count, unsigned port) {
	log4cpp::net::socket_fd fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (log4cpp::net::INVALID_FD == fd) {
		printf("socket creation failed...\n");
		return -1;
	}
	const char *hello = "hello";
	const char *bye = "bye";

	sockaddr_in remote_addr = {};
	remote_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &(remote_addr.sin_addr.s_addr));
	remote_addr.sin_port = htons(port);
	socklen_t socklen = sizeof(remote_addr);
	sendto(fd, hello, strlen(hello), 0, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
	running.store(true);
	char buffer[1024];
	for (unsigned int i = 0; i < log_count; ++i) {
		int len = recv(fd, buffer, sizeof(buffer) - 1, 0);
		if (len > 0) {
			buffer[len] = 0;
			printf("UDP[%u]: %s", i, buffer);
		}
		else if (len == -1) {
			break;
		}
	}
	sendto(fd, bye, strlen(bye), 0, reinterpret_cast<sockaddr *>(&remote_addr), socklen);
	log4cpp::net::close_socket(fd);
	return 0;
}

TEST(udp_appender_test, udp_appender_test) {
	std::string config_file = "tcp_udp_appender_test.json";
	log4cpp::layout_manager::load_config(config_file);
	const log4cpp::log4cpp_config *config = log4cpp::layout_manager::get_config();
	const log4cpp::udp_appender_config *udp_config = config->get_appender().get_udp_cfg();
	unsigned short port = udp_config->get_port();

#ifdef _WIN32
	WSADATA wsa_data{};
	WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif
	std::atomic_bool running(false);

	const std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("udpLayout");
	log4cpp::log_level max_level = log->get_level();
	unsigned int log_count = static_cast<int>(max_level);

	std::thread client_thread = std::thread(&udp_appender_client, std::ref(running), log_count, port);

	while (!running) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");

	client_thread.join();
#ifdef _WIN32
	WSACleanup();
#endif
}
