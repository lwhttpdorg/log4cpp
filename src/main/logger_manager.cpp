#include <filesystem>

#if defined(_WIN32)

#include <io.h>

#endif

#ifdef _MSC_VER
#include <windows.h>
#define F_OK 0
#endif

#include "log4cpp.hpp"
#include "logger_builder.h"
#include "log4cpp_config.h"

using namespace log4cpp;

bool logger_manager::initialized = false;
log4cpp_config logger_manager::config;
std::shared_ptr<log_output> logger_manager::console_out = nullptr;
std::shared_ptr<log_output> logger_manager::file_out = nullptr;
std::shared_ptr<log_output> logger_manager::tcp_out = nullptr;
std::shared_ptr<log_output> logger_manager::udp_out = nullptr;
std::unordered_map<std::string, std::shared_ptr<logger>> logger_manager::loggers;
std::shared_ptr<logger> logger_manager::root_logger = nullptr;


void logger_manager::load_config(const std::string &json_filepath) {
	if (-1 != access(json_filepath.c_str(), F_OK)) {
		config = log4cpp_config::load_config(json_filepath);
		initialized = true;
	}
	else {
		throw std::filesystem::filesystem_error("Config file " + json_filepath + " opening failed!",
		                                        std::make_error_code(std::io_errc::stream));
	}
}

std::shared_ptr<logger> logger_manager::get_logger(const std::string &name) {
	static log_lock lock;
	if (!initialized) {
		lock.lock();
		if (!initialized) {
			config = log4cpp_config::load_config("./log4cpp.json");
			initialized = true;
		}
		lock.unlock();
	}
	if (loggers.empty()) {
		lock.lock();
		if (loggers.empty()) {
			build_output();
			build_logger();
			build_root_logger();
		}
		lock.unlock();
	}
	if (loggers.find(name) == loggers.end()) {
		return root_logger;
	}
	return loggers.at(name);
}

void logger_manager::build_output() {
	output_config output_cfg = config.output;
	if (output_cfg.OUT_FLAGS & CONSOLE_OUT_CFG) {
		console_out = std::shared_ptr<log_output>(
			console_output_config::get_instance(output_cfg.console_cfg));
	}
	if (output_cfg.OUT_FLAGS & FILE_OUT_CFG) {
		file_out = std::shared_ptr<log_output>(
			file_output_config::get_instance(output_cfg.file_cfg));
	}
	if (output_cfg.OUT_FLAGS & TCP_OUT_CFG) {
		tcp_out = std::shared_ptr<log_output>(
			tcp_output_config::get_instance(output_cfg.tcp_cfg));
	}
	if (output_cfg.OUT_FLAGS & UDP_OUT_CFG) {
		udp_out = std::shared_ptr<log_output>(
			udp_output_config::get_instance(output_cfg.udp_cfg));
	}
}

void logger_manager::build_logger() {
	for (auto &x:config.loggers) {
		logger_builder::builder builder = logger_builder::builder::new_builder();
		builder.set_name(x.get_logger_name());
		builder.set_log_level(x.get_logger_level());
		const auto flags = x.get_outputs();
		if ((flags & CONSOLE_OUT_CFG) != 0) {
			builder.set_console_output(console_out);
		}
		else {
			builder.set_console_output(nullptr);
		}
		if ((flags & FILE_OUT_CFG) != 0) {
			builder.set_file_output(file_out);
		}
		else {
			builder.set_file_output(nullptr);
		}
		if ((flags & TCP_OUT_CFG) != 0) {
			builder.set_tcp_output(tcp_out);
		}
		else {
			builder.set_tcp_output(nullptr);
		}
		if ((flags & UDP_OUT_CFG) != 0) {
			builder.set_udp_output(udp_out);
		}
		else {
			builder.set_udp_output(nullptr);
		}
		loggers[x.get_logger_name()] = builder.build();
	}
}

void logger_manager::build_root_logger() {
	const logger_config root_log_cfg = config.root_logger;
	logger_builder::builder builder = logger_builder::builder::new_builder();
	builder.set_name("root");
	builder.set_log_level(root_log_cfg.get_logger_level());
	const auto flags = root_log_cfg.get_outputs();
	if ((flags & CONSOLE_OUT_CFG) != 0) {
		builder.set_console_output(console_out);
	}
	else {
		builder.set_console_output(nullptr);
	}
	if ((flags & FILE_OUT_CFG) != 0) {
		builder.set_file_output(file_out);
	}
	else {
		builder.set_file_output(nullptr);
	}
	if ((flags & TCP_OUT_CFG) != 0) {
		builder.set_tcp_output(tcp_out);
	}
	else {
		builder.set_tcp_output(nullptr);
	}
	if ((flags & UDP_OUT_CFG) != 0) {
		builder.set_udp_output(udp_out);
	}
	else {
		builder.set_udp_output(nullptr);
	}
	root_logger = builder.build();
}
