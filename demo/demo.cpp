#include <pthread.h>

#include "log4cpp.hpp"

void *child_thread_routine(void *) {
	pthread_setname_np(pthread_self(), "child");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("Child: this is a trace 0x%x", pthread_self());
	logger->info("Child: this is a info 0x%x", pthread_self());
	logger->debug("Child: this is a debug 0x%x", pthread_self());
	logger->warn("Child: this is an warning 0x%x", pthread_self());
	logger->error("Child: this is an error 0x%x", pthread_self());
	logger->fatal("Child: this is a fatal 0x%x", pthread_self());
	return nullptr;
}

int main() {
	pthread_t child_tid;
	pthread_create(&child_tid, nullptr, child_thread_routine, nullptr);
	pthread_setname_np(pthread_self(), "main");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("Main: this is a trace 0x%x", pthread_self());
	logger->info("Main: this is a info 0x%x", pthread_self());
	logger->debug("Main: this is a debug 0x%x", pthread_self());
	logger->warn("Main: this is an warning 0x%x", pthread_self());
	logger->error("Main: this is an error 0x%x", pthread_self());
	logger->fatal("Main: this is a fatal 0x%x", pthread_self());

	pthread_join(child_tid, nullptr);

	return 0;
}
