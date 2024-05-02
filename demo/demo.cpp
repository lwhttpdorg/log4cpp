#include "log4cpp.hpp"

#ifdef _MSC_VER

#include <windows.h>

DWORD WINAPI thread_routine(LPVOID lpParam) {
	(void)lpParam;
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("Child: this is a trace");
	logger->info("Child: this is a info");
	logger->debug("Child: this is a debug");
	logger->warn("Child: this is an warning");
	logger->error("Child: this is an error");
	logger->fatal("Child: this is a fatal");

	return 0;
}

#endif

#ifdef __linux__

#include <pthread.h>

#include "log4cpp.hpp"

void *thread_routine(void *) {
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

#endif

int main() {
#ifdef _MSC_VER
	DWORD thread_id;
	HANDLE handle = CreateThread(nullptr, 0, thread_routine, nullptr, 0, &thread_id);
#endif
#ifdef __linux__
	pthread_t id;
	pthread_create(&id, nullptr, thread_routine, nullptr);
	pthread_setname_np(pthread_self(), "main");
#endif
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("Main: this is a trace");
	logger->info("Main: this is a info");
	logger->debug("Main: this is a debug");
	logger->warn("Main: this is an warning");
	logger->error("Main: this is an error");
	logger->fatal("Main: this is a fatal");

#ifdef _MSC_VER
	WaitForSingleObject(handle, INFINITE);
#endif
#ifdef __linux__
	pthread_join(id, nullptr);
#endif
	return 0;
}
