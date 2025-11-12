#include "config/appender.hpp"

namespace log4cpp::config {
    // =========================================================
    // console appender
    // =========================================================

    void to_json(nlohmann::json &j, const console_appender &config) {
        j = nlohmann::json{{"out_stream", config.out_stream}};
    }

    void from_json(const nlohmann::json &j, console_appender &config) {
        j.at("out_stream").get_to(config.out_stream);
    }

    // =========================================================
    // file appender
    // =========================================================

    void to_json(nlohmann::json &j, const file_appender &config) {
        j = nlohmann::json{{"file_path", config.file_path}};
    }

    void from_json(const nlohmann::json &j, file_appender &config) {
        j.at("file_path").get_to(config.file_path);
    }

    // =========================================================
    // tcp appender
    // =========================================================

    void to_json(nlohmann::json &j, const tcp_appender &config) {
        j = nlohmann::json{{"local_addr", config.local_addr.to_string()}, {"port", config.port}};
    }

    void from_json(const nlohmann::json &j, tcp_appender &config) {
        j.at("local_addr").get_to(config.local_addr);
        j.at("port").get_to(config.port);
    }

    // =========================================================
    // udp appender
    // =========================================================

    void to_json(nlohmann::json &j, const udp_appender &config) {
        j = nlohmann::json{{"local_addr", config.local_addr.to_string()}, {"port", config.port}};
    }

    void from_json(const nlohmann::json &j, udp_appender &config) {
        j.at("local_addr").get_to(config.local_addr);
        j.at("port").get_to(config.port);
    }
}
