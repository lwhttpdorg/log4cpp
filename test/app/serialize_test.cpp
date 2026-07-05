#include <filesystem> // for std::filesystem
#include <fstream>    // for std::ifstream, std::ofstream
#include <string>     // for std::string

#include <gtest/gtest.h> // for TEST, ASSERT_*

#include "common/json.hpp"     // for json_value
#include "config/log4cpp.hpp"  // for log4cpp config
#include "log4cpp/log4cpp.hpp" // for supervisor

void parse_json(const std::string &config_file, log4cpp::json_value &expected_json) {
    std::ifstream ifs(config_file);
    ASSERT_TRUE(ifs.is_open());
    expected_json = log4cpp::json_value::parse(ifs);
    ifs.close();
}

TEST(configuration_serialize_test, log4cpp_config_roundtrip_test) {
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();

    for (const auto &entry: std::filesystem::directory_iterator(std::filesystem::current_path())) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            const std::string filename = entry.path().filename().string();
            if (!filename.starts_with("test_") && !filename.starts_with("log4cpp")) {
                continue;
            }

            // Load original config
            ASSERT_NO_THROW(log_mgr.load_config(filename));
            const log4cpp::config::log4cpp original_config = *log_mgr.get_config();

            // Serialize to JSON string
            const std::string json_str = log4cpp::config::log4cpp::serialize(original_config);

            // Write to a temporary file
            const std::string tmpfile = (std::filesystem::temp_directory_path() / entry.path().filename()).string();
            {
                std::ofstream ofs(tmpfile);
                ofs << json_str;
            }

            // Load config back from the temporary file
            ASSERT_NO_THROW(log_mgr.load_config(tmpfile));
            const log4cpp::config::log4cpp roundtrip_config = *log_mgr.get_config();

            // Compare the two config objects (you may need to implement operator==)
            EXPECT_EQ(original_config, roundtrip_config) << "Roundtrip mismatch in file: " << filename;
        }
    }
}
