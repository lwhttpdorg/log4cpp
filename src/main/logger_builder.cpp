#include "logger_builder.h"

using namespace log4cpp;

log4cpp_config logger_builder::config;

logger_builder::logger_builder()
{
    std::string default_config = "./log4cpp.json";
    if (-1 != access(default_config.c_str(), F_OK))
    {
        logger_builder::config = load_config(default_config);
    }
}

void logger_builder::set_config_filepath(const std::string &json_filepath)
{
    if (-1 != access(json_filepath.c_str(), F_OK))
    {
        logger_builder::config = load_config(json_filepath);
    }
    else
    {
        std::string what("Can not open the config file, ");
        what.append(strerror(errno));
        what.append("(" + std::to_string(errno) + ")");
        throw std::runtime_error(what);
    }
}

logger_builder::builder logger_builder::new_builder()
{
    return logger_builder::builder{};
}

logger_builder::builder &logger_builder::builder::set_name(const std::string &name)
{
    this->log.name = name;
    return *this;
}

logger_builder::builder &logger_builder::builder::set_log_level(log_level level)
{
    this->log.level = level;
    return *this;
}

logger_builder::builder &logger_builder::builder::set_console_output(log_output *output)
{
    if (output != nullptr)
    {
        if (this->log.outputs == nullptr)
        {
            throw std::runtime_error("Bad usage");
        }
        else
        {
            this->log.outputs->set_console_output(output);
        }
    }
    return *this;
}

logger_builder::builder &logger_builder::builder::set_file_output(log_output *output)
{
    if (output != nullptr)
    {
        if (this->log.outputs == nullptr)
        {
            throw std::runtime_error("Bad usage");
        }
        else
        {
            this->log.outputs->set_file_output(output);
        }
    }
    return *this;
}

logger logger_builder::builder::build()
{
    return this->log;
}
