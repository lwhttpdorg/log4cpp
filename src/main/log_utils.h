#pragma once

#include <cstddef>
#include <string>
#include "log4cpp.hpp"

size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args);

size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...);
