#include <memory>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "logger_builder.h"

using namespace log4cpp;


layout_builder::builder layout_builder::builder::new_builder() {
	builder builder = layout_builder::builder{};
	builder._layout = std::make_shared<layout>();
	return builder;
}

layout_builder::builder &layout_builder::builder::set_name(const std::string &name) {
	this->_layout->name = name;
	return *this;
}

layout_builder::builder &layout_builder::builder::set_log_level(log_level level) {
	this->_layout->level = level;
	return *this;
}

layout_builder::builder &layout_builder::builder::set_console_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<console_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(console_appender).name();
			throw std::runtime_error(what);
		}
		this->_layout->appenders.push_back(appender);
	}
	return *this;
}

layout_builder::builder &layout_builder::builder::set_file_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<file_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(file_appender).name();
			throw std::runtime_error(what);
		}
		this->_layout->appenders.push_back(appender);
	}
	return *this;
}

layout_builder::builder &layout_builder::builder::set_tcp_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<tcp_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(tcp_appender).name();
			throw std::runtime_error(what);
		}
		this->_layout->appenders.push_back(appender);
	}
	return *this;
}

layout_builder::builder &layout_builder::builder::set_udp_appender(const std::shared_ptr<log_appender> &appender) {
	if (appender != nullptr) {
		if (nullptr == dynamic_cast<udp_appender *>(appender.get())) {
			std::string what = "'appender' not an instance of ";
			what += typeid(udp_appender).name();
			throw std::runtime_error(what);
		}
		this->_layout->appenders.push_back(appender);
	}
	return *this;
}

std::shared_ptr<layout> layout_builder::builder::build() {
	if (this->_layout == nullptr) {
		throw std::runtime_error("Call new_builder() first");
	}
	return this->_layout;
}
