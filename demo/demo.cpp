#include <thread>

#include "log4cpp.hpp"

void thread_routine() {
	log4cpp::set_thread_name("child");
	// std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("recordLayout");
	// for (int i = 0; i < 100; i++) {
	// 	log->trace("this is a trace");
	// 	log->info("this is a info");
	// 	log->debug("this is a debug");
	// 	log->warn("this is an warning");
	// 	log->error("this is an error");
	// 	log->fatal("this is a fatal");
	// }
}

int main() {
	std::thread t(thread_routine);
	log4cpp::set_thread_name("main");
	std::shared_ptr<log4cpp::layout> log = log4cpp::layout_manager::get_layout("console_layout");
	for (int i = 0; i < 10; ++i) {
		log->trace("this is a trace");
		log->info("this is a info");
		log->debug("this is a debug");
		log->warn("this is an warning");
		log->error("this is an error");
		log->fatal("this is a fatal");
	}
	t.join();
	return 0;
}
