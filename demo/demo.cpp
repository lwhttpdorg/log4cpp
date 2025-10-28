#include <thread>

#include "log4cpp.hpp"

void thread_routine() {
	log4cpp::set_thread_name("child");
	std::shared_ptr<log4cpp::logger> log = log4cpp::layout_manager::get_layout("recordLayout");
	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->warn("this is an warning");
	log->error("this is an error");
	log->fatal("this is a fatal");
}

int main() {
	std::thread child(thread_routine);
	log4cpp::set_thread_name("main");
	std::shared_ptr<log4cpp::logger> log = log4cpp::layout_manager::get_layout("console_layout");

	log->trace("this is a trace");
	log->info("this is a info");
	log->debug("this is a debug");
	log->warn("this is an warning");
	log->error("this is an error");
	log->fatal("this is a fatal");
	child.join();
	return 0;
}
