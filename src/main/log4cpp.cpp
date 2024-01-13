#ifdef _MSC_VER
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <boost/json.hpp>
#include <boost/algorithm/string.hpp>

#include "../include/log4cpp.hpp"
#include "log4cpp_config.h"
#include "console_output.h"
#include "file_output.h"

using namespace log4cpp;

/************************** log_level *****************************/
const char *LOG_LEVEL_FATAL = "FATAL";
const char *LOG_LEVEL_ERROR = "ERROR";
const char *LOG_LEVEL_WARN = "WARN";
const char *LOG_LEVEL_INFO = "INFO";
const char *LOG_LEVEL_DEBUG = "DEBUG";
const char *LOG_LEVEL_TRACE = "TRACE";

std::string log4cpp::to_string(log_level level)
{
    std::string str;
    switch (level)
    {
        case log_level::FATAL:
            str = LOG_LEVEL_FATAL;
            break;
        case log_level::ERROR:
            str = LOG_LEVEL_ERROR;
            break;
        case log_level::WARN:
            str = LOG_LEVEL_WARN;
            break;
        case log_level::INFO:
            str = LOG_LEVEL_INFO;
            break;
        case log_level::DEBUG:
            str = LOG_LEVEL_DEBUG;
            break;
        case log_level::TRACE:
            str = LOG_LEVEL_TRACE;
            break;
    }
    return str;
}

log_level log4cpp::from_string(const std::string &s)
{
    log_level level;
    auto tmp = boost::algorithm::to_upper_copy(s);
    if (tmp == LOG_LEVEL_FATAL)
    {
        level = log_level::FATAL;
    }
    else if (tmp == LOG_LEVEL_ERROR)
    {
        level = log_level::ERROR;
    }
    else if (tmp == LOG_LEVEL_WARN)
    {
        level = log_level::WARN;
    }
    else if (tmp == LOG_LEVEL_INFO)
    {
        level = log_level::INFO;
    }
    else if (tmp == LOG_LEVEL_DEBUG)
    {
        level = log_level::DEBUG;
    }
    else if (tmp == LOG_LEVEL_TRACE)
    {
        level = log_level::TRACE;
    }
    else
    {
        throw std::invalid_argument("invalid loglevel: " + s);
    }
    return level;
}

/**************************log*****************************/
logger::logger()
{
    this->level = log_level::WARN;
}

logger::logger(const std::string &log_name)
{
    this->name = log_name;
    this->level = log_level::WARN;
}

bool valid_output(const std::vector<std::string> &outputs, std::string &invalid_output_name)
{
    bool valid_output = true;
    for (const std::string &output: outputs)
    {
        if ((output != CONSOLE_OUTPUT_NAME) && (output != FILE_OUTPUT_NAME) && (output != TCP_OUTPUT_NAME) &&
            (output != UDP_OUTPUT_NAME))
        {
            valid_output = false;
            invalid_output_name = output;
            break;
        }
    }
    return valid_output;
}

logger::logger(const std::string &log_name, log_level l, const std::vector<std::string> &out)
{
    this->name = log_name;
    this->level = l;
    this->outputs = new output();
    std::string invalid_output_name;
    if (!valid_output(out, invalid_output_name))
    {
        throw std::invalid_argument("Unknown logOutPuts: " + invalid_output_name);
    }
    for (const std::string &output_name: out)
    {
        if (output_name == CONSOLE_OUTPUT_NAME)
        {
            this->outputs->set_console_output(new console_output());
        }
        else if (output_name == FILE_OUTPUT_NAME)
        {
            this->outputs->set_file_output(new file_output());
        }
        else
        {
            throw std::invalid_argument("invalid logger name: " + output_name);
        }
    }
}

logger::~logger()
{
    delete this->outputs;
}

// logger序列化, boost::json::value_from会调用此函数,
void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json_value, const logger &obj)
{
    boost::json::object json_obj = boost::json::object{{"name",       obj.name},
                                                       {"logLevel",   to_string(obj.level)},
                                                       {"logOutPuts", nullptr}};
    if (obj.outputs != nullptr)
    {
        json_obj["logOutPuts"] = boost::json::value_from<>(*obj.outputs);
    }
    json_value = json_obj;
}

// logger反序列化, boost::json::value_to会调用此函数
logger log4cpp::tag_invoke(boost::json::value_to_tag<logger>, const boost::json::value &json_value)
{
    std::vector<std::string> outputs = boost::json::value_to<std::vector<std::string>>(json_value.at("logOutPuts"));
    log_level level = from_string(boost::json::value_to<std::string>(json_value.at("logLevel")));
    std::string name;
    if (json_value.as_object().if_contains("name"))
    {
        name = boost::json::value_to<std::string>(json_value.at("name"));
    }
    else
    {
        name = "root";
    }
    return logger{name, level, outputs};
}


void logger::log(log_level _level, const char *fmt, va_list args)
{
    log_output *output = this->outputs->get_console_output();
    if (output != nullptr)
    {
        output->log(_level, fmt, args);
    }
    output = this->outputs->get_file_output();
    if (output != nullptr)
    {
        output->log(_level, fmt, args);
    }
}

void logger::fatal(const char *__restrict fmt, ...)
{
    if (this->level >= log_level::FATAL)
    {
        va_list args;
        va_start(args, fmt);
        this->log(log_level::FATAL, fmt, args);
        va_end(args);
    }
}

void logger::error(const char *__restrict fmt, ...)
{
    if (this->level >= log_level::ERROR)
    {
        va_list args;
        va_start(args, fmt);
        this->log(log_level::ERROR, fmt, args);
        va_end(args);
    }
}

void logger::warn(const char *__restrict fmt, ...)
{
    if (this->level >= log_level::WARN)
    {
        va_list args;
        va_start(args, fmt);
        this->log(log_level::WARN, fmt, args);
        va_end(args);
    }
}

void logger::info(const char *__restrict fmt, ...)
{
    if (this->level >= log_level::INFO)
    {
        va_list args;
        va_start(args, fmt);
        this->log(log_level::INFO, fmt, args);
        va_end(args);
    }
}

void logger::debug(const char *__restrict fmt, ...)
{
    if (this->level >= log_level::DEBUG)
    {

        va_list args;
        va_start(args, fmt);
        this->log(log_level::DEBUG, fmt, args);
        va_end(args);
    }
}

void logger::trace(const char *__restrict fmt, ...)
{
    if (this->level >= log_level::TRACE)
    {
        va_list args;
        va_start(args, fmt);
        this->log(log_level::TRACE, fmt, args);
        va_end(args);
    }
}

logger::logger(const logger &other)
{
    this->name = other.name;
    this->level = other.level;
    this->outputs = other.outputs;
}

logger::logger(logger &&other) noexcept
{
    this->name = other.name;
    this->level = other.level;
    this->outputs = other.outputs;
    other.outputs = nullptr;
}

logger &logger::operator=(const logger &other)
{
    if (this != &other)
    {
        this->name = other.name;
        this->level = other.level;
        this->outputs = other.outputs;
    }
    return *this;
}

logger &logger::operator=(logger &&other) noexcept
{
    if (this != &other)
    {
        this->name = other.name;
        this->level = other.level;
        this->outputs = other.outputs;
        other.outputs = nullptr;
    }
    return *this;
}
