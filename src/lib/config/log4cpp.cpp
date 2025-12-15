#include "config/log4cpp.hpp"
#include "config/appender.hpp"
#include "exception/config_exception.hpp"

namespace log4cpp::config {
    // =========================================================
    // log_appender
    // =========================================================

    bool log_appender::empty() const {
        return !(console.has_value() || file.has_value() || socket.has_value());
    }

    std::vector<std::string> appender_flag_to_name(unsigned char flag) {
        std::vector<std::string> result;
        for (const auto &entry: APPENDER_TABLE) {
            if (flag & static_cast<unsigned char>(entry.type)) {
                result.emplace_back(entry.name);
            }
        }
        return result;
    }

    unsigned char appender_name_to_flag(const std::vector<std::string> &arr) {
        unsigned char flag = 0;
        for (const auto &s: arr) {
            bool matched = false;
            for (const auto &entry: APPENDER_TABLE) {
                if (s == entry.name) {
                    flag |= static_cast<unsigned char>(entry.type);
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                throw invalid_config_exception("unknown appender name: " + s);
            }
        }
        return flag;
    }

    void to_json(nlohmann::json &j, const log_appender &config) {
        j = nlohmann::json{};
        for (const auto &entry: APPENDER_TABLE) {
            switch (entry.type) {
                case APPENDER_TYPE::CONSOLE:
                    if (config.console) j[entry.name] = *config.console;
                    break;
                case APPENDER_TYPE::FILE:
                    if (config.file) j[entry.name] = *config.file;
                    break;
                case APPENDER_TYPE::SOCKET:
                    if (config.socket) j[entry.name] = *config.socket;
                    break;
                default:
                    break;
            }
        }
    }

    void from_json(const nlohmann::json &j, log_appender &config) {
        for (const auto &entry: APPENDER_TABLE) {
            if (!j.contains(entry.name)) continue;
            switch (entry.type) {
                case APPENDER_TYPE::CONSOLE:
                    config.console = j.at(entry.name).get<console_appender>();
                    break;
                case APPENDER_TYPE::FILE:
                    config.file = j.at(entry.name).get<file_appender>();
                    break;
                case APPENDER_TYPE::SOCKET:
                    config.socket = j.at(entry.name).get<socket_appender>();
                    break;
                default:
                    break;
            }
        }
    }

    // =========================================================
    // log4cpp
    // =========================================================

    void to_json(nlohmann::json &j, const log4cpp &config) {
        j = nlohmann::json{{"appenders", config.appenders}, {"loggers", config.loggers}};
        if (config.log_pattern.has_value()) {
            j["log-pattern"] = config.log_pattern.value();
        }
    }

    void from_json(const nlohmann::json &j, log4cpp &config) {
        /* Validate required fields */
        /* "log-pattern" is optional */
        /* "appenders" is mandatory */
        if (!j.contains("appenders")) throw invalid_config_exception("missing required field 'appenders'");
        /* "loggers" is mandatory */
        if (!j.contains("loggers")) throw invalid_config_exception("missing required field 'loggers'");

        // parse required fields
        if (j.contains("log-pattern")) {
            std::string pattern;
            j.at("log-pattern").get_to(pattern);
            config.log_pattern = pattern;
        }
        else {
            config.log_pattern = std::nullopt;
        }
        j.at("appenders").get_to(config.appenders);

        // "appenders" must define at least one appender
        if (config.appenders.empty()) throw invalid_config_exception("no appenders defined");

        // parse optional loggers
        j.at("loggers").get_to(config.loggers);

        // "root" logger must be defined in "loggers"
        bool root_found = false;
        std::vector<logger>::iterator root_it;
        logger root_logger;
        for (auto it = config.loggers.begin(); it != config.loggers.end(); ++it) {
            if (it->name == "root") {
                root_logger = *it;
                root_it = it;
                root_found = true;
                break;
            }
        }
        if (!root_found) {
            throw invalid_config_exception("root logger is not defined in 'loggers'");
        }

        if (root_logger.level.has_value()) {
            root_logger.level = log_level::WARN;
        }
        if (root_logger.appender == 0) {
            throw invalid_config_exception("root logger must define at least one appender");
        }

        // move root logger to first position in loggers
        std::rotate(config.loggers.begin(), root_it, config.loggers.end());

        // validate that each logger references only defined appenders
        for (const auto &lg: config.loggers) {
            // Skip the undefined appender
            if (0 == lg.appender) {
                continue;
            }
            for (const auto &ref: appender_flag_to_name(lg.appender)) {
                bool exists = false;
                for (const auto &entry: APPENDER_TABLE) {
                    if (ref == entry.name) {
                        switch (entry.type) {
                            case APPENDER_TYPE::CONSOLE:
                                exists = config.appenders.console.has_value();
                                break;
                            case APPENDER_TYPE::FILE:
                                exists = config.appenders.file.has_value();
                                break;
                            case APPENDER_TYPE::SOCKET:
                                exists = config.appenders.socket.has_value();
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                }
                if (!exists) {
                    throw invalid_config_exception("logger '" + lg.name + "' references undefined appender '" + ref
                                                   + "'");
                }
            }
        }
    }

    std::string log4cpp::serialize(const log4cpp &cfg) {
        nlohmann::json j;
        to_json(j, cfg);
        return j.dump();
    }

    log4cpp log4cpp::deserialize(const std::string &json) {
        nlohmann::json j = nlohmann::json::parse(json);
        return j.get<log4cpp>();
    }
}
