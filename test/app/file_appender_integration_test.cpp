#include <algorithm>  // for std::all_of
#include <cctype>     // for std::isdigit
#include <filesystem> // for std::filesystem
#include <memory>     // for std::shared_ptr
#include <string>     // for std::string
#include <thread>     // for std::thread

#include <gtest/gtest.h> // for TEST, ASSERT_NO_THROW

#include "log4cpp/log4cpp.hpp" // for logger, logger_manager

namespace {
    void info_logger() {
        const std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger("aaa");
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is an info");
        log->warn("this is a warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }

    void warn_logger() {
        const std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger("bbb");
        log->trace("this is a trace");
        log->debug("this is a debug");
        log->info("this is an info");
        log->warn("this is a warning");
        log->error("this is an error");
        log->fatal("this is a fatal");
    }

    void load_configuration() {
        const std::string config_file = "test_file_appender.json";
        auto &log_mgr = log4cpp::supervisor::get_logger_manager();
        ASSERT_NO_THROW(log_mgr.load_config(config_file));
    }
} // namespace

TEST(file_appender_integration_test, single_thread_test) {
    load_configuration();
    info_logger();
    warn_logger();
}

TEST(file_appender_integration_test, multithread_test) {
    load_configuration();
    std::thread info_logger_thread(info_logger);
    std::thread warn_logger_thread(warn_logger);
    info_logger_thread.join();
    warn_logger_thread.join();
}

TEST(file_appender_integration_test, size_rolling_test) {
    const std::filesystem::path log_dir("log");
    if (std::filesystem::exists(log_dir)) {
        for (const auto &entry: std::filesystem::directory_iterator(log_dir)) {
            const std::string filename = entry.path().filename().string();
            if (filename == "rolling_file_appender_test.log"
                || filename.starts_with("rolling_file_appender_test.log.")) {
                std::filesystem::remove(entry.path());
            }
        }
    }

    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config("test_rolling_file_appender.json"));

    const std::shared_ptr<log4cpp::logger> log = log4cpp::logger_manager::get_logger("rolling");
    for (int i = 0; i < 20; ++i) {
        log->info("rolling file appender test message {}", i);
    }

    ASSERT_TRUE(std::filesystem::exists(log_dir / "rolling_file_appender_test.log"));

    size_t archive_count = 0;
    for (const auto &entry: std::filesystem::directory_iterator(log_dir)) {
        const std::string filename = entry.path().filename().string();
        if (filename.starts_with("rolling_file_appender_test.log.")) {
            ++archive_count;
            const std::string suffix = filename.substr(std::string("rolling_file_appender_test.log.").size());
            EXPECT_FALSE(suffix.empty());
            EXPECT_TRUE(std::all_of(suffix.begin(), suffix.end(), [](const char ch) {
                return std::isdigit(static_cast<unsigned char>(ch)) != 0;
            })) << filename;
        }
    }
    EXPECT_GT(archive_count, 0);
    EXPECT_LE(archive_count, 2);
}
