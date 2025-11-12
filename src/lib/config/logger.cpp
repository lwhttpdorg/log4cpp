#include "config/logger.hpp"

#include <config/log4cpp.hpp>
#include "config/appender.hpp"
#include "exception/invalid_config_exception.hpp"

namespace log4cpp::config {
    constexpr const char *DEFAULT_LOGGER_NAME = "root";

    void to_json(nlohmann::json &j, const logger &config) {
        std::vector<std::string> appenders;
        for (const auto &entry: APPENDER_TABLE) {
            if (config.appender_flag & static_cast<unsigned char>(entry.type)) {
                appenders.emplace_back(entry.name);
            }
        }
        j = nlohmann::json{{"level", level_to_string(config.level)}, {"appenders", appenders}};
        // {"name", config.name}
        if (config.name != DEFAULT_LOGGER_NAME) {
            j["name"] = config.name;
        }
    }

    void from_json(const nlohmann::json &j, logger &config) {
        if (!j.contains("name")) {
            config.name = DEFAULT_LOGGER_NAME;
        }
        else {
            j.at("name").get_to(config.name);
        }
        if (!j.contains("level"))
            throw invalid_config_exception("logger '" + j.value("name", "") + "' missing required field 'level'");
        if (!j.contains("appenders"))
            throw invalid_config_exception("logger '" + j.value("name", "") + "' missing required field 'appenders'");

        std::string level_str;
        j.at("level").get_to(level_str);
        config.level = level_from_string(level_str);

        std::vector<std::string> appenders;
        j.at("appenders").get_to(appenders);

        config.appender_flag = appenders_to_flag(appenders);
    }
}
