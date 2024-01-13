#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>

#include "log4cpp_config.h"
#include "console_output.h"
#include "file_output.h"


using namespace log4cpp;

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const output_config &obj)
{
    json = boost::json::object{{"consoleOutPut", boost::json::value_from(*obj.console_cfg)},
                               {"fileOutPut",    boost::json::value_from(*obj.file_cfg)}};
}

output_config log4cpp::tag_invoke(boost::json::value_to_tag<output_config>, boost::json::value const &json)
{
    console_output_config console_out = boost::json::value_to<console_output_config>(json.at("consoleOutPut"));
    file_output_config file_out = boost::json::value_to<file_output_config>(json.at("fileOutPut"));
    output_config output_config{};
    output_config.console_cfg = new console_output_config(console_out);
    output_config.file_cfg = new file_output_config(file_out);
    return output_config;
}

log_output *output::get_console_output() const
{
    return console_out;
}

void output::set_console_output(log_output *out)
{
    console_out = out;
}

log_output *output::get_file_output() const
{
    return file_out;
}

void output::set_file_output(log_output *file_output)
{
    file_out = file_output;
}

output::output(const output &other)
{
    if (other.console_out != nullptr)
    {
        console_output console_o = *dynamic_cast<console_output *>(other.console_out);
        this->console_out = new console_output(console_o);
    }
    else
    {
        this->console_out = nullptr;
    }
    if (other.file_out != nullptr)
    {
        file_output file_o = *dynamic_cast<file_output *>(other.file_out);
        this->file_out = new file_output(file_o);
    }
    else
    {
        this->file_out = nullptr;
    }
}

output::output(output &&other) noexcept
{
    this->console_out = other.console_out;
    other.console_out = nullptr;
    this->file_out = other.file_out;
    other.file_out = nullptr;
}

output &output::operator=(const output &other)
{
    if (this != &other)
    {
        if (other.console_out != nullptr)
        {
            console_output console_o = *dynamic_cast<console_output *>(other.console_out);
            this->console_out = new console_output(console_o);
        }
        else
        {
            this->console_out = nullptr;
        }
        if (other.file_out != nullptr)
        {
            file_output file_o = *dynamic_cast<file_output *>(other.file_out);
            this->file_out = new file_output(file_o);
        }
        else
        {
            this->file_out = nullptr;
        }
    }
    return *this;
}

output &output::operator=(output &&other) noexcept
{
    if (this != &other)
    {
        this->console_out = other.console_out;
        other.console_out = nullptr;
        this->file_out = other.file_out;
        other.file_out = nullptr;
    }
    return *this;
}

output::~output()
{
    delete this->console_out;
    delete this->file_out;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const log4cpp_config &obj)
{
    json = boost::json::object{{"pattern",    obj.pattern},
                               {"logOutPut",  boost::json::value_from(obj._output)},
                               {"loggers",    boost::json::value_from(obj.loggers)},
                               {"rootLogger", boost::json::value_from(obj.root_logger)}};
}

log4cpp_config log4cpp::tag_invoke(boost::json::value_to_tag<log4cpp_config>, boost::json::value const &json)
{
    std::string pattern = boost::json::value_to<std::string>(json.at("pattern"));
    std::vector<std::string> outputs = boost::json::value_to<std::vector<std::string>>(json.at("logOutPut"));
    std::vector<logger> loggers = boost::json::value_to<std::vector<logger>>(json.at("loggers"));
    logger root = boost::json::value_to<logger>(json.at("rootLogger"));
    return log4cpp_config{pattern, out, loggers, root};
}

log4cpp_config log4cpp_config::load_config(const std::string &json_file)
{
    std::ifstream ifs;
    ifs.open(json_file, std::ios::in);
    if (!ifs.is_open())
    {
        std::cerr << "JSON file opening failed" << std::endl;
        throw std::filesystem::filesystem_error("JSON file " + json_file + "opening failed",
                                                std::make_error_code(std::io_errc::stream));
    }
    std::string a_string;
    boost::json::error_code error_code;
    boost::json::parse_options parse_options{};
    parse_options.allow_comments = true;
    boost::json::storage_ptr sp{};
    boost::json::stream_parser sparser(sp, parse_options);
    while (ifs.good())
    {
        char buffer[4096];
        ifs.read(buffer, sizeof(buffer) - 1);
        size_t read_size = ifs.gcount();
        if (read_size > 0)
        {
            buffer[read_size] = '\0';
            a_string += std::string(buffer);
            sparser.write(a_string, error_code);
            if (error_code)
            {
                break;
            }
        }
    }
    ifs.close();
    if (error_code)
    {
        throw std::invalid_argument("JSON parse failed! " + error_code.message());
    }
    sparser.finish();
    boost::json::value json_value = sparser.release();
    return boost::json::value_to<log4cpp_config>(json_value);
}

std::string log4cpp::serialize(const log4cpp_config &obj)
{
    // 将对象序列化为JSON
    boost::json::value json = boost::json::value_from(obj);
    // 将JSON反序列化
    return boost::json::serialize(json);
}

log4cpp_config::log4cpp_config(std::string _pattern, output &o, const std::vector<logger> &l, logger root): pattern(
        std::move(_pattern)), _output(o), loggers(l), root_logger(std::move(root))
{}

log4cpp_config::log4cpp_config(const log4cpp_config &other)
{
    this->pattern = other.pattern;
    this->_output = other._output;
    this->loggers = other.loggers;
    this->root_logger = other.root_logger;
}

log4cpp_config::log4cpp_config(log4cpp_config &&other) noexcept
{
    this->pattern = other.pattern;
    this->_output = std::move(other._output);
    this->loggers = other.loggers;
    this->root_logger = other.root_logger;
}

log4cpp_config &log4cpp_config::operator=(const log4cpp_config &other)
{
    if (this != &other)
    {
        this->pattern = other.pattern;
        this->_output = other._output;
        this->loggers = other.loggers;
        this->root_logger = other.root_logger;
    }
    return *this;
}

log4cpp_config &log4cpp_config::operator=(log4cpp_config &&other) noexcept
{
    if (this != &other)
    {
        this->pattern = other.pattern;
        this->_output = std::move(other._output);
        this->loggers = other.loggers;
        this->root_logger = other.root_logger;
    }
    return *this;
}

logger_config log4cpp_config::get_logger_config(const std::string &name)
{
    for (auto &&config: loggers)
    {
        if (config.get_logger_name() == name)
        {
            return config;
        }
    }
    return root_logger;
}
