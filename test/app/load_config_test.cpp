#include <gtest/gtest.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <common/log_utils.hpp>
#include <filesystem>
#include <fstream>

#include "nlohmann/json.hpp"

#include "log4cpp/log4cpp.hpp"

#include "common/log_net.hpp"
#include "config/log4cpp.hpp"

void log_pattern_check(const nlohmann::json &expected_json, const std::string &log_pattern) {
    const std::string expected = expected_json.at("log-pattern");
    EXPECT_EQ(log_pattern, expected);
}

void console_appender_check(const nlohmann::json &console_appender, const log4cpp::config::console_appender &cfg) {
    const std::string expected = console_appender.at("out-stream");
    EXPECT_EQ(cfg.out_stream, expected);
}

void file_appender_check(const nlohmann::json &file_appender, const log4cpp::config::file_appender &cfg) {
    const std::string expected = file_appender.at("file-path");
    EXPECT_EQ(cfg.file_path, expected);
}

void socket_appender_check(const nlohmann::json &tcp_appender, const log4cpp::config::socket_appender &cfg) {
    const std::string expected_host = tcp_appender.at("host");
    EXPECT_EQ(cfg.host, expected_host);

    unsigned short expected_port = tcp_appender.at("port");
    EXPECT_EQ(cfg.port, expected_port);

    std::string expected_proto_str = tcp_appender.at("protocol");
    std::string proto_str;
    if (cfg.proto == log4cpp::config::socket_appender::protocol::TCP) {
        proto_str = "tcp";
    }
    else if (cfg.proto == log4cpp::config::socket_appender::protocol::UDP) {
        proto_str = "udp";
    }
    proto_str = log4cpp::common::to_upper(proto_str);
    expected_proto_str = log4cpp::common::to_upper(expected_proto_str);
    EXPECT_EQ(proto_str, expected_proto_str);

    std::string expected_prefer_stack = tcp_appender.at("prefer-stack");
    expected_prefer_stack = log4cpp::common::to_lower(expected_prefer_stack);
    std::string actual_prefer_stack;
    if (cfg.prefer == log4cpp::common::prefer_stack::IPv4) {
        actual_prefer_stack = "ipv4";
    }
    else if (cfg.prefer == log4cpp::common::prefer_stack::IPv6) {
        actual_prefer_stack = "ipv6";
    }
    else if (cfg.prefer == log4cpp::common::prefer_stack::AUTO) {
        actual_prefer_stack = "auto";
    }
    EXPECT_EQ(actual_prefer_stack, expected_prefer_stack);
}

void appenders_check(const nlohmann::json &appenders_json, const log4cpp::config::log_appender &appenders_cfg) {
    const nlohmann::json &appenders = appenders_json.at("appenders");
    // Console Appender
    ASSERT_EQ(true == appenders.contains("console"), appenders_cfg.console.has_value());
    const log4cpp::config::console_appender &console_appender_cfg = appenders_cfg.console.value();
    const nlohmann::json &console_appender = appenders.at("console");
    console_appender_check(console_appender, console_appender_cfg);

    // File Appender
    ASSERT_EQ(true == appenders.contains("file"), appenders_cfg.file.has_value());
    const log4cpp::config::file_appender &file_appender_cfg = appenders_cfg.file.value();
    const nlohmann::json &file_appender = appenders.at("file");
    file_appender_check(file_appender, file_appender_cfg);

    // Socket Appender
    ASSERT_EQ(true == appenders.contains("socket"), appenders_cfg.socket.has_value());
    const log4cpp::config::socket_appender &socket_appender_cfg = appenders_cfg.socket.value();
    const nlohmann::json &socket_appender = appenders.at("socket");
    socket_appender_check(socket_appender, socket_appender_cfg);
}

namespace log4cpp {
    void from_json(const nlohmann::json &json, log4cpp::config::logger &obj) {
        obj.name = json.at("name");
        log4cpp::log_level level;
        from_string(json.at("level"), level);
        obj.level = level;
        std::vector<std::string> appenders = json.at("appenders");
        unsigned char appenders_flag = log4cpp::config::appender_name_to_flag(appenders);
        obj.appender = appenders_flag;
    }

    void to_json(nlohmann::json &json, const log4cpp::config::logger &obj) {
        std::vector<std::string> appenders = log4cpp::config::appender_flag_to_name(obj.appender);
        json = nlohmann::json{{{"name", obj.name}, {"level", obj.level.value()}, {"appenders", appenders}}};
    }
}

bool logger_less(const log4cpp::config::logger &a, const log4cpp::config::logger &b) {
    if (a.name != b.name) return a.name < b.name;
    if (a.level != b.level) return a.level < b.level;
    return a.appender < b.appender;
}

bool logger_equal(const log4cpp::config::logger &a, const log4cpp::config::logger &b) {
    return a.name == b.name && a.level == b.level && a.appender == b.appender;
}

void logger_check(const nlohmann::json &expected_json, std::vector<log4cpp::config::logger> &actual_loggers) {
    EXPECT_EQ(actual_loggers.empty(), true != expected_json.contains("loggers"));
    if (actual_loggers.empty()) {
        return;
    }
    const nlohmann::json &loggers_json = expected_json.at("loggers");
    std::vector<log4cpp::config::logger> expected_loggers = loggers_json.get<std::vector<log4cpp::config::logger>>();
    std::sort(actual_loggers.begin(), actual_loggers.end(), logger_less);
    std::sort(expected_loggers.begin(), expected_loggers.end(), logger_less);
    bool is_equal = std::equal(expected_loggers.begin(), expected_loggers.end(), actual_loggers.begin(), logger_equal);
    EXPECT_EQ(is_equal, true);
}

void parse_json(const std::string &file, nlohmann::json &out) {
    std::ifstream ifs(file);
    ASSERT_TRUE(ifs.is_open()) << "cannot open " << file;
    out = nlohmann::json::parse(ifs);
    ifs.close();
}

void configuration_check(const nlohmann::json &expected_json, const log4cpp::config::log4cpp *config) {
    // Logger pattern
    const std::string &log_pattern = config->log_pattern.value();
    log_pattern_check(expected_json, log_pattern);
    // Appenders
    appenders_check(expected_json, config->appenders);

    // Loggers
    std::vector<log4cpp::config::logger> loggers_cfg;
    for (const auto &[name, logger]: config->loggers) {
        loggers_cfg.push_back(logger);
    }
    std::sort(loggers_cfg.begin(), loggers_cfg.end(), logger_less);
    logger_check(expected_json, loggers_cfg);
}

TEST(load_config_test, auto_load_config) {
    // Just to load the configuration file
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    // Calling get_logger() causes automatic loading
    const auto logger = log4cpp::logger_manager::get_logger(log4cpp::FALLBACK_LOGGER_NAME);
    logger->info("[load_config_test.auto_load_config] Calling get_logger() causes automatic loading");
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    nlohmann::json expected_json;
    parse_json("log4cpp.json", expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, load_config_test_1) {
    const std::string config_file = "log4cpp_config_1.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    nlohmann::json expected_json;
    parse_json(config_file, expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, load_config_test_2) {
    const std::string config_file = "log4cpp_config_2.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    nlohmann::json expected_json;
    parse_json(config_file, expected_json);
    configuration_check(expected_json, config);
}
