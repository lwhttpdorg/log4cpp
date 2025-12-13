#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

#include "log4cpp/log4cpp.hpp"

#ifdef _MSC_VER
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on

#endif

#include <gtest/gtest.h>

#include "config/log4cpp.hpp"

void parse_json(const std::string &config_file, nlohmann::json &expected_json) {
    std::ifstream ifs(config_file);
    ASSERT_EQ(ifs.is_open(), true);
    expected_json = nlohmann::json::parse(ifs);
    ifs.close();
}

TEST(configuration_serialize_test, log4cpp_config_serialize_test) {
    // Just to load the configuration file
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();
    ASSERT_NO_THROW(log_mgr.load_config("serialize_test.json"));
    std::shared_ptr<log4cpp::log::logger> logger = log4cpp::logger_manager::get_logger("console_logger");
    const log4cpp::config::log4cpp *config = log_mgr.get_config();
    const std::string actual_json_str = log4cpp::config::log4cpp::serialize(*config);
    nlohmann::json expected_json;
    parse_json("serialize_test.json", expected_json);
    const nlohmann::json actual_json = nlohmann::json::parse(actual_json_str);
    EXPECT_EQ(expected_json, actual_json);
}
