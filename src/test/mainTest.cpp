#include <thread>
#include "gtest/gtest.h"

#include "log4cpp.hpp"


int main(int argc, const char **argv) {
	::testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}

TEST(logConfigTest, loadConfig) {
	log4cpp::logger_manager::load_config("./log4cpp.json");
}

TEST(logOutputTest, consoleOutput) {
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("This is a trace: %s:%d", __func__, __LINE__);
	logger->info("This is a info: %s:%d", __func__, __LINE__);
	logger->debug("This is a debug: %s:%d", __func__, __LINE__);
	logger->warn("This is a warning: %s:%d", __func__, __LINE__);
	logger->error("This is a error: %s:%d", __func__, __LINE__);
	logger->fatal("This is a fatal: %s:%d", __func__, __LINE__);
}

TEST(logOutputTest, fileOutput) {
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("This is a trace: %s:%d", __func__, __LINE__);
	logger->info("This is a info: %s:%d", __func__, __LINE__);
	logger->debug("This is a debug: %s:%d", __func__, __LINE__);
	logger->error("This is a error: %s:%d", __func__, __LINE__);
	logger->fatal("This is a fatal: %s:%d", __func__, __LINE__);
}
