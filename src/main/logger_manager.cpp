#include <filesystem>
#include "log4cpp.hpp"
#include "logger_builder.h"
#include "log4cpp_config.h"
#include "log_lock.h"

using namespace log4cpp;

bool logger_manager::initialized = false;
log4cpp_config logger_manager::config;
logger_manager::auto_load_config logger_manager::init;
log_lock logger_manager::lock;
std::unordered_map<std::string, logger> logger_manager::loggers;


logger_manager::auto_load_config::auto_load_config()
{
    std::string default_config_file = "./log4cpp.json";
    if (-1 != access(default_config_file.c_str(), F_OK))
    {
        logger_manager::config = log4cpp_config::load_config(default_config_file);
        logger_manager::initialized = true;
    }
}

logger_manager::auto_load_config::~auto_load_config()
{
    while (!logger_manager::loggers.empty())
    {
        auto begin = logger_manager::loggers.begin();
        logger_manager::loggers.erase(begin);
    }
}

void logger_manager::load_config(const std::string &json_filepath)
{
    if (-1 != access(json_filepath.c_str(), F_OK))
    {
        config = log4cpp_config::load_config(json_filepath);
        logger_manager::initialized = true;
    }
    else
    {
        throw std::filesystem::filesystem_error("Config file " + json_filepath + "opening failed!",
                                                std::make_error_code(std::io_errc::stream));
    }
}

logger logger_manager::get_logger(const std::string &name)
{
    if (!logger_manager::initialized)
    {
        throw std::runtime_error("Not initialized: The configuration file does not exist? Or forgotten load_config()?");
    }
    logger log;
    if (logger_manager::loggers.find(name) == logger_manager::loggers.end())
    {
        logger_manager::lock.lock();
        if (logger_manager::loggers.find(name) == logger_manager::loggers.end())
        {
            logger tmp_log = build_logger(name);
            logger_manager::loggers.insert({name, tmp_log});
        }
        log = logger_manager::loggers.at(name);
        logger_manager::lock.unlock();
    }
    else
    {
        log = logger_manager::loggers.at(name);
    }
    return log;
}

logger logger_manager::build_logger(const std::string &name)
{
    logger_config cfg = config.get_logger_config(name);
    logger_builder::builder builder = logger_builder::new_builder();
    builder.set_name(name);
    builder.set_log_level(cfg.get_logger_level());
    auto it = std::find_if(config.loggers.begin(), config.loggers.end(),
                           [&name](logger const &logger) { return logger.name == name; });
    if (it != config.loggers.cend())
    {
        logger log = *it;
        builder builder = logger_builder::new_builder();
        builder.set_name(name);
        builder.set_log_level(log.level);
        log_output *out = log.outputs->get_console_output();
        if (out != nullptr)
        {
            builder.set_console_output(out);
        }
        out = log.outputs->get_file_output();
        if (out != nullptr)
        {
            builder.set_file_output(out);
        }
        return builder.build();
    }
    else
    {
        logger log = config.root_logger;
        builder builder = logger_builder::new_builder();
        builder.set_name(name);
        builder.set_log_level(log.level);
        log_output *out = log.outputs->get_console_output();
        if (out != nullptr)
        {
            builder.set_console_output(out);
        }
        out = log.outputs->get_file_output();
        if (out != nullptr)
        {
            builder.set_file_output(out);
        }
        return builder.build();
    }
}
