#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <filesystem>

#ifdef __GNUC__

#endif

#include <gtest/gtest.h>

#include "common/log_utils.hpp"
#include "pattern/log_pattern.hpp"

#include "../include/log4cpp_test.h"

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

TEST(log_pattern_tests, full_format_test) {
    const std::string pattern = "${5NM}: ${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TH}] [${L}] -- ${W}";
    log4cpp::pattern::log_pattern::set_pattern(pattern);
    char actual[1024];
    tm now_tm{};
    unsigned short ms;
    log4cpp::log_level level = log4cpp::log_level::INFO;
    const char *LOGGER_NAME = "log_pattern_tests";
    log4cpp::common::get_time_now(now_tm, ms);
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), LOGGER_NAME, level, "hello");
    char expected[1024];
    char th_name[16];
    char thread_name[16];
    unsigned long tid = log4cpp::get_thread_name_id(th_name, sizeof(th_name));
    log4cpp::common::log4c_scnprintf(thread_name, sizeof(thread_name), "%08lu", tid);
    log4cpp::common::log4c_scnprintf(
        expected, sizeof(expected), "%-5.5s: %04d-%02d-%02d %02d:%02d:%02d:%03d [T%08s] [%-5s] -- %s\n", LOGGER_NAME,
        now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, ms,
        thread_name, log4cpp::level_to_string(level).c_str(), "hello");
    // The length of "2025-11-27 19:07:24:401" is 23
    size_t datetime_str_len = 23;
    // Verify the datetime part
    LOG4C_EXPECT_STRN_EQ(expected, actual, datetime_str_len, "Datetime part mismatch");
}

TEST(log_pattern_tests, year_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];

    /* short year(2 digit) */
    // The length of "25-01-07 " is 9
    size_t cmp_len = 9;
    std::string time_pattern = "${yy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%02d-%02d-%02d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year % 100, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Short year format mismatch");
    /* Full year(4 digit) */
    // The length of "2025-01-07 " is 11
    cmp_len = 11;
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Full year format mismatch");
}

TEST(log_pattern_tests, month_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];
    // The length of "2025-01-07 " is 11
    size_t cmp_len = 10;
    /* Month without leading zeros(one digit) */
    std::string time_pattern = "${yyyy}-${MM}-${d} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 9;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Month without leading zeros(one digit) format mismatch");
    /* Month without leading zeros(two digit) */
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 29;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Month without leading zeros(two digit) format mismatch");

    /* Month with leading zeros(one digit) */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    cmp_len = 11;
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 9;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Month with leading zeros(one digit) format mismatch");
    /* Month with leading zeros(two digit) */
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 29;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Month with leading zeros(two digit) format mismatch");
    /* Month Abbreviation */
    time_pattern = "${MMM} ${d}, ${yyyy} ${HH}:${mm}:${ss}:${ms}";
    for (int i = 0; i < 12; ++i) {
        strcpy(actual, time_pattern.c_str());
        now_tm.tm_mon = i;
        log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
        log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%s %02d, %04d %02d:%02d:%02d:%03d\n",
                                         log4cpp::pattern::MONTH_ABBR_NAME[now_tm.tm_mon], now_tm.tm_mday,
                                         now_tm.tm_year + 1900, now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, ms);
        LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Month Abbreviation format mismatch");
    }
}

TEST(log_pattern_tests, day_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];
    // The length of "2025-01-07 " is 11
    size_t cmp_len = 10;
    /* Day of the month without leading zeros(one digit) */
    std::string time_pattern = "${yyyy}-${MM}-${d} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 9;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len,
                         "Day of the month without leading zeros(one digit) format mismatch");
    /* Day of the month without leading zeros(two digit) */
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 29;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len,
                         "Day of the month without leading zeros(two digit) format mismatch");

    /* Day of the month with leading zeros(one digit) */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    cmp_len = 11;
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 9;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Day of the month with leading zeros(one digit) format mismatch");
    /* Day of the month with leading zeros(two digit) */
    strcpy(actual, time_pattern.c_str());
    now_tm.tm_mday = 29;
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d\n",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, cmp_len, "Day of the month with leading zeros(two digit) format mismatch");
}

