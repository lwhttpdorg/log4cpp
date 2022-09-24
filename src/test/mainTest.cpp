#include <yaml-cpp/yaml.h>

#include "gtest/gtest.h"
#include "../main/LogConfiger.h"


int main(int argc, char **argv)
{
	::testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}

TEST(LogConfigerTest, logConfig)
{
	std::string yamlFile = "/home/nereus/WorkSpace/github/log4cpp/log4cpp.yml";
	Log4CppConfiger log4CppConfiger;
}
