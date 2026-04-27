#include "config/appender.hpp"

#include <common/log_utils.hpp>

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

    void to_json(json_value &j, const file_appender &config) {
        j = json_value{{"file-path", config.file_path}};
    }

    void from_json(const json_value &j, file_appender &config) {
        j.at("file-path").get_to(config.file_path);
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
