#pragma once

#define LOG4C_EXPECT_STR_EQ(expected, actual, message) \
    if (0 != strcmp(expected, actual)) { \
        std::cout << "!!! " << __FILE__ << ":" << __LINE__ << std::endl; \
        std::cout << "Expected: " << expected << std::endl; \
        std::cout << "Acutal  : " << actual << std::endl; \
        ADD_FAILURE_AT(__FILE__, __LINE__) << message; \
    }

#define LOG4C_EXPECT_STRN_EQ(expected, actual, length, message) \
    if (0 != strncmp(expected, actual, length)) { \
        std::cout << "!!! " << __FILE__ << ":" << __LINE__ << std::endl; \
        std::cout << "Expected: " << expected << std::endl; \
        std::cout << "Acutal  : " << actual << std::endl; \
        std::cout << "Length  :" << length << std::endl; \
        ADD_FAILURE_AT(__FILE__, __LINE__) << message; \
    }
