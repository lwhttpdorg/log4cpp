#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include <log4cpp/log4cpp.hpp>

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

    void to_json(nlohmann::json &j, const logger &config);

    void from_json(const nlohmann::json &j, logger &config);
} // namespace log4cpp::config
