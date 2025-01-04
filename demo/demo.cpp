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
	std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("recordLayout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->warn("this is an warning");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

int main() {
	std::thread t(thread_routine);
	set_thread_name("main");
	std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("consoleLayout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->warn("this is an warning");
	log->error("this is an error");
	log->fatal("this is a fatal");
	t.join();
	return 0;
}
