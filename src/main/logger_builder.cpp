#include <memory>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "logger_builder.h"

using namespace log4cpp;

logger_builder::builder logger_builder::builder::new_builder() {
	builder builder = logger_builder::builder{};
	builder._logger = std::make_shared<core_logger>();
	return builder;
}

logger_builder::builder &logger_builder::builder::set_name(const std::string &name) {
	this->_logger->name = name;
	return *this;
}

logger_builder::builder &logger_builder::builder::set_log_level(log_level level) {
	this->_logger->level = level;
	return *this;
}

logger_builder::builder &logger_builder::builder::set_console_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<console_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(console_appender).name();
			throw std::runtime_error(what);
		}
		this->_logger->appenders.push_back(appender);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_file_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<file_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(file_appender).name();
			throw std::runtime_error(what);
		}
		this->_logger->appenders.push_back(appender);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_tcp_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<tcp_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(tcp_appender).name();
			throw std::runtime_error(what);
		}
		this->_logger->appenders.push_back(appender);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_udp_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<udp_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(udp_appender).name();
			throw std::runtime_error(what);
		}
		this->_logger->appenders.push_back(appender);
	}
	return *this;
}

std::shared_ptr<logger> logger_builder::builder::build() {
	if (this->_logger == nullptr) {
		throw std::runtime_error("Call new_builder() first");
	}
	return this->_logger;
}
