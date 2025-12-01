#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "common/log_utils.hpp"
#include "log4cpp/log4cpp.hpp"

namespace log4cpp {
    /************************** log_level *****************************/
    const char *LOG_LEVEL_FATAL = "FATAL";
    const char *LOG_LEVEL_ERROR = "ERROR";
    const char *LOG_LEVEL_WARN = "WARN";
    const char *LOG_LEVEL_INFO = "INFO";
    const char *LOG_LEVEL_DEBUG = "DEBUG";
    const char *LOG_LEVEL_TRACE = "TRACE";

    void to_string(log_level level, std::string &str) {
        switch (level) {
            case log_level::FATAL:
                str = LOG_LEVEL_FATAL;
                break;
            case log_level::ERROR:
                str = LOG_LEVEL_ERROR;
                break;
            case log_level::WARN:
                str = LOG_LEVEL_WARN;
                break;
            case log_level::INFO:
                str = LOG_LEVEL_INFO;
                break;
            case log_level::DEBUG:
                str = LOG_LEVEL_DEBUG;
                break;
            case log_level::TRACE:
                str = LOG_LEVEL_TRACE;
                break;
            default:
                break;
        }
    }

    void from_string(const std::string &str, log_level &level) {
        std::string tmp = common::to_upper(str);
        if (tmp == LOG_LEVEL_FATAL) {
            level = log_level::FATAL;
        }
        else if (tmp == LOG_LEVEL_ERROR) {
            level = log_level::ERROR;
        }
        else if (tmp == LOG_LEVEL_WARN) {
            level = log_level::WARN;
        }
        else if (tmp == LOG_LEVEL_INFO) {
            level = log_level::INFO;
        }
        else if (tmp == LOG_LEVEL_DEBUG) {
            level = log_level::DEBUG;
        }
        else if (tmp == LOG_LEVEL_TRACE) {
            level = log_level::TRACE;
        }
        else {
            throw std::invalid_argument("invalid loglevel \'" + str + "\'");
        }
    }
}
