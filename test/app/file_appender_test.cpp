#include <cctype>     // for std::isdigit
#include <chrono>     // for std::chrono
#include <filesystem> // for std::filesystem
#include <fstream>    // for std::ofstream
#include <optional>   // for std::optional
#include <string>     // for std::string
#include <vector>     // for std::vector

#include <gtest/gtest.h> // for TEST, EXPECT_*

#include "appender/file_appender.hpp"               // for file_appender
#include "appender/rolling_file_appender_rules.hpp" // for rolling_file_appender_rules

namespace {
    void remove_rolling_test_logs(const std::string &prefix = "rolling_file_appender_test.log") {
        const std::filesystem::path log_dir("log");
        if (!std::filesystem::exists(log_dir)) {
            return;
        }
        for (const auto &entry: std::filesystem::directory_iterator(log_dir)) {
            const std::string filename = entry.path().filename().string();
            if (filename == prefix || filename.starts_with(prefix + ".")) {
                std::filesystem::remove(entry.path());
            }
        }
    }

    std::vector<std::string> rolling_archive_names(const std::string &prefix) {
        const std::filesystem::path log_dir("log");
        if (!std::filesystem::exists(log_dir)) {
            return {};
        }

        std::vector<std::string> archives;
        for (const auto &entry: std::filesystem::directory_iterator(log_dir)) {
            const std::string filename = entry.path().filename().string();
            if (filename.starts_with(prefix + ".")) {
                archives.emplace_back(filename);
            }
        }
        return archives;
    }

    size_t count_rolling_archives(const std::string &prefix) {
        return rolling_archive_names(prefix).size();
    }

    bool is_digits(const std::string &value) {
        if (value.empty()) {
            return false;
        }
        for (const char ch: value) {
            if (std::isdigit(static_cast<unsigned char>(ch)) == 0) {
                return false;
            }
        }
        return true;
    }

    bool is_size_time_archive_name(const std::string &name, const std::string &prefix) {
        const std::string expected_prefix = prefix + ".";
        if (!name.starts_with(expected_prefix)) {
            return false;
        }

        const std::string suffix = name.substr(expected_prefix.size());
        const size_t dot_pos = suffix.find('.');
        if (dot_pos == std::string::npos) {
            return false;
        }

        const std::string bucket = suffix.substr(0, dot_pos);
        const std::string index = suffix.substr(dot_pos + 1);
        return (bucket.size() == 8 || bucket.size() == 10) && is_digits(bucket) && is_digits(index);
    }

    void write_messages(log4cpp::appender::file_appender &appender, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            const std::string msg = "rolling file appender direct test message " + std::to_string(i) + "\n";
            appender.log(msg.data(), msg.size());
        }
    }
} // namespace

TEST(file_appender_test, rolling_policy_size_decision_test) {
    const std::optional<log4cpp::config::rolling_policy> rolling = log4cpp::config::rolling_policy{
        .policy = log4cpp::config::rolling_policy_type::SIZE,
        .file_size = 10,
    };

    EXPECT_FALSE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 0, 20, "", ""));
    EXPECT_FALSE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 5, 5, "", ""));
    EXPECT_TRUE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 10, 1, "", ""));
    EXPECT_TRUE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 9, 2, "", ""));
}

TEST(file_appender_test, rolling_policy_time_decision_test) {
    const std::optional<log4cpp::config::rolling_policy> rolling = log4cpp::config::rolling_policy{
        .policy = log4cpp::config::rolling_policy_type::TIME,
        .interval = log4cpp::config::rolling_time_interval::DAY,
    };

    EXPECT_TRUE(log4cpp::appender::rolling_file_appender_rules::uses_time_bucket(rolling));
    EXPECT_FALSE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 0, 1, "20260705", "20260705"));
    EXPECT_TRUE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 0, 1, "20260705", "20260706"));
}

TEST(file_appender_test, rolling_policy_size_time_decision_test) {
    const std::optional<log4cpp::config::rolling_policy> rolling = log4cpp::config::rolling_policy{
        .policy = log4cpp::config::rolling_policy_type::SIZE_TIME,
        .file_size = 10,
        .interval = log4cpp::config::rolling_time_interval::DAY,
    };

    EXPECT_TRUE(log4cpp::appender::rolling_file_appender_rules::uses_time_bucket(rolling));
    EXPECT_FALSE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 5, 1, "20260705", "20260706"));
    EXPECT_TRUE(log4cpp::appender::rolling_file_appender_rules::should_roll(rolling, 9, 2, "20260705", "20260706"));
}

