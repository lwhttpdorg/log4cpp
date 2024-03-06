#if defined(_WIN32)

#include <io.h>

#endif

#include "logger_builder.h"

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

logger_builder::builder &logger_builder::builder::set_console_output(log_output *output) {
	if (output != nullptr) {
		if (typeid(*output) != typeid(console_output)) {
			throw std::runtime_error("output is not an instance of console_output");
		}
		this->log->outputs.push_back(output);
	}
	return *this;
}

logger_builder::builder &logger_builder::builder::set_file_output(log_output *output) {
	if (output != nullptr) {
		if (typeid(*output) != typeid(file_output)) {
			throw std::runtime_error("output is not an instance of file_output");
		}
		this->log->outputs.push_back(output);
	}
	return *this;
}

logger * logger_builder::builder::build() {
	return this->log;
}
