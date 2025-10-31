#include <thread>
#include <unistd.h>

#include "log4cpp.hpp"

// void thread_routine() {
//	log4cpp::set_thread_name("child");
//	auto &log_mgr = log4cpp::logger_manager::instance();
//	std::shared_ptr<log4cpp::logger> log = log_mgr.get_logger("aaa");
//	for (int i = 0; i < 100; ++i) {
//		log->trace("this is a trace");
//		log->info("this is a info");
//		log->debug("this is a debug");
//		log->warn("this is an warning");
//		log->error("this is an error");
//		log->fatal("this is a fatal");
//		sleep(1);
//	}
// }

int main() {
	//	std::thread child(thread_routine);
	log4cpp::set_thread_name("main");
	auto &log_mgr = log4cpp::logger_manager::instance();
	log_mgr.enable_config_hot_loading();
	std::shared_ptr<log4cpp::logger> log = log_mgr.get_logger("bbb");

	for (int i = 0; i < 100; ++i) {
		log->trace("this is a trace");
		log->info("this is a info");
		log->debug("this is a debug");
		log->warn("this is an warning");
		log->error("this is an error");
		log->fatal("this is a fatal");
		sleep(1);
	}
	//	child.join();
	return 0;
}
