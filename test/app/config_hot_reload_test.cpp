#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <sys/types.h>
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

class log4cpp_config_hot_reload_test: public ::testing::Test {
protected:
    void SetUp() override {
        std::remove(HOT_RELOAD_CONFIG);
        config_epoch.store(0);
    }

    void TearDown() override {
        std::remove(HOT_RELOAD_CONFIG);
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
    auto logger = log4cpp::logger_manager::get_logger("aaa");
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
    auto logger = log4cpp::logger_manager::get_logger("bbb");
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
        auto logger = log4cpp::logger_manager::get_logger("aaa");
        logger->error("Type 1 Thread %d: This is an ERROR log message.", id);
        logger->fatal("Type 1 Thread %d: This is a FATAL log message.", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// --- GTest Test Case ---

TEST_F(log4cpp_config_hot_reload_test, multi_thread_signal_hotloading) {
    // 读取CONFIG_FILE_v1, 将其写入到HOT_RELOAD_CONFIG
    write_test_config_file(CONFIG_FILE_v1, HOT_RELOAD_CONFIG);

    // Load initial config and enable hot-loading
    auto &manager = log4cpp::supervisor::get_logger_manager();
    manager.load_config(HOT_RELOAD_CONFIG);
    log4cpp::supervisor::enable_config_hot_loading();

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

    // 读取CONFIG_FILE_v2, 将其写入到HOT_RELOAD_CONFIG
    write_test_config_file(CONFIG_FILE_v2, HOT_RELOAD_CONFIG);

    // Send SIGUSR2 signal to trigger hot-loading
    pid_t pid = getpid();
    int kill_result = kill(pid, SIGUSR2);
    ASSERT_EQ(0, kill_result) << "Sending SIGUSR2 failed: " << strerror(errno);

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
}
