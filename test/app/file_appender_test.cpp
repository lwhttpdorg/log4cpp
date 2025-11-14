#include <filesystem>
#include <thread>

#include <gtest/gtest.h>

#include "log4cpp/log4cpp.hpp"

class TestEnvironment: public testing::Environment {
public:
    explicit TestEnvironment(const std::string &cur_path) {
        size_t end = cur_path.find_last_of('\\');
        if (end == std::string::npos) {
            end = cur_path.find_last_of('/');
        }
        const std::string work_dir = cur_path.substr(0, end);
        std::filesystem::current_path(work_dir);
    }
};

int main(int argc, char **argv) {
    const std::string cur_path = argv[0];
    testing::InitGoogleTest(&argc, argv);
    AddGlobalTestEnvironment(new TestEnvironment(cur_path));
    return RUN_ALL_TESTS();
}

void info_logger() {
    const std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger("info_logger");
    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");
}

void warn_logger() {
    const std::shared_ptr<log4cpp::log::logger> log = log4cpp::logger_manager::get_logger("warn_logger");
    log->trace("this is a trace");
    log->debug("this is a debug");
    log->info("this is a info");
    log->warn("this is an warning");
    log->error("this is an error");
    log->fatal("this is a fatal");
}

void load_configuration() {
    const std::string config_file = "file_appender_test.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
}

TEST(file_appender_test, single_thread_test) {
    load_configuration();
    info_logger();
    warn_logger();
}

TEST(file_appender_test, multithread_test) {
    load_configuration();
    std::thread info_logger_thread(info_logger);
    std::thread warn_logger_thread(warn_logger);
    info_logger_thread.join();
    warn_logger_thread.join();
}
