#pragma once

#include <nlohmann/json.hpp>

#include "common/log_net.hpp"

namespace log4cpp::config {
    // =========================================================
    // console appender
    // =========================================================

    class console_appender {
    public:
        /* The out stream, "stdout" or "stderr" */
        std::string out_stream{};
    };

    void to_json(nlohmann::json &j, const console_appender &config);

    void from_json(const nlohmann::json &j, console_appender &config);

    // =========================================================
    // file appender
    // =========================================================

    class file_appender {
    public:
        std::string file_path;
    };

    void to_json(nlohmann::json &j, const file_appender &config);

    void from_json(const nlohmann::json &j, file_appender &config);

    // =========================================================
    // tcp appender
    // =========================================================

    class tcp_appender {
    public:
        common::net_addr local_addr{};
        unsigned short port{0};
    };

    void to_json(nlohmann::json &j, const tcp_appender &config);

    void from_json(const nlohmann::json &j, tcp_appender &config);

    // =========================================================
    // udp appender
    // =========================================================

    class udp_appender {
    public:
        common::net_addr local_addr{};
        unsigned short port{0};
    };

    void to_json(nlohmann::json &j, const udp_appender &config);

    void from_json(const nlohmann::json &j, udp_appender &config);
}
