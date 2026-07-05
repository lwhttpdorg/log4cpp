#pragma once

#include <chrono>   // for std::chrono::seconds
#include <cstdint>  // for uint8_t, uint32_t, uint64_t
#include <optional> // for std::optional
#include <string>   // for std::string

#include "common/json.hpp"    // for json_value
#include "common/log_net.hpp" // for prefer_stack

namespace log4cpp::config {
    // =========================================================
    // console appender
    // =========================================================

    class console_appender {
    public:
        /* The out stream, "stdout" or "stderr" */
        std::string out_stream;

        friend bool operator==(const console_appender &lhs, const console_appender &rhs) {
            return lhs.out_stream == rhs.out_stream;
        }
        friend bool operator!=(const console_appender &lhs, const console_appender &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(::log4cpp::json_value &j, const console_appender &config);

    void from_json(const ::log4cpp::json_value &j, console_appender &config);

    // =========================================================
    // file appender
    // =========================================================

    enum class rolling_policy_type : uint8_t { NONE, SIZE, TIME, ON_START, SIZE_TIME };

    enum class rolling_time_interval : uint8_t { HOUR, DAY };

    void to_string(rolling_policy_type policy, std::string &str);

    void from_string(const std::string &str, rolling_policy_type &policy);

    void to_string(rolling_time_interval interval, std::string &str);

    void from_string(const std::string &str, rolling_time_interval &interval);

    class rolling_policy {
    public:
        rolling_policy_type policy{rolling_policy_type::NONE};
        uint64_t file_size{0};
        rolling_time_interval interval{rolling_time_interval::DAY};
        uint32_t file_count{0};
        std::chrono::seconds max_history{0};

        friend bool operator==(const rolling_policy &lhs, const rolling_policy &rhs) {
            return lhs.policy == rhs.policy && lhs.file_size == rhs.file_size && lhs.interval == rhs.interval
                   && lhs.file_count == rhs.file_count && lhs.max_history == rhs.max_history;
        }
        friend bool operator!=(const rolling_policy &lhs, const rolling_policy &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(::log4cpp::json_value &j, const rolling_policy &config);

    void from_json(const ::log4cpp::json_value &j, rolling_policy &config);

    class file_appender {
    public:
        std::string file_path;
        std::optional<rolling_policy> rolling;

        [[nodiscard]] static uint64_t parse_file_size(const std::string &value);
        [[nodiscard]] static std::chrono::seconds parse_history(const std::string &value);
        [[nodiscard]] static std::string format_file_size(uint64_t bytes);
        [[nodiscard]] static std::string format_history(std::chrono::seconds value);

        friend bool operator==(const file_appender &lhs, const file_appender &rhs) {
            return lhs.file_path == rhs.file_path && lhs.rolling == rhs.rolling;
        }
        friend bool operator!=(const file_appender &lhs, const file_appender &rhs) {
            return !(lhs == rhs);
        }
    };

    void to_json(::log4cpp::json_value &j, const file_appender &config);

    void from_json(const ::log4cpp::json_value &j, file_appender &config);

    // =========================================================
    // socket appender
    // =========================================================

    class socket_appender {
    public:
        enum class protocol : uint8_t { TCP, UDP };
        std::string host;
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

    void to_json(::log4cpp::json_value &j, const socket_appender &config);
    void from_json(const ::log4cpp::json_value &j, socket_appender &config);
} // namespace log4cpp::config
