#include "gtest/gtest.h"
#include <thread>

#include "log4cpp.hpp"
#include "net_comm.h"
#include "main/log4cpp_config.h"
#include "main/tcp_appender.h"
#include "main/udp_appender.h"

namespace {
	bool backslash;
	std::string base_path;
}

class TestEnvironment : public testing::Environment {
public:
	explicit TestEnvironment(const std::string &cur_path) {
		size_t end = cur_path.find_last_of('\\');
		if (end != std::string::npos) {
			backslash = true;
		}
		else {
			backslash = false;
			end = cur_path.find_last_of('/');
		}
		base_path = cur_path.substr(0, end);
	}
};

int main(int argc, char **argv) {
	const std::string cur_path = argv[0];
	testing::InitGoogleTest(&argc, argv);
	testing::AddGlobalTestEnvironment(new TestEnvironment(cur_path));
	return RUN_ALL_TESTS();
}

void consoleAppenderTest() {
	const std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("consoleLayout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

void fileAppenderTest() {
	const std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("fileLayout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}


void rootLayoutTest() {
	std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("root");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

void layoutTest() {
	consoleAppenderTest();
	fileAppenderTest();
	rootLayoutTest();
}

TEST(logConfigTest, loadConfigTest1) {
	std::string config_file_path = base_path;
	if (backslash) {
		config_file_path += '\\';
	}
	else {
		config_file_path += '/';
	}
	config_file_path += "log4cpp-test-1.json";
	log4cpp::layout_manager::load_config(config_file_path);
	layoutTest();
}

TEST(logConfigTest, loadConfigTest2) {
	std::string config_file_path = base_path;
	if (backslash) {
		config_file_path += '\\';
	}
	else {
		config_file_path += '/';
	}
	config_file_path += "log4cpp-test-2.json";
	log4cpp::layout_manager::load_config(config_file_path);
	layoutTest();
}

int tcpAppenderTest(std::atomic_bool &running, unsigned int log_count, unsigned port);

int udpAppenderTest(std::atomic_bool &running, unsigned int log_count, unsigned port);

TEST(NetAppenderTest, TCPUDPAppenderTest) {
	std::string config_file_path = base_path;
	if (backslash) {
		config_file_path += '\\';
	}
	else {
		config_file_path += '/';
	}
	config_file_path += "log4cpp-tcp_udp-test.json";
	log4cpp::layout_manager::load_config(config_file_path);
	const log4cpp::log4cpp_config *config = log4cpp::layout_manager::get_config();
	const log4cpp::tcp_appender_config *tcp_config = config->get_appender().get_tcp_cfg();
	const log4cpp::udp_appender_config *udp_config = config->get_appender().get_udp_cfg();
	unsigned short tcp_port = tcp_config->port;
	unsigned short udp_port = udp_config->port;

#ifdef _WIN32
	winsock_init init;
#endif
	std::atomic_bool tcp_running(false);
	std::atomic_bool udp_running(false);

	const std::shared_ptr<log4cpp::layout> tcp_log = log4cpp::layout_manager::get_layout("tcpLayout");
	const std::shared_ptr<log4cpp::layout> udp_log = log4cpp::layout_manager::get_layout("udpLayout");

	std::thread tcp_appender_thread = std::thread(&tcpAppenderTest, std::ref(tcp_running), 5, tcp_port);
	std::thread udp_appender_thread = std::thread(&udpAppenderTest, std::ref(udp_running), 5, udp_port);

	while (!tcp_running || !udp_running) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	tcp_log->trace("this is a trace");
	tcp_log->info("this is a info");
	tcp_log->debug("this is a debug");
	tcp_log->error("this is an error");
	tcp_log->fatal("this is a fatal");

	udp_log->trace("this is a trace");
	udp_log->info("this is a info");
	udp_log->debug("this is a debug");
	udp_log->error("this is an error");
	udp_log->fatal("this is a fatal");

	tcp_appender_thread.join();
	udp_appender_thread.join();
}
