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

        bool operator==(const console_appender &other) const noexcept {
            return out_stream == other.out_stream;
        }

        bool operator!=(const console_appender &other) const noexcept {
            return !(*this == other);
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

        bool operator==(const file_appender &other) const noexcept {
            return file_path == other.file_path;
        }

        bool operator!=(const file_appender &other) const noexcept {
            return !(*this == other);
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

        bool operator==(const socket_appender &other) const noexcept {
            return host == other.host && port == other.port && proto == other.proto && prefer == other.prefer;
        }

        bool operator!=(const socket_appender &other) const noexcept {
            return !(*this == other);
        }
    };

    void to_json(nlohmann::json &j, const socket_appender &config);
    void from_json(const nlohmann::json &j, socket_appender &config);
}