TEST(file_appender_test, rolling_policy_archive_prefix_test) {
    const std::optional<log4cpp::config::rolling_policy> size_rolling = log4cpp::config::rolling_policy{
        .policy = log4cpp::config::rolling_policy_type::SIZE,
        .file_size = 10,
    };
    const std::optional<log4cpp::config::rolling_policy> time_rolling = log4cpp::config::rolling_policy{
        .policy = log4cpp::config::rolling_policy_type::TIME,
        .interval = log4cpp::config::rolling_time_interval::DAY,
    };
    const std::optional<log4cpp::config::rolling_policy> size_time_rolling = log4cpp::config::rolling_policy{
        .policy = log4cpp::config::rolling_policy_type::SIZE_TIME,
        .file_size = 10,
        .interval = log4cpp::config::rolling_time_interval::DAY,
    };

    EXPECT_EQ(log4cpp::appender::rolling_file_appender_rules::archive_prefix(size_rolling, "app.log", "", ""),
              "app.log");
    EXPECT_EQ(
        log4cpp::appender::rolling_file_appender_rules::archive_prefix(time_rolling, "app.log", "20260705", "20260706"),
        "app.log.20260705");
    EXPECT_EQ(log4cpp::appender::rolling_file_appender_rules::archive_prefix(size_time_rolling, "app.log", "20260705",
                                                                             "20260706"),
              "app.log.20260705");
}

TEST(file_appender_test, none_rolling_policy_test) {
    constexpr const char *prefix = "rolling_none_file_appender_test.log";
    remove_rolling_test_logs(prefix);

    log4cpp::config::file_appender cfg{
        .file_path = "log/rolling_none_file_appender_test.log",
        .rolling =
            log4cpp::config::rolling_policy{
                .policy = log4cpp::config::rolling_policy_type::NONE,
                .file_size = 128,
                .file_count = 2,
                .max_history = std::chrono::hours(24),
            },
    };

    log4cpp::appender::file_appender appender(cfg);
    write_messages(appender, 20);

    EXPECT_TRUE(std::filesystem::exists("log/rolling_none_file_appender_test.log"));
    EXPECT_EQ(count_rolling_archives(prefix), 0);
}

TEST(file_appender_test, on_start_rolling_policy_test) {
    constexpr const char *prefix = "rolling_on_start_file_appender_test.log";
    remove_rolling_test_logs(prefix);

    std::filesystem::create_directories("log");
    {
        std::ofstream ofs("log/rolling_on_start_file_appender_test.log");
        ofs << "previous process log\n";
    }

    log4cpp::config::file_appender cfg{
        .file_path = "log/rolling_on_start_file_appender_test.log",
        .rolling =
            log4cpp::config::rolling_policy{
                .policy = log4cpp::config::rolling_policy_type::ON_START,
                .file_count = 2,
                .max_history = std::chrono::hours(24),
            },
    };

    log4cpp::appender::file_appender appender(cfg);

    EXPECT_TRUE(std::filesystem::exists("log/rolling_on_start_file_appender_test.log"));
    EXPECT_EQ(count_rolling_archives(prefix), 1);
    EXPECT_TRUE(std::filesystem::exists("log/rolling_on_start_file_appender_test.log.1"));
}

TEST(file_appender_test, size_time_rolling_policy_test) {
    constexpr const char *prefix = "rolling_size_time_file_appender_test.log";
    remove_rolling_test_logs(prefix);

    log4cpp::config::file_appender cfg{
        .file_path = "log/rolling_size_time_file_appender_test.log",
        .rolling =
            log4cpp::config::rolling_policy{
                .policy = log4cpp::config::rolling_policy_type::SIZE_TIME,
                .file_size = 128,
                .interval = log4cpp::config::rolling_time_interval::DAY,
                .file_count = 2,
                .max_history = std::chrono::hours(24),
            },
    };

    log4cpp::appender::file_appender appender(cfg);
    write_messages(appender, 20);

    const size_t archive_count = count_rolling_archives(prefix);
    EXPECT_GT(archive_count, 0);
    EXPECT_LE(archive_count, 2);

    for (const auto &name: rolling_archive_names(prefix)) {
        EXPECT_TRUE(is_size_time_archive_name(name, prefix)) << name;
    }
}
