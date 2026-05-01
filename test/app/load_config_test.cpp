#include <gtest/gtest.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <common/log_utils.hpp>
#include <filesystem>
#include <fstream>

#include "common/json.hpp"

#include "log4cpp/log4cpp.hpp"

#include "common/log_net.hpp"
#include "config/log4cpp.hpp"
#include "exception/config_exception.hpp"

using json = log4cpp::json_value;

void log_pattern_check(const json &expected_json, const std::string &log_pattern) {
    const std::string expected = expected_json.at("log-pattern").get<std::string>();
    EXPECT_EQ(log_pattern, expected);
}

void console_appender_check(const json &console_appender, const log4cpp::config::console_appender &cfg) {
    const std::string expected = console_appender.at("out-stream").get<std::string>();
    EXPECT_EQ(cfg.out_stream, expected);
}

void file_appender_check(const json &file_appender, const log4cpp::config::file_appender &cfg) {
    const std::string expected = file_appender.at("file-path").get<std::string>();
    EXPECT_EQ(cfg.file_path, expected);
}

void socket_appender_check(const json &tcp_appender, const log4cpp::config::socket_appender &cfg) {
    const std::string expected_host = tcp_appender.at("host").get<std::string>();
    EXPECT_EQ(cfg.host, expected_host);

    unsigned short expected_port = tcp_appender.at("port").get<unsigned short>();
    EXPECT_EQ(cfg.port, expected_port);

    std::string expected_proto_str = tcp_appender.at("protocol").get<std::string>();
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

    std::string expected_prefer_stack = tcp_appender.at("prefer-stack").get<std::string>();
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

void appenders_check(const json &appenders_json, const log4cpp::config::log_appender &appenders_cfg) {
    const json &appenders = appenders_json.at("appenders");
    // Console Appender
    ASSERT_EQ(true == appenders.contains("console"), appenders_cfg.console.has_value());
    const log4cpp::config::console_appender &console_appender_cfg = appenders_cfg.console.value();
    const json &console_appender = appenders.at("console");
    console_appender_check(console_appender, console_appender_cfg);

    // File Appender
    ASSERT_EQ(true == appenders.contains("file"), appenders_cfg.file.has_value());
    const log4cpp::config::file_appender &file_appender_cfg = appenders_cfg.file.value();
    const json &file_appender = appenders.at("file");
    file_appender_check(file_appender, file_appender_cfg);

    // Socket Appender
    ASSERT_EQ(true == appenders.contains("socket"), appenders_cfg.socket.has_value());
    const log4cpp::config::socket_appender &socket_appender_cfg = appenders_cfg.socket.value();
    const json &socket_appender = appenders.at("socket");
    socket_appender_check(socket_appender, socket_appender_cfg);
}

void from_json_logger(const json &j, log4cpp::config::logger &obj) {
    obj.name = j.at("name").get<std::string>();
    log4cpp::log_level level;
    from_string(j.at("level").get<std::string>(), level);
    obj.level = level;
    std::vector<std::string> appenders = j.at("appenders").get<std::vector<std::string>>();
    unsigned char appenders_flag = log4cpp::config::appender_name_to_flag(appenders);
    obj.appender = appenders_flag;
}

bool logger_less(const log4cpp::config::logger &a, const log4cpp::config::logger &b) {
    if (a.name != b.name) {
        return a.name < b.name;
    }
    if (a.level != b.level) {
        return a.level < b.level;
    }
    return a.appender < b.appender;
}

bool logger_equal(const log4cpp::config::logger &a, const log4cpp::config::logger &b) {
    return a.name == b.name && a.level == b.level && a.appender == b.appender;
}

void logger_check(const json &expected_json, std::vector<log4cpp::config::logger> &actual_loggers) {
    EXPECT_EQ(actual_loggers.empty(), true != expected_json.contains("loggers"));
    if (actual_loggers.empty()) {
        return;
    }
    const json &loggers_json = expected_json.at("loggers");
    auto loggers_arr = loggers_json.get<log4cpp::json_array>();
    std::vector<log4cpp::config::logger> expected_loggers;
    for (const auto &elem: loggers_arr) {
        log4cpp::config::logger l;
        from_json_logger(elem, l);
        expected_loggers.push_back(l);
    }
    std::sort(actual_loggers.begin(), actual_loggers.end(), logger_less);
    std::sort(expected_loggers.begin(), expected_loggers.end(), logger_less);
    bool is_equal = std::equal(expected_loggers.begin(), expected_loggers.end(), actual_loggers.begin(), logger_equal);
    EXPECT_EQ(is_equal, true);
}

void parse_json(const std::string &file, json &out) {
    std::ifstream ifs(file);
    ASSERT_TRUE(ifs.is_open()) << "cannot open " << file;
    out = json::parse(ifs);
    ifs.close();
}

void configuration_check(const json &expected_json, const log4cpp::config::log4cpp *config) {
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

    json expected_json;
    parse_json("log4cpp.json", expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, load_config_test_1) {
    const std::string config_file = "log4cpp_config_1.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    json expected_json;
    parse_json(config_file, expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, load_config_test_2) {
    const std::string config_file = "log4cpp_config_2.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config(config_file));
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    json expected_json;
    parse_json(config_file, expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, missing_appenders_field) {
    constexpr const char *bad_json = R"({"loggers":[{"name":"root","level":"INFO","appenders":["console"]}]})";
    EXPECT_THROW(log4cpp::config::log4cpp::deserialize(bad_json), log4cpp::config::invalid_config_exception);
}

TEST(load_config_test, missing_loggers_field) {
    constexpr const char *bad_json = R"({"appenders":{"console":{"out-stream":"stdout"}}})";
    EXPECT_THROW(log4cpp::config::log4cpp::deserialize(bad_json), log4cpp::config::invalid_config_exception);
}

TEST(load_config_test, missing_root_logger) {
    constexpr const char *bad_json =
        R"({"appenders":{"console":{"out-stream":"stdout"}},"loggers":[{"name":"other","level":"INFO","appenders":["console"]}]})";
    EXPECT_THROW(log4cpp::config::log4cpp::deserialize(bad_json), log4cpp::config::invalid_config_exception);
}

TEST(load_config_test, invalid_json_syntax) {
    constexpr const char *bad_json = "{this is not json";
    EXPECT_THROW(log4cpp::config::log4cpp::deserialize(bad_json), log4cpp::json_parse_error);
}

TEST(load_config_test, undefined_appender_reference) {
    constexpr const char *bad_json =
        R"({"appenders":{"console":{"out-stream":"stdout"}},"loggers":[{"name":"root","level":"INFO","appenders":["file"]}]})";
    EXPECT_THROW(log4cpp::config::log4cpp::deserialize(bad_json), log4cpp::config::invalid_config_exception);
}

TEST(load_config_test, invalid_socket_protocol) {
    constexpr const char *bad_json =
        R"({"appenders":{"socket":{"host":"localhost","port":1234,"protocol":"unknown","prefer-stack":"auto"}},"loggers":[{"name":"root","level":"INFO","appenders":["socket"]}]})";
    EXPECT_THROW(log4cpp::config::log4cpp::deserialize(bad_json), std::invalid_argument);
}
