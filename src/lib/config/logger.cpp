#include "log4cpp/log4cpp.hpp"

#include "config/appender.hpp"
#include "config/log4cpp.hpp"
#include "config/logger.hpp"
#include "exception/config_exception.hpp"

namespace log4cpp::config {
    void to_json(json_value &j, const logger &config) {
        std::vector<std::string> appenders;
        for (const auto &entry: APPENDER_TABLE) {
            if (config.appender & static_cast<unsigned char>(entry.type)) {
                appenders.emplace_back(entry.name);
            }
        }
        json_array arr;
        for (const auto &a: appenders) {
            arr.emplace_back(json_value(a));
        }
        j = json_value{{"name", config.name}, {"appenders", json_value(std::move(arr))}};
        if (config.level.has_value()) {
            std::string str;
            to_string(config.level.value(), str);
            j["level"] = json_value(str);
        }
    }

    void from_json(const json_value &j, logger &config) {
        // Name is mandatory
        if (!j.contains("name")) {
            throw invalid_config_exception("Logger name is missing");
        }
        j.at("name").get_to(config.name);

        // Level is optional
        if (j.contains("level")) {
            std::string level_str;
            j.at("level").get_to(level_str);
            log_level level;
            from_string(level_str, level);
            config.level = level;
        }
        else {
            config.level = std::nullopt;
        }

        // Appender is optional
        if (j.contains("appenders")) {
            std::vector<std::string> appenders = j.at("appenders").get<std::vector<std::string>>();
            config.appender = appender_name_to_flag(appenders);
        }
        else {
            config.appender = 0;
        }
    }
} // namespace log4cpp::config
