#include <csignal>
#include <filesystem>
#include <sys/eventfd.h>

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

const char *DEFAULT_CONFIG_FILE_PATH = "./log4cpp.json";

log_lock logger_manager::lock;
log4cpp_config logger_manager::config;

enum event_type {
	HOT_RELOAD = 1, SHUTDOWN = 2
};

void logger_manager::handle_sigusr2([[maybe_unused]] int sig_num) {
	auto &mgr = logger_manager::instance();
	mgr.notify_config_hot_reload();
}

bool logger_manager::enable_config_hot_loading() {
	struct sigaction sa{};

	sa.sa_handler = handle_sigusr2;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGUSR2, &sa, nullptr) == -1) {
		return false;
	}
	evt_loop_run.store(true);
	evt_fd = eventfd(0, 0);
	evt_loop_thread = std::thread(&logger_manager::event_loop, this);
	return true;
}

logger_manager::logger_manager() {
	evt_fd = -1;
	evt_loop_run = false;
	initialized = false;
	config_file_path = DEFAULT_CONFIG_FILE_PATH;
	console_appender = nullptr;
	file_appender = nullptr;
	tcp_appender = nullptr;
	udp_appender = nullptr;
	root_logger = nullptr;
}

logger_manager::~logger_manager() {
	if (evt_loop_thread.joinable()) {
		evt_loop_run.store(false);
		const uint64_t val = event_type::SHUTDOWN;
		write(evt_fd, &val, sizeof(uint64_t));
		evt_loop_thread.join();
	}
	if (evt_fd != -1) {
		close(evt_fd);
	}
}

void logger_manager::load_config(const std::string &file_path) {
	if (-1 != access(file_path.c_str(), F_OK)) {
		config = log4cpp_config::load_config(file_path);
		config_file_path = file_path;
		initialized = true;
	}
	else {
		throw std::filesystem::filesystem_error("Config file " + file_path + " opening failed!",
		                                        std::make_error_code(std::io_errc::stream));
	}
}

const log4cpp_config *logger_manager::get_config() {
	return &config; // Non-void function does not return a value
}

void logger_manager::event_loop() {
	uint64_t event;

	while (evt_loop_run.load()) {
		ssize_t s = read(evt_fd, &event, sizeof(uint64_t));
		if (s == sizeof(uint64_t)) {
			if (event_type::HOT_RELOAD == event) {
				hot_reload_config();
			}
		}
		else if (s == -1) {
			if (errno == EINTR) {
				continue;
			}
			if (errno != EBADF) {
				printf("%s: event fd read error! break event loop!\n", __func__);
			}
			break;
		}
	}
}

void logger_manager::notify_config_hot_reload() const {
	const uint64_t val = event_type::HOT_RELOAD;
	write(evt_fd, &val, sizeof(uint64_t));
}

void logger_manager::hot_reload_config() {
	std::lock_guard guard(lock);
	initialized = false;
	load_config(config_file_path);
	for (auto &logger: loggers) {
		logger.second.reset();
	}
	loggers.clear();
	root_logger.reset();
	build_appender();
	build_logger();
	build_root_logger();
}

std::shared_ptr<logger> logger_manager::get_logger(const std::string &name) {
	if (!initialized) {
		std::lock_guard guard(lock);
		if (!initialized) {
			fprintf(stdout, "%s\n", BANNER);
			fflush(stdout);
			config = log4cpp_config::load_config(config_file_path);
			initialized = true;
		}
	}
	if (loggers.empty()) {
		std::lock_guard guard(lock);
		if (loggers.empty()) {
			build_appender();
			build_logger();
			build_root_logger();
		}
	}
	if (loggers.find(name) == loggers.end()) {
		return root_logger;
	}
	return loggers.at(name);
}

void logger_manager::build_appender() {
	appender_config appender_cfg = logger_manager::config.appender;
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

void logger_manager::build_logger() {
	for (auto &x: logger_manager::config.loggers) {
		logger_builder::builder builder = logger_builder::builder::new_builder();
		builder.set_name(x.get_logger_name());
		builder.set_log_level(x.get_logger_level());
		const auto flags = x.get_logger_flag();
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
		std::shared_ptr<logger> logger = builder.build();
		auto name = x.get_logger_name();
		auto proxy = loggers[name];
		if (proxy == nullptr) {
			loggers[name] = std::make_shared<logger_proxy>(logger);
		}
		else {
			proxy->set_target(logger);
		}
	}
}

void logger_manager::build_root_logger() {
	const logger_config root_log_cfg = logger_manager::config.root_logger;
	logger_builder::builder builder = logger_builder::builder::new_builder();
	builder.set_name("root");
	builder.set_log_level(root_log_cfg.get_logger_level());
	const auto flags = root_log_cfg.get_logger_flag();
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
	std::shared_ptr<logger> target = builder.build();
	if (nullptr == root_logger) {
		root_logger = std::make_shared<logger_proxy>(target);
	}
	else {
		root_logger->set_target(target);
	}
}
