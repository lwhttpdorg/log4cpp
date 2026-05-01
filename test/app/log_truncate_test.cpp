#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include <log4cpp/log4cpp.hpp>
#include "pattern/log_pattern.hpp"

TEST(log_truncate_tests, pattern_format_does_not_overflow_on_long_message) {
    char buf[2048];
    // Construct a message far exceeding LOG_LINE_MAX (512)
    const std::string long_msg(800, 'X');

    log4cpp::pattern::log_pattern formatter;
    const size_t used_len =
        formatter.format(buf, sizeof(buf), "truncate_test", log4cpp::log_level::INFO, long_msg.c_str());

    // used_len must fit inside the provided buffer
    EXPECT_LT(used_len, sizeof(buf));
    // Output must be null-terminated and end with a newline
    EXPECT_EQ(buf[used_len - 1], '\n');
    EXPECT_EQ(buf[used_len], '\0');
}

TEST(log_truncate_tests, message_content_is_truncated_to_log_line_max) {
    char buf[2048];
    // The raw message is 600 'A's; after formatting it should be truncated
    // to at most LOG_LINE_MAX - 1 characters (vsnprintf reserves one for '\0')
    const std::string long_msg(600, 'A');

    log4cpp::pattern::log_pattern formatter;
    formatter.format(buf, sizeof(buf), "test", log4cpp::log_level::WARN, long_msg.c_str());

    // The formatted buffer should not contain the full 600 'A's
    const char *msg_start = std::strstr(buf, " -- ");
    ASSERT_NE(msg_start, nullptr);
    msg_start += 4; // skip " -- "

    size_t msg_len = std::strlen(msg_start);
    // Remove trailing '\n' for the count
    if (msg_len > 0 && msg_start[msg_len - 1] == '\n') {
        --msg_len;
    }

    EXPECT_LE(msg_len, log4cpp::LOG_LINE_MAX - 1) << "Message portion should be truncated to fit LOG_LINE_MAX";
}
