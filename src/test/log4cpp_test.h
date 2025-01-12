#pragma once

#define LOG4C_EXPECT_STR_EQ(expected, actual) \
if (0 != strcmp(expected, actual)) { \
	std::cout << __FILE__ << ":" << __LINE__ << ":" << __func__ << std::endl;\
	std::cout << "Expected: " << expected << std::endl;\
	std::cout << "Acutal  : " << actual << std::endl;\
	EXPECT_EQ(0, strcmp(expected, actual)); \
}

#define LOG4C_EXPECT_STRN_EQ(expected, actual, length) \
if (0 != strncmp(expected, actual, length)) { \
	std::cout << __FILE__ << ":" << __LINE__ << ":" << __func__ << std::endl;\
	std::cout << "Expected: " << expected << std::endl;\
	std::cout << "Acutal  : " << actual << std::endl;\
	std::cout << "Length  :" << length << std::endl;\
	EXPECT_EQ(0, strncmp(expected, actual, length)); \
}
