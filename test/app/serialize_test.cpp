#include <filesystem>
#include <fstream>
#include "nlohmann/json.hpp"

#include "log4cpp/log4cpp.hpp"

#include <gtest/gtest.h>

#include "config/log4cpp.hpp"

namespace fs = std::filesystem;

void parse_json(const std::string &config_file, nlohmann::json &expected_json) {
    std::ifstream ifs(config_file);
    ASSERT_TRUE(ifs.is_open());
    expected_json = nlohmann::json::parse(ifs);
    ifs.close();
}

TEST(configuration_serialize_test, log4cpp_config_roundtrip_test) {
    auto &log_mgr = log4cpp::supervisor::get_logger_manager();

    for (const auto &entry: fs::directory_iterator(fs::current_path())) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            const std::string filename = entry.path().string();

            // Load original config
            ASSERT_NO_THROW(log_mgr.load_config(filename));
            const log4cpp::config::log4cpp original_config = *log_mgr.get_config();

            // Serialize to JSON string
            const std::string json_str = log4cpp::config::log4cpp::serialize(original_config);

            // Write to a temporary file
            const std::string tmpfile = (fs::temp_directory_path() / entry.path().filename()).string();
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
