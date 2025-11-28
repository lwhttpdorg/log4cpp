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

        bool operator==(const logger &other) const noexcept {
            return name == other.name && level == other.level && appender == other.appender;
        }

        bool operator!=(const logger &other) const noexcept {
            return !(*this == other);
        }
    };

    void to_json(nlohmann::json &j, const logger &config);

    void from_json(const nlohmann::json &j, logger &config);
}
