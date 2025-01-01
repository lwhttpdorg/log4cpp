#pragma once

#include <cstddef>
#include <string>
#include "log4cpp.hpp"

/* formatted output to a string
 * buf: the buffer to store the formatted string
 * size: the size of the buffer
 * fmt: the format string
 * args: the arguments
 * return: the length of the formatted string
 */
size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args);

/* formatted output to a string
 * buf: the buffer to store the formatted string
 * size: the size of the buffer
 * fmt: the format string
 * return: the length of the formatted string
 */
size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...);

/* Replace the first occurrence of str_old with str_new in target
 * target: the target string
 * length: the length of the target string
 * str_old: the string to be replaced
 * str_new: the string to replace
 * return: 0 if successful, -1 if failed
 */
int replace(char *target, size_t length, const char *str_old, const char *str_new);

/* Replace the first occurrence of str_old with str_new in target
 * target: the target string
 * str_old: the string to be replaced
 * str_new: the string to replace
 * return: the new string
 */
std::string replace(const std::string &target, const std::string &str_old, const std::string &str_new);
