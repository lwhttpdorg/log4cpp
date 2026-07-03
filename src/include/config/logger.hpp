#pragma once

#include <optional> // for std::optional
#include <string>   // for std::string

#include "common/json.hpp"     // for json_value
#include "log4cpp/log4cpp.hpp" // for log_level

namespace log4cpp::config {
    class logger {
    public:
        /* Logger name */
        std::string name;
        /* Logger level */
        std::optional<log_level> level;
        /* appender flag */
        unsigned char appender{};

        friend bool operator==(const logger &lhs, const logger &rhs) {
            return lhs.name == rhs.name && lhs.level == rhs.level && lhs.appender == rhs.appender;
        }

        friend bool operator!=(const logger &lhs, const logger &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(::log4cpp::json_value &j, const logger &config);

    void from_json(const ::log4cpp::json_value &j, logger &config);
} // namespace log4cpp::config
