#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

#include <gtest/gtest.h>

#include "log4cpp/log4cpp.hpp"

std::atomic<int> config_epoch(0);

void write_config_file(const std::string &file_path, const std::string &json_content) {
    std::ofstream ofs(file_path);
    ofs << json_content;
    ofs.close();
}

const char *HOT_RELOAD_CONFIG = "config_hot_reload_test.json";
const char *CONFIG_FILE_v1 = "config_hot_reload_test_v1.json";
const char *CONFIG_FILE_v2 = "config_hot_reload_test_v2.json";
const char *LOG_V1_FILE = "log_v1.log";
const char *LOG_V2_FILE = "log_v2.log";

int main(int argc, char **argv) {
    const std::string cur_path = argv[0];
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

class log4cpp_config_hot_reload_test: public ::testing::Test {
protected:
    void SetUp() override {
        std::remove(HOT_RELOAD_CONFIG);
        // Delete log files
        std::remove(LOG_V1_FILE);
        std::remove(LOG_V2_FILE);
        config_epoch.store(0);
    }

    void TearDown() override {
        std::remove(HOT_RELOAD_CONFIG);
        // Delete log files
        std::remove(LOG_V1_FILE);
        std::remove(LOG_V2_FILE);
    }
};

void write_test_config_file(const std::string &src_file, const std::string &dst_file) {
    // Open source file in binary mode
    std::ifstream src(src_file, std::ios::binary);
    if (!src) {
        throw std::runtime_error("Failed to open source file: " + src_file);
    }

    // Open destination file in binary mode, overwrite existing content
    std::ofstream dst(dst_file, std::ios::binary | std::ios::trunc);
    if (!dst) {
        throw std::runtime_error("Failed to open destination file: " + dst_file);
    }

    // Copy content from source to destination
    dst << src.rdbuf();
}

std::atomic<bool> finished(false);

void worker_thread_logging_routine_type1(int id) {
    log4cpp::set_thread_name(("worker_type1_" + std::to_string(id)).c_str());
    const auto logger = log4cpp::logger_manager::get_logger("aaa");
    while (!finished.load()) {
        logger->trace("Type 1 Thread %d: This is a TRACE log message.", id);
        logger->debug("Type 1 Thread %d: This is a DEBUG log message.", id);
        logger->info("Type 1 Thread %d: This is an INFO log message.", id);
        logger->warn("Type 1 Thread %d: This is a WARN log message.", id);
        logger->error("Type 1 Thread %d: This is an ERROR log message.", id);
        logger->fatal("Type 1 Thread %d: This is a FATAL log message.", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void worker_thread_logging_routine_type2(int id) {
    log4cpp::set_thread_name(("worker_type2_" + std::to_string(id)).c_str());
    const auto logger = log4cpp::logger_manager::get_logger("bbb");
    while (config_epoch.load() == 0 && !finished.load()) {
        logger->trace("Type 2 Thread %d: This is a TRACE log message.", id);
        logger->debug("Type 2 Thread %d: This is a DEBUG log message.", id);
        logger->info("Type 2 Thread %d: This is an INFO log message.", id);
        logger->warn("Type 2 Thread %d: This is a WARN log message.", id);
        logger->error("Type 2 Thread %d: This is an ERROR log message.", id);
        logger->fatal("Type 2 Thread %d: This is a FATAL log message.", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Switch to V2 logging
    while (config_epoch.load() == 2 && !finished.load()) {
        logger->trace("Type 2 Thread %d: This is a TRACE log message.", id);
        logger->debug("Type 2 Thread %d: This is a DEBUG log message.", id);
        logger->info("Type 2 Thread %d: This is an INFO log message.", id);
        logger->warn("Type 2 Thread %d: This is a WARN log message.", id);
        logger->error("Type 2 Thread %d: This is an ERROR log message.", id);
        logger->fatal("Type 2 Thread %d: This is a FATAL log message.", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void worker_thread_logging_routine_type3(int id) {
    log4cpp::set_thread_name(("worker_type3_" + std::to_string(id)).c_str());
    while (!finished.load()) {
        const auto logger = log4cpp::logger_manager::get_logger("aaa");
        logger->error("Type 3 Thread %d: This is an ERROR log message.", id);
        logger->fatal("Type 3 Thread %d: This is a FATAL log message.", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// --- GTest Test Case ---

TEST_F(log4cpp_config_hot_reload_test, multi_thread_signal_hotloading) {
    // Read CONFIG_FILE_v1 and write it to HOT_RELOAD_CONFIG.
    write_test_config_file(CONFIG_FILE_v1, HOT_RELOAD_CONFIG);

    // Load initial config and enable hot-loading
    auto &manager = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(manager.load_config(HOT_RELOAD_CONFIG));
    log4cpp::supervisor::enable_config_hot_loading(SIGHUP);

    std::vector<std::thread> worker_threads;

    for (int i = 0; i < 2; ++i) {
        worker_threads.emplace_back(worker_thread_logging_routine_type1, i);
    }

    for (int i = 0; i < 2; ++i) {
        worker_threads.emplace_back(worker_thread_logging_routine_type2, i);
    }

    for (int i = 0; i < 2; ++i) {
        worker_threads.emplace_back(worker_thread_logging_routine_type3, i);
    }

    // Main thread logging (V1 Phase)
    log4cpp::set_thread_name("main_thread");

    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Read CONFIG_FILE_v2 and write it to HOT_RELOAD_CONFIG
    write_test_config_file(CONFIG_FILE_v2, HOT_RELOAD_CONFIG);

    // Send SIGHUP signal to trigger hot-loading
    pid_t pid = getpid();
    int kill_result = kill(pid, SIGHUP);
    ASSERT_EQ(0, kill_result) << "Sending SIGHUP failed: " << strerror(errno);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    config_epoch.store(2);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    config_epoch.store(3);
    finished.store(true);

    // Wait for all worker threads to finish
    for (auto &t: worker_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // Read log_v2.log to verify that logs pattern is mathched with "${yy}-${MMM}-${dd} ${h}:${mm}:${ss}:${ms}"
    std::ifstream log_file("log_v2.log");
    ASSERT_TRUE(log_file.is_open()) << "Failed to open log_v2.log";
    std::string line;
    bool pattern_matched = false;
    // log pattern, e.g., "24-Jun-05 14:23:45:678"
    std::regex pattern(R"(\d{2}-[A-Za-z]{3}-\d{2}\s\d{1,2}:\d{2}:\d{2}:\d{3}\s(AM|PM))");
    while (std::getline(log_file, line)) {
        if (std::regex_search(line, pattern)) {
            pattern_matched = true;
            break;
        }
    }
    // Use EXPECT_TRUE to continue the test even if pattern not matched
    EXPECT_TRUE(pattern_matched) << "Log pattern not matched in " << LOG_V2_FILE;

    // And log level of "aaa"(start with "aaa") is less than or equal to ERROR(log4cpp::log_level::ERROR), ignore other
    // loggers(such as "root")
    log_file.clear();
    log_file.seekg(0, std::ios::beg);
    while (std::getline(log_file, line)) {
// start with "aaa"
#if __cplusplus >= 202002L
        if (line.starts_with("aaa")) {
#else
        if (line.substr(0, 3) == "aaa") {
#endif
            // regex_search for log level \\[(FATAL|ERROR|WARN|INFO|DEBUG|TRACE)\\s*\\]
            std::smatch match;
            std::regex level_pattern(R"(\[(FATAL|ERROR|WARN|INFO|DEBUG|TRACE)\s*\])");
            if (std::regex_search(line, match, level_pattern)) {
                std::string level_str = match[1];
                // log_level level_from_string(const std::string &s)
                auto level = log4cpp::level_from_string(level_str);
                ASSERT_LE(level, log4cpp::log_level::ERROR)
                    << "Log level of 'aaa' logger is higher than ERROR: " << line;
            }
        }
    }
    // Close the log file
    log_file.close();
}
