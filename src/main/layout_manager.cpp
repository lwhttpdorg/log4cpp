#include <filesystem>

#include "log4cpp.hpp"

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "log4cpp_config.h"
#include "logger_builder.h"

#if defined(_WIN32)
#include <io.h>
#endif

#ifdef _MSC_VER
#define F_OK 0
#endif

using namespace log4cpp;

/* Banner */
constexpr char BANNER[] = R"(
   __    ___   ___  _  _      ___
  / /   /___\ / _ \| || |    / __\  _      _
 / /   //  /// /_\/| || |_  / /   _| |_  _| |_
/ /___/ \_/// /_\\ |__   _|/ /___|_   _||_   _|
\____/\___/ \____/    |_|  \____/  |_|    |_|
)";

bool layout_manager::initialized = false;
log4cpp_config layout_manager::config;
std::shared_ptr<log_appender> layout_manager::console_appender = nullptr;
std::shared_ptr<log_appender> layout_manager::file_appender = nullptr;
std::shared_ptr<log_appender> layout_manager::tcp_appender = nullptr;
std::shared_ptr<log_appender> layout_manager::udp_appender = nullptr;
std::unordered_map<std::string, std::shared_ptr<logger>> layout_manager::layouts;
std::shared_ptr<logger> layout_manager::root_layout = nullptr;

void layout_manager::load_config(const std::string &json_filepath) {
	if (-1 != access(json_filepath.c_str(), F_OK)) {
		config = log4cpp_config::load_config(json_filepath);
		initialized = true;
	}
	else {
		throw std::filesystem::filesystem_error("Config file " + json_filepath + " opening failed!",
												std::make_error_code(std::io_errc::stream));
	}
}

const log4cpp_config *layout_manager::get_config() {
	return &config;
}

log_lock layout_manager::lock;

std::shared_ptr<logger> layout_manager::get_layout(const std::string &name) {
	if (!initialized) {
		std::lock_guard guard(lock);
		if (!initialized) {
			fprintf(stdout, "%s\n", BANNER);
			fflush(stdout);
			config = log4cpp_config::load_config("./log4cpp.json");
			initialized = true;
		}
	}
	if (layouts.empty()) {
		std::lock_guard guard(lock);
		if (layouts.empty()) {
			build_appender();
			build_layout();
			build_root_layout();
		}
	}
	if (layouts.find(name) == layouts.end()) {
		return root_layout;
	}
	return layouts.at(name);
}

void layout_manager::build_appender() {
	appender_config appender_cfg = config.appender;
	if (appender_cfg.APPENDER_FLAGS & CONSOLE_APPENDER_FLAG) {
		console_appender =
			std::shared_ptr<log_appender>(console_appender_config::get_instance(appender_cfg.console_cfg));
	}
	if (appender_cfg.APPENDER_FLAGS & FILE_APPENDER_FLAG) {
		file_appender = std::shared_ptr<log_appender>(file_appender_config::get_instance(appender_cfg.file_cfg));
	}
	if (appender_cfg.APPENDER_FLAGS & TCP_APPENDER_FLAG) {
		tcp_appender = std::shared_ptr<log_appender>(tcp_appender_config::get_instance(appender_cfg.tcp_cfg));
	}
	if (appender_cfg.APPENDER_FLAGS & UDP_APPENDER_FLAG) {
		udp_appender = std::shared_ptr<log_appender>(udp_appender_config::get_instance(appender_cfg.udp_cfg));
	}
}

void layout_manager::build_layout() {
	for (auto &x: config.layouts) {
		layout_builder::builder builder = layout_builder::builder::new_builder();
		builder.set_name(x.get_logger_name());
		builder.set_log_level(x.get_logger_level());
		const auto flags = x.get_layout_flag();
		if ((flags & CONSOLE_APPENDER_FLAG) != 0) {
			builder.set_console_appender(console_appender);
		}
		else {
			builder.set_console_appender(nullptr);
		}
		if ((flags & FILE_APPENDER_FLAG) != 0) {
			builder.set_file_appender(file_appender);
		}
		else {
			builder.set_file_appender(nullptr);
		}
		if ((flags & TCP_APPENDER_FLAG) != 0) {
			builder.set_tcp_appender(tcp_appender);
		}
		else {
			builder.set_tcp_appender(nullptr);
		}
		if ((flags & UDP_APPENDER_FLAG) != 0) {
			builder.set_udp_appender(udp_appender);
		}
		else {
			builder.set_udp_appender(nullptr);
		}
		std::shared_ptr<layout> layout = builder.build();
		layouts[x.get_logger_name()] = std::make_shared<layout_proxy>(layout);
	}
}

void layout_manager::build_root_layout() {
	const layout_config root_log_cfg = config.root_layout;
	layout_builder::builder builder = layout_builder::builder::new_builder();
	builder.set_name("root");
	builder.set_log_level(root_log_cfg.get_logger_level());
	const auto flags = root_log_cfg.get_layout_flag();
	if ((flags & CONSOLE_APPENDER_FLAG) != 0) {
		builder.set_console_appender(console_appender);
	}
	else {
		builder.set_console_appender(nullptr);
	}
	if ((flags & FILE_APPENDER_FLAG) != 0) {
		builder.set_file_appender(file_appender);
	}
	else {
		builder.set_file_appender(nullptr);
	}
	if ((flags & TCP_APPENDER_FLAG) != 0) {
		builder.set_tcp_appender(tcp_appender);
	}
	else {
		builder.set_tcp_appender(nullptr);
	}
	if ((flags & UDP_APPENDER_FLAG) != 0) {
		builder.set_udp_appender(udp_appender);
	}
	else {
		builder.set_udp_appender(nullptr);
	}
	std::shared_ptr<layout> root_logger = builder.build();
	root_layout = std::make_shared<layout_proxy>(root_logger);
}
