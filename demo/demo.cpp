#include <thread>

#include <log4cpp/log4cpp.hpp>

void thread_routine() {
    log4cpp::set_thread_name("child");
    const auto log = log4cpp::logger_manager::get_logger("aaa");
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
    const std::string config_file = "demo.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    std::thread child(thread_routine);
    log4cpp::set_thread_name("main");
    const auto log = log4cpp::logger_manager::get_logger("hello");

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
