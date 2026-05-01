#pragma once

#include <cstring>
#include <iostream>

#define LOG4C_EXPECT_STR_EQ(expected, actual, message) \
    do { \
        if ((expected) == nullptr || (actual) == nullptr) { \
            std::cout << "!!! " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::cout << "Expected: " << ((expected) ? (expected) : "(null)") << std::endl; \
            std::cout << "Actual  : " << ((actual) ? (actual) : "(null)") << std::endl; \
            ADD_FAILURE_AT(__FILE__, __LINE__) << message; \
        } \
        else if (0 != std::strcmp((expected), (actual))) { \
            std::cout << "!!! " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::cout << "Expected: " << (expected) << std::endl; \
            std::cout << "Actual  : " << (actual) << std::endl; \
            ADD_FAILURE_AT(__FILE__, __LINE__) << message; \
        } \
    } while (0)

#define LOG4C_EXPECT_STRN_EQ(expected, actual, length, message) \
    do { \
        if ((expected) == nullptr || (actual) == nullptr) { \
            std::cout << "!!! " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::cout << "Expected: " << ((expected) ? (expected) : "(null)") << std::endl; \
            std::cout << "Actual  : " << ((actual) ? (actual) : "(null)") << std::endl; \
            std::cout << "Length  :" << (length) << std::endl; \
            ADD_FAILURE_AT(__FILE__, __LINE__) << message; \
        } \
        else if (0 != std::strncmp((expected), (actual), (length))) { \
            std::cout << "!!! " << __FILE__ << ":" << __LINE__ << std::endl; \
            std::cout << "Expected: " << (expected) << std::endl; \
            std::cout << "Actual  : " << (actual) << std::endl; \
            std::cout << "Length  :" << (length) << std::endl; \
            ADD_FAILURE_AT(__FILE__, __LINE__) << message; \
        } \
    } while (0)
