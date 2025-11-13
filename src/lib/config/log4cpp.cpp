#include "config/appender.hpp"
#include "config/log4cpp.hpp"
#include "exception/invalid_config_exception.hpp"

namespace log4cpp::config {
    // =========================================================
    // log_appender
    // =========================================================

    bool log_appender::empty() const {
        return !(console.has_value() || file.has_value() || tcp.has_value() || udp.has_value());
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
                case APPENDER_TYPE::TCP:
                    if (config.tcp) j[entry.name] = *config.tcp;
                    break;
                case APPENDER_TYPE::UDP:
                    if (config.udp) j[entry.name] = *config.udp;
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
                case APPENDER_TYPE::TCP:
                    config.tcp = j.at(entry.name).get<tcp_appender>();
                    break;
                case APPENDER_TYPE::UDP:
                    config.udp = j.at(entry.name).get<udp_appender>();
                    break;
            }
        }
    }

    // =========================================================
    // log4cpp
    // =========================================================

    void to_json(nlohmann::json &j, const log4cpp &config) {
        j = nlohmann::json{
            {"log_pattern", config.log_pattern}, {"appenders", config.appenders}, {"root", config.root_logger}};
        if (!config.loggers.empty()) {
            j["loggers"] = config.loggers;
        }
    }

    void from_json(const nlohmann::json &j, log4cpp &config) {
        // check required fields
        if (!j.contains("log_pattern")) throw invalid_config_exception("missing required field 'log_pattern'");
        if (!j.contains("appenders")) throw invalid_config_exception("missing required field 'appenders'");
        if (!j.contains("root")) throw invalid_config_exception("missing required field 'root'");

        // parse required fields
        j.at("log_pattern").get_to(config.log_pattern);
        j.at("appenders").get_to(config.appenders);
        j.at("root").get_to(config.root_logger);

        // validate appenders
        if (config.appenders.empty()) throw invalid_config_exception("no appenders defined");

        // parse optional loggers
        if (j.contains("loggers")) {
            j.at("loggers").get_to(config.loggers);
        }
        else {
            config.loggers.clear();
        }

        // validate that each logger references only defined appenders
        for (const auto &lg: config.loggers) {
            for (const auto &ref: appender_flag_to_name(lg.appender_flag)) {
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
                            case APPENDER_TYPE::TCP:
                                exists = config.appenders.tcp.has_value();
                                break;
                            case APPENDER_TYPE::UDP:
                                exists = config.appenders.udp.has_value();
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

        // validate that root logger also references only defined appenders
        for (const auto &ref: appender_flag_to_name(config.root_logger.appender_flag)) {
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
                        case APPENDER_TYPE::TCP:
                            exists = config.appenders.tcp.has_value();
                            break;
                        case APPENDER_TYPE::UDP:
                            exists = config.appenders.udp.has_value();
                            break;
                    }
                    break;
                }
            }
            if (!exists) {
                throw invalid_config_exception("root logger references undefined appender '" + ref + "'");
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
