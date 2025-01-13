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

void info_layout() {
	const std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("info_layout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

void warn_layout() {
	const std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("warn_layout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

void load_configuration() {
	const std::string config_file = "file_appender_test.json";
	log4cpp::layout_manager::load_config(config_file);
}

TEST(file_appender_test, single_thread_test) {
	load_configuration();
	info_layout();
	warn_layout();
}

TEST(file_appender_test, multithread_test) {
	load_configuration();
	std::thread info_layout_thread(info_layout);
	std::thread warn_layout_thread(warn_layout);
	info_layout_thread.join();
	warn_layout_thread.join();
}
