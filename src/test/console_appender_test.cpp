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

void console_appender_layout() {
	const std::shared_ptr<log4cpp::logger> log = log4cpp::layout_manager::get_layout("console_layout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

TEST(console_appender_test, stdout_test) {
	const std::string config_file = "console_appender_stdout.json";
	log4cpp::layout_manager::load_config(config_file);
	console_appender_layout();
}

TEST(console_appender_test, stderr_test) {
	const std::string config_file = "console_appender_stderr.json";
	log4cpp::layout_manager::load_config(config_file);
	console_appender_layout();
}

TEST(console_appender_test, multithread_test) {
	std::thread info_layout_thread(console_appender_layout);
	std::thread warn_layout_thread(console_appender_layout);
	info_layout_thread.join();
	warn_layout_thread.join();
}
