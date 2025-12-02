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

        friend bool operator==(const console_appender &lhs, const console_appender &rhs) {
            return lhs.out_stream == rhs.out_stream;
        }
        friend bool operator!=(const console_appender &lhs, const console_appender &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(nlohmann::json &j, const console_appender &config);

    void from_json(const nlohmann::json &j, console_appender &config);

    // =========================================================
    // file appender
    // =========================================================

    class file_appender {
    public:
        std::string file_path;

        friend bool operator==(const file_appender &lhs, const file_appender &rhs) {
            return lhs.file_path == rhs.file_path;
        }
        friend bool operator!=(const file_appender &lhs, const file_appender &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(nlohmann::json &j, const file_appender &config);

    void from_json(const nlohmann::json &j, file_appender &config);

    // =========================================================
    // socket appender
    // =========================================================

    class socket_appender {
    public:
        enum class protocol { TCP, UDP };
        std::string host{};
        unsigned short port{0};
        protocol proto{protocol::TCP};
        common::prefer_stack prefer{common::prefer_stack::AUTO};

        friend bool operator==(const socket_appender &lhs, const socket_appender &rhs) {
            return lhs.host == rhs.host && lhs.port == rhs.port && lhs.proto == rhs.proto && lhs.prefer == rhs.prefer;
        }
        friend bool operator!=(const socket_appender &lhs, const socket_appender &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(nlohmann::json &j, const socket_appender &config);
    void from_json(const nlohmann::json &j, socket_appender &config);
}
