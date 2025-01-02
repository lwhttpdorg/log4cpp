#include <thread>

#include "log4cpp.hpp"

void set_thread_name(const char *name) {
#ifdef _PTHREAD_H
	pthread_setname_np(pthread_self(), name);
#elif __linux__
	prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("child"));
#endif
}

void thread_routine() {
	set_thread_name("child");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("this is a trace");
	logger->info("this is a info");
	logger->debug("this is a debug");
	logger->warn("this is an warning");
	logger->error("this is an error");
	logger->fatal("this is a fatal");
}

int main() {
	std::thread t(thread_routine);
	set_thread_name("main");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("this is a trace");
	logger->info("this is a info");
	logger->debug("this is a debug");
	logger->warn("this is an warning");
	logger->error("this is an error");
	logger->fatal("this is a fatal");
	t.join();
	return 0;
}
