#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include <log4cpp/log4cpp.hpp>

namespace log4cpp::config {
    class logger {
    public:
        /* Logger name */
        std::string name;
        /* Logger level */
        log_level level;
        /* appender flag */
        unsigned char appender_flag{};
    };

    void to_json(nlohmann::json &j, const logger &config);

    void from_json(const nlohmann::json &j, logger &config);
}
