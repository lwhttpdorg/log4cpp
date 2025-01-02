#include <iostream>
#include "logger_builder.h"
#include "log_utils.h"

using namespace log4cpp;

logger_builder::builder::builder() {
	this->log = new logger();
}

logger_builder::builder logger_builder::new_builder() {
	return logger_builder::builder{};
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

logger *logger_builder::builder::build() {
	return this->log;
}
