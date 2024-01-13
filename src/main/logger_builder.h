#pragma once

#include <string>
#include "log4cpp.hpp"
#include "log4cpp_config.h"

namespace log4cpp
{
    class logger_builder
    {
    public:
        class builder
        {
        public:
            builder &set_name(const std::string &name);

            builder &set_log_level(log_level level);

            builder &set_console_output(log_output *output);

            builder &set_file_output(log_output *output);

            logger build();

        private:
            logger log;
        };

        friend class logger_manager;

    public:
        static void set_config_filepath(const std::string &json_filepath);

        static builder new_builder();

        static logger get_logger(const std::string &name);

    private:
        logger_builder();
    };
}