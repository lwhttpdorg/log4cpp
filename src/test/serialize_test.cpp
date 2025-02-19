#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "log4cpp.hpp"

#ifdef _MSC_VER
// clang-format off
#include <winsock2.h>
#include <windows.h>
// clang-format on

#endif
#include "main/log4cpp_config.h"

#include "gtest/gtest.h"

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

void parse_json(const std::string &config_file, nlohmann::json &expected_json) {
	std::ifstream ifs(config_file);
	ASSERT_EQ(ifs.is_open(), true);
	expected_json = nlohmann::json::parse(ifs);
	ifs.close();
}

TEST(configuration_serialize_test, log4cpp_config_serialize_test) {
	// Just to load the configuration file
	std::shared_ptr<log4cpp::layout> layout = log4cpp::layout_manager::get_layout("console_layout");
	const log4cpp::log4cpp_config *config = log4cpp::layout_manager::get_config();
	const std::string actual_json_str = log4cpp::log4cpp_config::serialize(*config);
	nlohmann::json expected_json;
	parse_json("log4cpp.json", expected_json);
	const nlohmann::json actual_json = nlohmann::json::parse(actual_json_str);
	EXPECT_EQ(expected_json, actual_json);
}
