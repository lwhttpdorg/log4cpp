#include <filesystem>

#if defined(_WIN32)

#include <io.h>

#endif
#ifdef _MSC_VER

#define F_OK 0
#endif

#include "log4cpp.hpp"
#include "logger_builder.h"
#include "log4cpp_config.h"

using namespace log4cpp;

bool logger_manager::initialized = false;
log4cpp_config logger_manager::config;
std::shared_ptr<log_output> logger_manager::console_out = nullptr;
std::shared_ptr<log_output>logger_manager::file_out = nullptr;
std::unordered_map<std::string, std::shared_ptr<logger>> logger_manager::loggers;
std::shared_ptr<logger> logger_manager::root_logger = nullptr;


void logger_manager::load_config(const std::string &json_filepath)
{
	if (-1 != access(json_filepath.c_str(), F_OK))
	{
		config = log4cpp_config::load_config(json_filepath);
		logger_manager::initialized = true;
	}
	else
	{
		throw std::filesystem::filesystem_error("Config file " + json_filepath + " opening failed!",
		                                        std::make_error_code(std::io_errc::stream));
	}
}

std::shared_ptr<logger> logger_manager::get_logger(const std::string &name)
{
	if (!logger_manager::initialized)
	{
		logger_manager::config = log4cpp_config::load_config("./log4cpp.json");
		logger_manager::initialized = true;
		//throw std::runtime_error("Not initialized: The configuration file does not exist? or forgotten load_config()?");
	}
	if (logger_manager::loggers.empty())
	{
		logger_manager::build_output();
		logger_manager::build_logger();
		logger_manager::build_root_logger();
	}
	if (logger_manager::loggers.find(name) == logger_manager::loggers.end())
	{
		return logger_manager::root_logger;
	}
	else
	{
		return logger_manager::loggers.at(name);
	}
}

void logger_manager::build_output()
{
	output_config output_cfg = logger_manager::config.output;
	if (output_cfg.OUT_FLAGS&CONSOLE_OUT_CFG)
	{
		logger_manager::console_out = std::shared_ptr<log_output>(
				log4cpp::console_output_config::get_instance(output_cfg.console_cfg));
	}
	if (output_cfg.OUT_FLAGS&FILE_OUT_CFG)
	{
		logger_manager::file_out = std::shared_ptr<log_output>(
				log4cpp::file_output_config::get_instance(output_cfg.file_cfg));
	}
}

void logger_manager::build_logger()
{
	for (auto &x:logger_manager::config.loggers)
	{
		logger_builder::builder builder = logger_builder::new_builder();
		builder.set_name(x.get_logger_name());
		builder.set_log_level(x.get_logger_level());
		auto flags = x.get_outputs();
		if ((flags&CONSOLE_OUT_CFG) != 0)
		{
			builder.set_console_output(logger_manager::console_out);
		}
		else
		{
			builder.set_console_output(nullptr);
		}
		if ((flags&FILE_OUT_CFG) != 0)
		{
			builder.set_file_output(logger_manager::file_out);
		}
		else
		{
			builder.set_file_output(nullptr);
		}
		logger *tmp_logger = builder.build();
		logger_manager::loggers[x.get_logger_name()] = std::shared_ptr<logger>(tmp_logger);
	}
}

void logger_manager::build_root_logger()
{
	logger_config root_log_cfg = config.root_logger;
	logger_builder::builder builder = logger_builder::new_builder();
	builder.set_name("root");
	builder.set_log_level(root_log_cfg.get_logger_level());
	auto flags = root_log_cfg.get_outputs();
	if ((flags&CONSOLE_OUT_CFG) != 0)
	{
		builder.set_console_output(logger_manager::console_out);
	}
	else
	{
		builder.set_console_output(nullptr);
	}
	if ((flags&FILE_OUT_CFG) != 0)
	{
		builder.set_file_output(logger_manager::file_out);
	}
	else
	{
		builder.set_file_output(nullptr);
	}
	logger_manager::root_logger = std::shared_ptr<logger>(builder.build());
}

//logger_manager::inner_garbo::~inner_garbo()
//{
//	delete logger_manager::console_out;
//	delete logger_manager::file_out;
//	for (auto &x:logger_manager::loggers)
//	{
//		delete x.second;
//	}
//	delete logger_manager::root_logger;
//}
