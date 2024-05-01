#include "log4cpp.hpp"

#ifdef _MSC_VER

#include <windows.h>

DWORD WINAPI thread_routine(LPVOID lpParam) {
	(void)lpParam;
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("This is a trace: %s:%d", __func__, __LINE__);
	logger->info("This is a info: %s:%d", __func__, __LINE__);
	logger->debug("This is a debug: %s:%d", __func__, __LINE__);
	logger->error("This is a error: %s:%d", __func__, __LINE__);
	logger->fatal("This is a fatal: %s:%d", __func__, __LINE__);
	return 0;
}

#endif

#ifdef __linux__

#include <pthread.h>

void *thread_routine(void *) {
	pthread_setname_np(pthread_self(), "child_thread_456");
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("recordLogger");
	logger->trace("This is a trace: %s:%d", __func__, __LINE__);
	logger->info("This is a info: %s:%d", __func__, __LINE__);
	logger->debug("This is a debug: %s:%d", __func__, __LINE__);
	logger->error("This is a error: %s:%d", __func__, __LINE__);
	logger->fatal("This is a fatal: %s:%d", __func__, __LINE__);
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
	pthread_setname_np(pthread_self(), "main_thread_123");
#endif
	std::shared_ptr<log4cpp::logger> logger = log4cpp::logger_manager::get_logger("consoleLogger");
	logger->trace("This is a trace: %s:%d", __func__, __LINE__);
	logger->info("This is a info: %s:%d", __func__, __LINE__);
	logger->debug("This is a debug: %s:%d", __func__, __LINE__);
	logger->warn("This is a warning: %s:%d", __func__, __LINE__);
	logger->error("This is a error: %s:%d", __func__, __LINE__);
	logger->fatal("This is a fatal: %s:%d", __func__, __LINE__);

#ifdef _MSC_VER
	WaitForSingleObject(handle, INFINITE);
#endif
#ifdef __linux__
	pthread_join(id, nullptr);
#endif
	return 0;
}
