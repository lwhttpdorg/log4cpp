#pragma once

#include <optional>

#include "appender.hpp"
#include "logger.hpp"

namespace log4cpp::config {
    // =========================================================
    // appender enum + table
    // =========================================================
    enum class APPENDER_TYPE : unsigned char { CONSOLE = 1 << 0, FILE = 1 << 1, SOCKET = 1 << 2 };

    class appender_attr {
    public:
        const char *name;
        APPENDER_TYPE type;
    };

    constexpr std::array<appender_attr, 3> APPENDER_TABLE{{{"console", APPENDER_TYPE::CONSOLE},
                                                           {"file", APPENDER_TYPE::FILE},
                                                           {"socket", APPENDER_TYPE::SOCKET}}};

    std::vector<std::string> appender_flag_to_name(unsigned char flag);

    unsigned char appender_name_to_flag(const std::vector<std::string> &arr);

    class log_appender {
    public:
        std::optional<console_appender> console;
        std::optional<file_appender> file;
        std::optional<socket_appender> socket;

        [[nodiscard]] bool empty() const;
    };

    void to_json(nlohmann::json &j, const log_appender &config);

    void from_json(const nlohmann::json &j, log_appender &config);

    class log4cpp {
    public:
        std::string log_pattern; // log_pattern
        log_appender appenders{}; // appenders
        std::vector<logger> loggers; // loggers
        logger root_logger; // Root logger
        static std::string serialize(const log4cpp &cfg);
        static log4cpp deserialize(const std::string &json);
    };

    void to_json(nlohmann::json &j, const log4cpp &config);

    void from_json(const nlohmann::json &j, log4cpp &config);
}
