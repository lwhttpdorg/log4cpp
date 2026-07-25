#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "appender/log_appender.hpp"
#include "log4cpp/logger.hpp"
#include "logger/real_logger.hpp"

namespace {

    class recording_appender final: public log4cpp::appender::log_appender {
    public:
        void log(const char *msg, size_t msg_len) override {
            messages.emplace_back(msg, msg_len);
        }

        std::vector<std::string> messages;
    };

    class recording_logger final: public log4cpp::logger {
    public:
        recording_logger(std::string name, log4cpp::log_level level) : name_(std::move(name)), level_(level) {
        }

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
            levels.push_back(level);
            messages.emplace_back(msg);
        }

        mutable std::vector<log4cpp::log_level> levels;
        mutable std::vector<std::string> messages;

    private:
        std::string name_;
        log4cpp::log_level level_;
    };

    TEST(real_logger_test, default_constructor_uses_warn_level) {
        log4cpp::real_logger logger;

        EXPECT_TRUE(logger.get_name().empty());
        EXPECT_EQ(logger.get_level(), log4cpp::log_level::WARN);

        logger.set_name("renamed");
        logger.set_level(log4cpp::log_level::DEBUG);
        EXPECT_EQ(logger.get_name(), "renamed");
        EXPECT_EQ(logger.get_level(), log4cpp::log_level::DEBUG);
    }

    TEST(real_logger_test, filters_messages_and_deduplicates_appenders) {
        log4cpp::real_logger logger("core", log4cpp::log_level::WARN, "${NM}|${L}|${msg}");
        const auto appender = std::make_shared<recording_appender>();

        logger.add_appender(appender);
        logger.add_appender(appender);
        logger.info("hidden");
        logger.error("visible");

        ASSERT_EQ(appender->messages.size(), 1U);
        EXPECT_EQ(appender->messages.front(), "core  |ERROR|visible\n");
    }

    TEST(real_logger_test, supports_copy_and_move_operations) {
        const auto appender = std::make_shared<recording_appender>();
        log4cpp::real_logger original("original", log4cpp::log_level::TRACE, "${msg}");
        original.add_appender(appender);

        log4cpp::real_logger copied(original);
        copied.info("copy-constructed");

        log4cpp::real_logger copy_assigned;
        copy_assigned = original;
        copy_assigned.warn("copy-assigned");
        copy_assigned = copy_assigned;

        log4cpp::real_logger moved(std::move(copied));
        moved.error("move-constructed");

        log4cpp::real_logger move_assigned;
        move_assigned = std::move(copy_assigned);
        move_assigned.fatal("move-assigned");
        move_assigned = std::move(move_assigned);

        ASSERT_EQ(appender->messages.size(), 4U);
        EXPECT_EQ(appender->messages[0], "copy-constructed\n");
        EXPECT_EQ(appender->messages[1], "copy-assigned\n");
        EXPECT_EQ(appender->messages[2], "move-constructed\n");
        EXPECT_EQ(appender->messages[3], "move-assigned\n");
    }

    TEST(logger_proxy_test, rejects_null_target) {
        EXPECT_THROW(log4cpp::logger_proxy(nullptr), std::invalid_argument);
    }

    TEST(logger_proxy_test, delegates_properties_and_messages) {
        const auto target = std::make_shared<recording_logger>("before", log4cpp::log_level::INFO);
        log4cpp::logger_proxy proxy(target);

        EXPECT_EQ(proxy.get_target(), target);
        EXPECT_EQ(proxy.get_name(), "before");
        EXPECT_EQ(proxy.get_level(), log4cpp::log_level::INFO);

        proxy.set_name("after");
        proxy.set_level(log4cpp::log_level::TRACE);
        proxy.warn("forwarded");

        EXPECT_EQ(target->get_name(), "after");
        EXPECT_EQ(target->get_level(), log4cpp::log_level::TRACE);
        ASSERT_EQ(target->levels.size(), 1U);
        EXPECT_EQ(target->levels.front(), log4cpp::log_level::WARN);
        ASSERT_EQ(target->messages.size(), 1U);
        EXPECT_EQ(target->messages.front(), "forwarded");
    }

} // namespace
