#pragma once

#include "../include/log4cpp.hpp"
#include "console_output.h"
#include "file_output.h"
#include "tcp_output.h"
#include "udp_output.h"

namespace log4cpp
{

    class output_config
    {
        friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, output_config const &obj);

        friend output_config tag_invoke(boost::json::value_to_tag<output_config>, boost::json::value const &json);

    public:
        static constexpr unsigned char CONSOLE_OUT_CFG = 0x01;
        static constexpr unsigned char FILE_OUT_CFG = 0x02;
        static constexpr unsigned char TCP_OUT_CFG = 0x04;
        static constexpr unsigned char UDP_OUT_CFG = 0x08;
        console_output_config *console_cfg;
        file_output_config *file_cfg;
        //tcp_output_config *tcp_cfg;
        //udp_output_config *udp_cfg;
    };

    void tag_invoke(boost::json::value_from_tag, boost::json::value &json, output_config const &obj);

    output_config tag_invoke(boost::json::value_to_tag<output_config>, boost::json::value const &json);

    class logger_config
    {
    public:
        std::string get_logger_name();

        log_level get_logger_level();

        console_output *get_console_output();

        file_output_config *get_file_output_cfg();

        //tcp_output_config *get_tcp_output_cfg();

        //udp_output_config *get_udp_output_cfg();

    private:
        std::string name;
        log_level level;
        console_output_config *console_cfg;
        file_output_config *file_cfg;
        //tcp_output_config *tcp_cfg;
        //udp_output_config *udp_cfg;
    };

    class log4cpp_config
    {
    public:
        static log4cpp_config load_config(const std::string &json_file);

        friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

        friend log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);

        friend std::string serialize(log4cpp_config const &obj);

        log4cpp_config() = default;

        log4cpp_config(std::string _pattern, output &o, const std::vector<logger> &l, logger root);

        log4cpp_config(const log4cpp_config &other);

        log4cpp_config(log4cpp_config &&other) noexcept;

        log4cpp_config &operator=(const log4cpp_config &other);

        log4cpp_config &operator=(log4cpp_config &&other) noexcept;

        logger_config get_logger_config(const std::string &name);

    private:
        std::string pattern;            // pattern
        unsigned char out_cfg_flags{0}; // logOutPut
        console_output_config console_cfg;
        file_output_config file_cfg;
        //tcp_output_config tcp_cfg;
        //udp_output_config udp_cfg;
        std::vector<logger_config> loggers; // loggers
        logger_config root_logger;          // rootLogger
    };

    void tag_invoke(boost::json::value_from_tag, boost::json::value &json, log4cpp_config const &obj);

    log4cpp_config tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json);

    std::string serialize(log4cpp_config const &obj);
}
