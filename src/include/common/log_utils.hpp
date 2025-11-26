#pragma once

#include <cstddef>
#include <string>

#include "log4cpp/log4cpp.hpp"

namespace log4cpp::common {
    /**
     * format a string
     * @param buf: the buffer to store the formatted string
     * @param size: the size of the buffer
     * @param fmt: the format string
     * @param args: the arguments
     * @return: the length of the formatted string
     */
    size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args);

    /**
     * format to a string
     * @param buf: the buffer to store the formatted string
     * @param size: the size of the buffer
     * @param fmt: the format string
     * @param ...: arguments
     * @return the length of the formatted string
     */
    size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...);

    /**
     * Replace the first occurrence of target with replace in original
     * @param original: the original string
     * @param length: the size of original buffer
     * @param target: the string to be replaced
     * @param replace: the string to replace
     * @return The length after replace
     */
    size_t log4c_replace(char *original, size_t length, const char *target, const char *replace);

    /**
     * Replace the first occurrence of str_old with str_new in target
     * @param original: the original string
     * @param target: the string to be replaced
     * @param replace: the string to replace
     * @return The length after replace
     */
    std::string log4c_replace(const std::string &original, const std::string &target, const std::string &replace);

    /**
     * Get current time(include Milliseconds)
     * @param now_tm: current time
     * @param ms: milliseconds of current time
     */
    void get_time_now(tm &now_tm, unsigned short &ms);

    /**
     * std::string convert to lowercase
     * @param s: input string
     * @return lowercase string
     */
    std::string to_lower(const std::string &s);

    /**
     * std::string convert to uppercase
     * @param s: input string
     * @return uppercase string
     */
    std::string to_upper(const std::string &s);
}
