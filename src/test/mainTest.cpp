#include "gtest/gtest.h"
#include <thread>

#include "log4cpp.hpp"

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
	std::string cur_path = argv[0];
	testing::InitGoogleTest(&argc, argv);
	testing::AddGlobalTestEnvironment(new TestEnvironment(cur_path));
	return RUN_ALL_TESTS();
}

void consoleOutputTest() {
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("Child: this is a trace");
	logger->info("Child: this is a info");
	logger->debug("Child: this is a debug");
	logger->error("Child: this is an error");
	logger->fatal("Child: this is a fatal");
}

void fileOutputTest() {
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("Child: this is a trace");
	logger->info("Child: this is a info");
	logger->debug("Child: this is a debug");
	logger->error("Child: this is an error");
	logger->fatal("Child: this is a fatal");
}

void rootLoggerTest() {
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("root");
	logger->trace("Child: this is a trace");
	logger->info("Child: this is a info");
	logger->debug("Child: this is a debug");
	logger->error("Child: this is an error");
	logger->fatal("Child: this is a fatal");
}

void logOutTest() {
	consoleOutputTest();
	fileOutputTest();
	rootLoggerTest();
}

TEST(logConfigTest, loadConfigTest1) {
	std::string config_file_path = base_path;
	if (backslash) {
		config_file_path += '\\';
	}
	else {
		config_file_path += '/';
	}
	config_file_path += "test-log4cpp-1.json";
	log4cpp::logger_manager::load_config(config_file_path);
	logOutTest();
}

TEST(logConfigTest, loadConfigTest2) {
	std::string config_file_path = base_path;
	if (backslash) {
		config_file_path += '\\';
	}
	else {
		config_file_path += '/';
	}
	config_file_path += "test-log4cpp-2.json";
	log4cpp::logger_manager::load_config(config_file_path);
	logOutTest();
}
