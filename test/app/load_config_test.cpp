#include <gtest/gtest.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <filesystem>
#include <fstream>

#include "nlohmann/json.hpp"

#include "log4cpp/log4cpp.hpp"

#include "common/log_net.hpp"
#include "config/log4cpp.hpp"

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

void log_pattern_check(const nlohmann::json &expected_json, const std::string &log_pattern) {
    const std::string expected = expected_json.at("log_pattern");
    EXPECT_EQ(log_pattern, expected);
}

void console_appender_check(const nlohmann::json &console_appender, const log4cpp::config::console_appender &cfg) {
    const std::string expected = console_appender.at("out_stream");
    EXPECT_EQ(cfg.out_stream, expected);
}

void file_appender_check(const nlohmann::json &file_appender, const log4cpp::config::file_appender &cfg) {
    const std::string expected = file_appender.at("file_path");
    EXPECT_EQ(cfg.file_path, expected);
}

void tcp_appender_check(const nlohmann::json &tcp_appender, const log4cpp::config::tcp_appender &cfg) {
    const log4cpp::common::net_addr expected_addr = log4cpp::common::net_addr(tcp_appender.at("local_addr"));
    EXPECT_EQ(cfg.local_addr, expected_addr);
    unsigned short expected_port = tcp_appender.at("port");
    EXPECT_EQ(cfg.port, expected_port);
}

void udp_appender_check(const nlohmann::json &udp_appender, const log4cpp::config::udp_appender &cfg) {
    const log4cpp::common::net_addr expected_addr = log4cpp::common::net_addr(udp_appender.at("local_addr"));
    EXPECT_EQ(cfg.local_addr, expected_addr);
    unsigned short expected_port = udp_appender.at("port");
    EXPECT_EQ(cfg.port, expected_port);
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

    // TCP Appender
    ASSERT_EQ(true == appenders.contains("tcp"), appenders_cfg.tcp.has_value());
    const log4cpp::config::tcp_appender &tcp_appender_cfg = appenders_cfg.tcp.value();
    const nlohmann::json &tcp_appender = appenders.at("tcp");
    tcp_appender_check(tcp_appender, tcp_appender_cfg);

    // UDP Appender
    ASSERT_EQ(true == appenders.contains("udp"), appenders_cfg.udp.has_value());
    const log4cpp::config::udp_appender &udp_appender_cfg = appenders_cfg.udp.value();
    const nlohmann::json &udp_appender = appenders.at("udp");
    udp_appender_check(udp_appender, udp_appender_cfg);
}

namespace log4cpp {
    void from_json(const nlohmann::json &json, log4cpp::config::logger &obj) {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! from_json !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        obj.name = json.at("name");
        obj.level = level_from_string(json.at("level"));
        std::vector<std::string> appenders = json.at("appenders");
        unsigned char appenders_flag = log4cpp::config::appender_name_to_flag(appenders);
        obj.appender_flag = appenders_flag;
    }

    void to_json(nlohmann::json &json, const log4cpp::config::logger &obj) {
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! to_json !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        std::vector<std::string> appenders = log4cpp::config::appender_flag_to_name(obj.appender_flag);
        json = nlohmann::json{{"name", obj.name}, {"level", obj.level}, {"appenders", appenders}};
    }
}

bool logger_less(const log4cpp::config::logger &a, const log4cpp::config::logger &b) {
    if (a.name != b.name) return a.name < b.name;
    if (a.level != b.level) return a.level < b.level;
    return a.appender_flag < b.appender_flag;
}

bool logger_equal(const log4cpp::config::logger &a, const log4cpp::config::logger &b) {
    return a.name == b.name && a.level == b.level && a.appender_flag == b.appender_flag;
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

void root_logger_check(const nlohmann::json &expected_json, const log4cpp::config::logger &root_logger) {
    const nlohmann::json &expected_root_logger = expected_json.at("root");
    // name
    EXPECT_EQ(root_logger.name, "root");

    log4cpp::log_level expected_level = log4cpp::level_from_string(expected_root_logger.at("level"));
    EXPECT_EQ(root_logger.level, expected_level);

    unsigned char actual_flag = root_logger.appender_flag;
    const std::vector<std::string> expected_appenders = expected_root_logger.at("appenders");
    unsigned char expected_flag = log4cpp::config::appender_name_to_flag(expected_appenders);
    EXPECT_EQ(actual_flag, expected_flag);
}

void parse_json(const std::string &file, nlohmann::json &out) {
    std::ifstream ifs(file);
    ASSERT_TRUE(ifs.is_open()) << "cannot open " << file;
    out = nlohmann::json::parse(ifs);
    ifs.close();
}

void configuration_check(const nlohmann::json &expected_json, const log4cpp::config::log4cpp *config) {
    // Logger pattern
    const std::string &log_pattern = config->log_pattern;
    log_pattern_check(expected_json, log_pattern);
    // Appenders
    appenders_check(expected_json, config->appenders);

    // Loggers
    std::vector<log4cpp::config::logger> loggers_cfg = config->loggers;
    std::sort(loggers_cfg.begin(), loggers_cfg.end(), logger_less);
    logger_check(expected_json, loggers_cfg);

    // Root logger
    const log4cpp::config::logger &root_logger_cfg = config->root_logger;
    root_logger_check(expected_json, root_logger_cfg);
}

TEST(load_config_test, auto_load_config) {
    // Just to load the configuration file
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    // Calling get_logger() causes automatic loading
    const auto logger = log4cpp::logger_manager::get_logger("root");
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    nlohmann::json expected_json;
    parse_json("log4cpp.json", expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, load_config_test_1) {
    const std::string config_file = "log4cpp_config_1.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    nlohmann::json expected_json;
    parse_json(config_file, expected_json);
    configuration_check(expected_json, config);
}

TEST(load_config_test, load_config_test_2) {
    const std::string config_file = "log4cpp_config_2.json";
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    log_mgr.load_config(config_file);
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    ASSERT_NE(nullptr, config);

    nlohmann::json expected_json;
    parse_json(config_file, expected_json);
    configuration_check(expected_json, config);
}
