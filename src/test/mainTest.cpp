#include <thread>
#include "gtest/gtest.h"

#include "log4cpp.hpp"


int main(int argc, const char **argv)
{
	::testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}

TEST(logConfigTest, loadConfig)
{
	log4cpp::logger_manager::load_config("./log4cpp.json");
}

TEST(logOutputTest, consoleOutput)
{
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("Child: this is a trace 0x%x", pthread_self());
	logger->info("Child: this is a info 0x%x", pthread_self());
	logger->debug("Child: this is a debug 0x%x", pthread_self());
	logger->error("Child: this is an error 0x%x", pthread_self());
	logger->fatal("Child: this is a fatal 0x%x", pthread_self());
}

TEST(logOutputTest, fileOutput)
{
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("Child: this is a trace 0x%x", pthread_self());
	logger->info("Child: this is a info 0x%x", pthread_self());
	logger->debug("Child: this is a debug 0x%x", pthread_self());
	logger->error("Child: this is an error 0x%x", pthread_self());
	logger->fatal("Child: this is a fatal 0x%x", pthread_self());
}
