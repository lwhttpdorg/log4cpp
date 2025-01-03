#include <memory>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "logger_builder.h"

using namespace log4cpp;


logger_builder::builder logger_builder::builder::new_builder() {
	builder builder = logger_builder::builder{};
	builder.log = std::make_shared<logger>();
	return builder;
}

logger_builder::builder &logger_builder::builder::set_name(const std::string &name) {
	this->log->name = name;
	return *this;
}

logger_builder::builder &logger_builder::builder::set_log_level(log_level level) {
	this->log->level = level;
	return *this;
}

logger_builder::builder &logger_builder::builder::set_console_output(const std::shared_ptr<log_output> &output) {
	if (output != nullptr) {
		if (typeid(*output) != typeid(console_output)) {
			std::string what = typeid(*output).name();
			what += " not an instance of ";
			what += typeid(console_output).name();
			throw std::runtime_error(what);
		}
		this->log->outputs.push_back(output);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_file_output(const std::shared_ptr<log_output> &output) {
	if (output != nullptr) {
		if (typeid(*output) != typeid(file_output)) {
			std::string what = typeid(*output).name();
			what += " not an instance of ";
			what += typeid(file_output).name();
			throw std::runtime_error(what);
		}
		this->log->outputs.push_back(output);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_tcp_output(const std::shared_ptr<log_output> &output) {
	if (output != nullptr) {
		if (typeid(*output) != typeid(tcp_output)) {
			std::string what = typeid(*output).name();
			what += " not an instance of ";
			what += typeid(tcp_output).name();
			throw std::runtime_error(what);
		}
		this->log->outputs.push_back(output);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_udp_output(const std::shared_ptr<log_output> &output) {
	if (output != nullptr) {
		if (typeid(*output) != typeid(udp_output)) {
			std::string what = typeid(*output).name();
			what += " not an instance of ";
			what += typeid(udp_output).name();
			throw std::runtime_error(what);
		}
		this->log->outputs.push_back(output);
	}
	return *this;
}

std::shared_ptr<logger> logger_builder::builder::build() {
	if (this->log == nullptr) {
		throw std::runtime_error("Call new_builder() first");
	}
	return this->log;
}
