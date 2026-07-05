#include <cctype>           // for std::isdigit, std::isspace
#include <initializer_list> // for std::initializer_list
#include <limits>           // for std::numeric_limits
#include <stdexcept>        // for std::invalid_argument
#include <string>           // for std::string
#include <string_view>      // for std::string_view
#include <utility>          // for std::pair

#include "common/log_utils.hpp" // for to_lower
#include "config/appender.hpp"  // for appender configs

namespace log4cpp::config {
    // =========================================================
    // console appender
    // =========================================================

    void to_json(json_value &j, const console_appender &config) {
        j = json_value{{"out-stream", config.out_stream}};
    }

    void from_json(const json_value &j, console_appender &config) {
        j.at("out-stream").get_to(config.out_stream);
    }

    // =========================================================
    // file appender
    // =========================================================

    static std::string trim_copy(std::string_view value) {
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
            value.remove_prefix(1);
        }
        while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
            value.remove_suffix(1);
        }
        return std::string(value);
    }

    static uint64_t parse_uint_with_unit(const std::string &value, const std::string &field_name,
                                         const std::initializer_list<std::pair<std::string_view, uint64_t>> &units) {
        const std::string trimmed = trim_copy(value);
        size_t pos = 0;
        while (pos < trimmed.size() && std::isdigit(static_cast<unsigned char>(trimmed[pos])) != 0) {
            ++pos;
        }
        if (pos == 0) {
            throw std::invalid_argument("Invalid '" + field_name + "' value '" + value + "'");
        }

        const uint64_t number = std::stoull(trimmed.substr(0, pos));
        std::string unit = common::to_upper(trim_copy(std::string_view(trimmed).substr(pos)));
        uint64_t multiplier = 0;
        for (const auto &[name, factor]: units) {
            if (unit == name) {
                multiplier = factor;
                break;
            }
        }
        if (multiplier == 0) {
            throw std::invalid_argument("Invalid '" + field_name + "' unit in value '" + value + "'");
        }
        if (number > std::numeric_limits<uint64_t>::max() / multiplier) {
            throw std::invalid_argument("'" + field_name + "' value is too large: '" + value + "'");
        }
        return number * multiplier;
    }

    uint64_t file_appender::parse_file_size(const std::string &value) {
        return parse_uint_with_unit(
            value, "file-size",
            {{"", 1}, {"B", 1}, {"KB", 1024}, {"MB", 1024 * 1024}, {"GB", 1024ull * 1024ull * 1024ull}});
    }

    std::chrono::seconds file_appender::parse_history(const std::string &value) {
        const uint64_t seconds = parse_uint_with_unit(
            value, "max-history", {{"S", 1}, {"M", 60}, {"H", 60 * 60}, {"D", 24 * 60 * 60}, {"W", 7 * 24 * 60 * 60}});
        if (seconds > static_cast<uint64_t>(std::chrono::seconds::max().count())) {
            throw std::invalid_argument("'max-history' value is too large: '" + value + "'");
        }
        return std::chrono::seconds(seconds);
    }

    std::string file_appender::format_file_size(uint64_t bytes) {
        constexpr uint64_t GB = 1024ull * 1024ull * 1024ull;
        constexpr uint64_t MB = 1024ull * 1024ull;
        constexpr uint64_t KB = 1024ull;
        if (bytes >= GB && bytes % GB == 0) {
            return std::to_string(bytes / GB) + "GB";
        }
        if (bytes >= MB && bytes % MB == 0) {
            return std::to_string(bytes / MB) + "MB";
        }
        if (bytes >= KB && bytes % KB == 0) {
            return std::to_string(bytes / KB) + "KB";
        }
        return std::to_string(bytes) + "B";
    }

    std::string file_appender::format_history(std::chrono::seconds value) {
        const auto seconds = value.count();
        constexpr int64_t WEEK = 7 * 24 * 60 * 60;
        constexpr int64_t DAY = 24 * 60 * 60;
        constexpr int64_t HOUR = 60 * 60;
        constexpr int64_t MINUTE = 60;
        if (seconds >= WEEK && seconds % WEEK == 0) {
            return std::to_string(seconds / WEEK) + "w";
        }
        if (seconds >= DAY && seconds % DAY == 0) {
            return std::to_string(seconds / DAY) + "d";
        }
        if (seconds >= HOUR && seconds % HOUR == 0) {
            return std::to_string(seconds / HOUR) + "h";
        }
        if (seconds >= MINUTE && seconds % MINUTE == 0) {
            return std::to_string(seconds / MINUTE) + "m";
        }
        return std::to_string(seconds) + "s";
    }

    void to_string(rolling_policy_type policy, std::string &str) {
        switch (policy) {
            case rolling_policy_type::NONE:
                str = "none";
                break;
            case rolling_policy_type::SIZE:
                str = "size";
                break;
            case rolling_policy_type::TIME:
                str = "time";
                break;
            case rolling_policy_type::ON_START:
                str = "on-start";
                break;
            case rolling_policy_type::SIZE_TIME:
                str = "size-time";
                break;
        }
    }

    void from_string(const std::string &str, rolling_policy_type &policy) {
        const std::string value = common::to_lower(str);
        if (value == "none") {
            policy = rolling_policy_type::NONE;
        }
        else if (value == "size") {
            policy = rolling_policy_type::SIZE;
        }
        else if (value == "time") {
            policy = rolling_policy_type::TIME;
        }
        else if (value == "on-start") {
            policy = rolling_policy_type::ON_START;
        }
        else if (value == "size-time" || value == "size+time" || value == "size_and_time") {
            policy = rolling_policy_type::SIZE_TIME;
        }
        else {
            throw std::invalid_argument("Invalid rolling policy string '" + str + "'");
        }
    }

    void to_string(rolling_time_interval interval, std::string &str) {
        str = interval == rolling_time_interval::HOUR ? "hour" : "day";
    }

    void from_string(const std::string &str, rolling_time_interval &interval) {
        const std::string value = common::to_lower(str);
        if (value == "hour") {
            interval = rolling_time_interval::HOUR;
        }
        else if (value == "day") {
            interval = rolling_time_interval::DAY;
        }
        else {
            throw std::invalid_argument("Invalid rolling interval string '" + str + "'");
        }
    }

    void to_json(json_value &j, const rolling_policy &config) {
        std::string policy;
        to_string(config.policy, policy);
        j = json_value{{"policy", policy}};
        if (config.policy == rolling_policy_type::SIZE || config.policy == rolling_policy_type::SIZE_TIME) {
            j["file-size"] = file_appender::format_file_size(config.file_size);
        }
        if (config.policy == rolling_policy_type::TIME || config.policy == rolling_policy_type::SIZE_TIME) {
            std::string interval;
            to_string(config.interval, interval);
            j["interval"] = interval;
        }
        if (config.file_count > 0) {
            j["file-count"] = json_value(static_cast<uint64_t>(config.file_count));
        }
        if (config.max_history.count() > 0) {
            j["max-history"] = file_appender::format_history(config.max_history);
        }
    }

    void from_json(const json_value &j, rolling_policy &config) {
        config = rolling_policy{};
        std::string policy_str;
        j.at("policy").get_to(policy_str);
        from_string(policy_str, config.policy);

        if (j.contains("file-size")) {
            config.file_size = file_appender::parse_file_size(j.at("file-size").get<std::string>());
        }
        if (j.contains("interval")) {
            std::string interval_str;
            j.at("interval").get_to(interval_str);
            from_string(interval_str, config.interval);
        }
        if (j.contains("file-count")) {
            const uint64_t file_count = j.at("file-count").get<uint64_t>();
            if (file_count > std::numeric_limits<uint32_t>::max()) {
                throw std::invalid_argument("'file-count' value is too large");
            }
            config.file_count = static_cast<uint32_t>(file_count);
        }
        if (j.contains("max-history")) {
            config.max_history = file_appender::parse_history(j.at("max-history").get<std::string>());
        }

        if ((config.policy == rolling_policy_type::SIZE || config.policy == rolling_policy_type::SIZE_TIME)
            && config.file_size == 0) {
            throw std::invalid_argument("rolling policy '" + policy_str + "' requires positive 'file-size'");
        }
    }

    void to_json(json_value &j, const file_appender &config) {
        j = json_value{{"file-path", config.file_path}};
        if (config.rolling) {
            json_value rolling_j;
            to_json(rolling_j, *config.rolling);
            j["rolling"] = rolling_j;
        }
    }

    void from_json(const json_value &j, file_appender &config) {
        j.at("file-path").get_to(config.file_path);
        config.rolling.reset();
        if (j.contains("rolling")) {
            rolling_policy rolling;
            from_json(j.at("rolling"), rolling);
            config.rolling = rolling;
        }
    }

    // =========================================================
    // socket appender
    // =========================================================

    void to_json(json_value &j, const socket_appender &config) {
        std::string prefer_str;
        to_string(config.prefer, prefer_str);
        j = json_value{
            {"host", config.host},
            {"port", json_value(static_cast<uint64_t>(config.port))},
            {"protocol", std::string(config.proto == socket_appender::protocol::TCP ? "TCP" : "UDP")},
            {"prefer-stack", prefer_str},
        };
    }

    void from_json(const json_value &j, socket_appender &config) {
        j.at("host").get_to(config.host);
        config.port = j.at("port").get<unsigned short>();
        std::string proto_str;
        j.at("protocol").get_to(proto_str);
        proto_str = common::to_upper(proto_str);
        if (proto_str == "TCP") {
            config.proto = socket_appender::protocol::TCP;
        }
        else if (proto_str == "UDP") {
            config.proto = socket_appender::protocol::UDP;
        }
        else {
            throw std::invalid_argument("Invalid protocol string \'" + proto_str + "\'");
        }
        std::string prefer_str;
        j.at("prefer-stack").get_to(prefer_str);
        // Convert to lowercase for comparison
        from_string(prefer_str, config.prefer);
    }
} // namespace log4cpp::config
