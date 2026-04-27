#if __cplusplus >= 202002L
#include <format>
#endif

#include "config/appender.hpp"
#include "config/log4cpp.hpp"
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

    void to_json(json_value &j, const log_appender &config) {
        j = json_value{};
        for (const auto &entry: APPENDER_TABLE) {
            switch (entry.type) {
                case APPENDER_TYPE::CONSOLE:
                    if (config.console) {
                        json_value cj;
                        to_json(cj, *config.console);
                        j[entry.name] = cj;
                    }
                    break;
                case APPENDER_TYPE::FILE:
                    if (config.file) {
                        json_value fj;
                        to_json(fj, *config.file);
                        j[entry.name] = fj;
                    }
                    break;
                case APPENDER_TYPE::SOCKET:
                    if (config.socket) {
                        json_value sj;
                        to_json(sj, *config.socket);
                        j[entry.name] = sj;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    void from_json(const json_value &j, log_appender &config) {
        for (const auto &entry: APPENDER_TABLE) {
            if (!j.contains(entry.name)) {
                continue;
            }
            switch (entry.type) {
                case APPENDER_TYPE::CONSOLE: {
                    console_appender ca;
                    from_json(j.at(entry.name), ca);
                    config.console = ca;
                    break;
                }
                case APPENDER_TYPE::FILE: {
                    file_appender fa;
                    from_json(j.at(entry.name), fa);
                    config.file = fa;
                    break;
                }
                case APPENDER_TYPE::SOCKET: {
                    socket_appender sa;
                    from_json(j.at(entry.name), sa);
                    config.socket = sa;
                    break;
                }
                default:
                    break;
            }
        }
    }

    // =========================================================
    // log4cpp
    // =========================================================

    void to_json(json_value &j, const log4cpp &config) {
        json_value appenders_j;
        to_json(appenders_j, config.appenders);
        j = json_value{{"appenders", appenders_j}};
        json_array cfg_loggers_arr;
        for (const auto &[name, log]: config.loggers) {
            json_value lj;
            to_json(lj, log);
            cfg_loggers_arr.push_back(lj);
        }
        j["loggers"] = json_value(std::move(cfg_loggers_arr));
        if (config.log_pattern.has_value()) {
            j["log-pattern"] = json_value(config.log_pattern.value());
        }
    }

    void from_json(const json_value &j, log4cpp &config) {
        /* Validate required fields */
        /* "log-pattern" is optional */
        /* "appenders" is mandatory */
        if (!j.contains("appenders")) {
            throw invalid_config_exception("missing required field 'appenders'");
        }
        /* "loggers" is mandatory */
        if (!j.contains("loggers")) {
            throw invalid_config_exception("missing required field 'loggers'");
        }

        // parse required fields
        if (j.contains("log-pattern")) {
            std::string pattern;
            j.at("log-pattern").get_to(pattern);
            config.log_pattern = pattern;
        }
        else {
            config.log_pattern = std::nullopt;
        }
        from_json(j.at("appenders"), config.appenders);

        // "appenders" must define at least one appender
        if (config.appenders.empty()) {
            throw invalid_config_exception("no appenders defined");
        }

        // parse optional loggers
        const auto &loggers_arr = j.at("loggers").get<json_array>();
        std::vector<logger> cfg_loggers;
        for (const auto &elem: loggers_arr) {
            logger l;
            from_json(elem, l);
            cfg_loggers.push_back(l);
        }
        for (auto &cfg_logger: cfg_loggers) {
            config.loggers.emplace(cfg_logger.name, cfg_logger);
        }

        // "root" logger must be defined in "loggers"
        auto root_it = config.loggers.find(FALLBACK_LOGGER_NAME);
        if (config.loggers.end() == root_it) {
            throw invalid_config_exception("root logger is not defined in 'loggers'");
        }
        logger root_logger = root_it->second;

        if (root_logger.level.has_value()) {
            root_logger.level = log_level::WARN;
        }
        if (root_logger.appender == 0) {
            throw invalid_config_exception("root logger must define at least one appender");
        }

        // validate that each logger references only defined appenders
        for (const auto &[name, log]: config.loggers) {
            // Skip the undefined appender
            if (0 == log.appender) {
                continue;
            }
            for (const auto &ref: appender_flag_to_name(log.appender)) {
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
#if __cplusplus >= 202002L
                    throw invalid_config_exception(
                        std::format("logger '{}' references undefined appender '{}'", name, ref));
#else
                    // NOLINTNEXTLINE(performance-inefficient-string-concatenation)
                    throw invalid_config_exception("logger '" + name + "' references undefined appender '" + ref + "'");
#endif
                }
            }
        }
    }

    std::string log4cpp::serialize(const log4cpp &cfg) {
        json_value j;
        to_json(j, cfg);
        return j.dump();
    }

    log4cpp log4cpp::deserialize(const std::string &json_str) {
        json_value j = json_value::parse(json_str);
        log4cpp cfg;
        from_json(j, cfg);
        return cfg;
    }
} // namespace log4cpp::config