TEST(log_pattern_tests, time_hour_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];

    /* 24-hour without leading zeros */
    std::string time_pattern = "${yyyy}-${MM}-${dd} ${H}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "24-hour without leading zeros format mismatch");
    /* 24-hour with leading zeros */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "24-hour with leading zeros format mismatch");
    /* 12-hour without leading zeros */
    time_pattern = "${yyyy}-${MM}-${dd} ${h}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    size_t len = log4cpp::common::log4c_scnprintf(
        expected, sizeof(expected), "%04d-%02d-%02d %d:%02d:%02d:%03d", now_tm.tm_year + 1900, now_tm.tm_mon + 1,
        now_tm.tm_mday, now_tm.tm_hour > 12 ? now_tm.tm_hour - 12 : now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, ms);
    if (now_tm.tm_hour >= 12) {
        log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " PM");
    }
    else {
        log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " AM");
    }
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "12-hour without leading zeros format mismatch");
    /* 12-hour with leading zeros */
    time_pattern = "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    len = log4cpp::common::log4c_scnprintf(
        expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d", now_tm.tm_year + 1900, now_tm.tm_mon + 1,
        now_tm.tm_mday, now_tm.tm_hour > 12 ? now_tm.tm_hour - 12 : now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, ms);
    if (now_tm.tm_hour >= 12) {
        log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " PM");
    }
    else {
        log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " AM");
    }
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "12-hour with leading zeros format mismatch");
    /* AM PM test */
    time_pattern = "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss}:${ms}";
    // 12:xx AM
    now_tm.tm_hour = 0;
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d AM",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "12:xx AM format mismatch");
    // 2:xx AM
    now_tm.tm_hour = 2;
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d AM",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "2:xx AM format mismatch");
    // 12:xx PM
    now_tm.tm_hour = 12;
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d PM",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "12:xx PM format mismatch");
    // 2:xx PM
    now_tm.tm_hour = 14;
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d PM",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour - 12,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "2:xx PM format mismatch");
}

TEST(log_pattern_tests, time_minutes_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];

    /* Minutes without leading zeros(one digit) */
    now_tm.tm_min = 9;
    std::string time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${m}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected),
                         "Minutes without leading zeros(one digit) format mismatch");
    /* Minutes with leading zeros(one digit) */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "Minutes with leading zeros(one digit) format mismatch");
    /* Minutes without leading zeros(two digit) */
    now_tm.tm_min = 59;
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${m}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected),
                         "Minutes without leading zeros(two digit) format mismatch");
    /* Minutes with leading zeros(two digit) */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "Minutes with leading zeros(two digit) format mismatch");
}

TEST(log_pattern_tests, time_seconds_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];

    /* Seconds without leading zeros(one digit) */
    now_tm.tm_sec = 9;
    std::string time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${s}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected),
                         "Seconds without leading zeros(one digit) format mismatch");

    /* Seconds with leading zeros(one digit) */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "Seconds with leading zeros(one digit) format mismatch");
    /* Seconds without leading zeros(two digit) */
    now_tm.tm_sec = 59;
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected),
                         "Seconds without leading zeros(two digit) format mismatch");
    /* Seconds with leading zeros(two digit) */
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "Seconds with leading zeros(two digit) format mismatch");
}

TEST(log_pattern_tests, time_milliseconds_format_test) {
    tm now_tm{};
    unsigned short ms;
    log4cpp::common::get_time_now(now_tm, ms);

    char actual[1024];
    char expected[1024];

    /* One digit milliseconds */
    ms = 9;
    std::string time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "One digit milliseconds format mismatch");
    /* Two digit milliseconds */
    ms = 59;
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "Two digit milliseconds format mismatch");
    /* Three digit milliseconds */
    ms = 199;
    time_pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms}";
    strcpy(actual, time_pattern.c_str());
    log4cpp::pattern::format_daytime(actual, sizeof(actual), time_pattern, now_tm, ms);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                     now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, now_tm.tm_hour,
                                     now_tm.tm_min, now_tm.tm_sec, ms);
    LOG4C_EXPECT_STRN_EQ(expected, actual, strlen(expected), "Three digit milliseconds format mismatch");
}

