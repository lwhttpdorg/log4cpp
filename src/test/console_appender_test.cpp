#include <filesystem>
#include <thread>

#include "gtest/gtest.h"

#include "log4cpp.hpp"

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

void console_appender_logger() {
	auto &log_mgr = log4cpp::logger_manager::instance();
	const std::shared_ptr<log4cpp::logger> log = log_mgr.get_logger("console_logger");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

TEST(console_appender_test, stdout_test) {
	const std::string config_file = "console_appender_stdout.json";
	auto &log_mgr = log4cpp::logger_manager::instance();
	log_mgr.load_config(config_file);
	console_appender_logger();
}

TEST(console_appender_test, stderr_test) {
	const std::string config_file = "console_appender_stderr.json";
	auto &log_mgr = log4cpp::logger_manager::instance();
	log_mgr.load_config(config_file);
	console_appender_logger();
}

TEST(console_appender_test, multithread_test) {
	std::thread info_logger_thread(console_appender_logger);
	std::thread warn_logger_thread(console_appender_logger);
	info_logger_thread.join();
	warn_logger_thread.join();
}
