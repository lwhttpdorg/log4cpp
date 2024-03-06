#include <cstdarg>
#include <unistd.h>
#include "console_output.h"
#include "main/log_utils.h"


using namespace log4cpp;

int stream_name_to_file_no(const std::string &out_stream) {
	int file_no;
	if (out_stream == "stdout") {
		file_no = STDOUT_FILENO;
	}
	else if (out_stream == "stderr") {
		file_no = STDERR_FILENO;
	}
	else {
		throw std::invalid_argument(
				"Invalid consoleOutPut outStream \"" + out_stream + "\", valid _name: stdout, stderr");
	}
	return file_no;
}

console_output::builder console_output::builder::new_builder() {
	return console_output::builder{};
}

console_output::builder &console_output::builder::set_out_stream(const std::string &out_stream) {
	this->instance = new console_output(out_stream);
	return *this;
}

console_output *console_output::builder::build() {
	return this->instance;
}

console_output::console_output(const std::string &out_stream) {
	this->file_no = stream_name_to_file_no(out_stream);
}

console_output::console_output(console_output &&other) noexcept {
	this->file_no == other.file_no;
}

console_output &console_output::operator=(console_output &&other) noexcept {
	if (this != &other) {
		this->file_no = other.file_no;
	}
	return *this;
}

void console_output::log(log_level level, const char *fmt, va_list args) {
	char buffer[LOG_LINE_MAX];
	size_t used_len = 0, buf_len = sizeof(buffer);
	buffer[0] = '\0';
	used_len += build_prefix(level, buffer, buf_len);
	used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
	used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
	singleton_log_lock &lock = singleton_log_lock::get_instance();
	lock.lock();
	(void) write(this->file_no, buffer, used_len);
	lock.unlock();
}

void console_output::log(log_level level, const char *fmt, ...) {
	char buffer[LOG_LINE_MAX];
	size_t used_len = 0, buf_len = sizeof(buffer);
	buffer[0] = '\0';
	used_len += build_prefix(level, buffer, buf_len);
	va_list args;
	va_start(args, fmt);
	used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
	va_end(args);
	used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
	singleton_log_lock &lock = singleton_log_lock::get_instance();
	lock.lock();
	(void) write(this->file_no, buffer, used_len);
	lock.unlock();
}

console_output *console_output_config::get_instance(const console_output_config &config) {
	static log_lock instance_lock;
	static console_output *instance = nullptr;
	if (instance == nullptr) {
		instance_lock.lock();
		if (instance == nullptr) {
			instance = console_output::builder::new_builder().set_out_stream(config.out_stream).build();
		}
		instance_lock.unlock();
	}
	return instance;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const console_output_config &obj) {
	json = boost::json::object{{"outStream", obj.out_stream}};
}

console_output_config
log4cpp::tag_invoke(boost::json::value_to_tag<console_output_config>, boost::json::value const &json) {
	console_output_config config;
	config.out_stream = boost::json::value_to<std::string>(json.at("outStream"));
	return config;
}