TEST(log_pattern_tests, thread_id_format_test) {
    const std::string pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TN}] [${L}] -- ${W}";
    log4cpp::pattern::log_pattern::set_pattern(pattern);
    char actual[1024];
    tm now_tm{};
    unsigned short ms;
    log4cpp::log_level level = log4cpp::log_level::INFO;
    log4cpp::common::get_time_now(now_tm, ms);

    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, "hello");
    char expected[1024];
    size_t len = log4cpp::common::log4c_scnprintf(expected, sizeof(expected), "%04d-%02d-%02d %02d:%02d:%02d:%03d",
                                                  now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                                                  now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, ms);
    size_t day_time = len;
    char th_name[16];
    // The name is empty, use "T+thread id"
    unsigned long tid = log4cpp::get_thread_name_id(th_name, sizeof(th_name));
    if ('\0' == th_name[0]) {
        len += log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " [T%08lu]", tid);
    }
    else {
        len += log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " [%-8.8s]", th_name);
    }
    log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), "hello");
    // The length of "2025-11-27 19:02:47:307" is 23
    size_t offset = 23;
    LOG4C_EXPECT_STR_EQ(actual + offset, expected + offset, "Thread id format mismatch");
    // Set thread name
    log4cpp::set_thread_name("test");
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, "hello");
    tid = log4cpp::get_thread_name_id(th_name, sizeof(th_name));
    len = day_time;
    if ('\0' == th_name[0]) {
        len += log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " [T%08lu]", tid);
    }
    else {
        len += log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " [%-8s]", th_name);
    }
    log4cpp::common::log4c_scnprintf(expected + len, sizeof(expected) - len, " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), "hello");
    /* "2025-11-27 19:07:24:401 [T140737353070464] [INFO ] -- hello"
     * The length before "[T140737353070464]" is 24
     */
    offset = 24;
    LOG4C_EXPECT_STR_EQ(actual + offset, expected + offset, "Thread name or id format mismatch");
}

TEST(log_pattern_tests, log_level_format_test) {
    const std::string pattern = "${yyyy}-${MM}-${dd} ${HH}:${mm}:${ss}:${ms} [${8TN}] [${L}] -- ${W}";
    log4cpp::pattern::log_pattern::set_pattern(pattern);
    char actual[1024];
    const char *message = "hello world!";
    // The length of " [INFO ] -- hello world!\n" is 29
    size_t cmp_len = 25;
    // FATAL
    log4cpp::log_level level = log4cpp::log_level::FATAL;
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, message);
    char expected[1024];
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), message);
    size_t offset = strlen(actual) - cmp_len;
    LOG4C_EXPECT_STRN_EQ(actual + offset, expected, cmp_len, "Log level FATAL format mismatch");
    // ERROR
    level = log4cpp::log_level::ERROR;
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, message);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), message);
    LOG4C_EXPECT_STRN_EQ(actual + offset, expected, cmp_len, "Log level ERROR format mismatch");
    // WARN
    level = log4cpp::log_level::WARN;
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, message);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), message);
    LOG4C_EXPECT_STRN_EQ(actual + offset, expected, cmp_len, "Log level WARN format mismatch");
    // INFO
    level = log4cpp::log_level::INFO;
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, message);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), message);
    LOG4C_EXPECT_STRN_EQ(actual + offset, expected, cmp_len, "Log level INFO format mismatch");
    // DEBUG
    level = log4cpp::log_level::DEBUG;
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, message);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), message);
    LOG4C_EXPECT_STRN_EQ(actual + offset, expected, cmp_len, "Log level DEBUG format mismatch");
    // TRACE
    level = log4cpp::log_level::TRACE;
    log4cpp::pattern::log_pattern::format(actual, sizeof(actual), "TEST", level, message);
    log4cpp::common::log4c_scnprintf(expected, sizeof(expected), " [%-5s] -- %s\n",
                                     log4cpp::level_to_string(level).c_str(), message);
    LOG4C_EXPECT_STRN_EQ(actual + offset, expected, cmp_len, "Log level TRACE format mismatch");
}
