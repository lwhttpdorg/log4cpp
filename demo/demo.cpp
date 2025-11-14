#include <thread>

#include <log4cpp/log4cpp.hpp>

void thread_routine() {
    log4cpp::set_thread_name("child");
    auto log = log4cpp::logger_manager::get_logger("aaa");
    for (int i = 0; i < 100; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is a info");
        log->warn("this is an warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
}

int main() {
    log4cpp::supervisor::enable_config_hot_loading();
    std::thread child(thread_routine);
    log4cpp::set_thread_name("main");
    auto log = log4cpp::logger_manager::get_logger("hello");

    for (int i = 0; i < 100; ++i) {
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is a info");
        log->warn("this is an warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }
    child.join();
    return 0;
}
