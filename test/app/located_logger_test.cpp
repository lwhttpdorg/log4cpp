#include <format>      // for std::format
#include <memory>      // for std::shared_ptr
#include <string>      // for std::string
#include <string_view> // for std::string_view

#include <gtest/gtest.h> // for TEST, EXPECT_EQ

#include "log4cpp/log4cpp.hpp" // for located_logger, logger, log_level

namespace {
    class capture_logger: public log4cpp::logger {
    public:
        using logger::log;

        [[nodiscard]] std::string get_name() const override {
            return name_;
        }

        void set_name(const std::string &name) override {
            name_ = name;
        }

        [[nodiscard]] log4cpp::log_level get_level() const override {
            return level_;
        }

        void set_level(log4cpp::log_level level) override {
            level_ = level;
        }

        void log(log4cpp::log_level level, std::string_view msg) const override {
            captured_level = level;
            captured_msg = msg;
        }

        mutable log4cpp::log_level captured_level = log4cpp::log_level::TRACE;
        mutable std::string captured_msg;

    private:
        std::string name_ = "capture";
        log4cpp::log_level level_ = log4cpp::log_level::TRACE;
    };
} // namespace

TEST(located_logger_tests, info_prefixes_file_and_line) {
    const auto logger = std::make_shared<capture_logger>();
    const int expected_line = __LINE__ + 1;
    logger->at().info("hello {}", "world");

    EXPECT_EQ(logger->captured_level, log4cpp::log_level::INFO);
    EXPECT_EQ(logger->captured_msg, std::format("[{}:{}] hello world", __FILE__, expected_line));
}

TEST(located_logger_tests, error_supports_plain_message) {
    const auto logger = std::make_shared<capture_logger>();
    const int expected_line = __LINE__ + 1;
    logger->at().error("plain error");

    EXPECT_EQ(logger->captured_level, log4cpp::log_level::ERROR);
    EXPECT_EQ(logger->captured_msg, std::format("[{}:{}] plain error", __FILE__, expected_line));
}

TEST(located_logger_tests, explicit_location_can_be_supplied) {
    const auto logger = std::make_shared<capture_logger>();
    const log4cpp::located_logger located(*logger, log4cpp::log_location{"source.cpp", "test", 42});

    located.warn("value={}", 7);

    EXPECT_EQ(logger->captured_level, log4cpp::log_level::WARN);
    EXPECT_EQ(logger->captured_msg, "[source.cpp:42] value=7");
}
