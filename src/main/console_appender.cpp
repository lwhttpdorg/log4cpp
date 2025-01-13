#if defined(__linux__)

#include <unistd.h>

#endif

#ifdef _WIN32

#include <io.h>
#include <windows.h>

#endif

#include "console_appender.h"

#ifdef _MSC_VER
#define STDOUT_FILENO _fileno(stdout)
#define STDERR_FILENO _fileno(stderr)
#endif

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
		throw std::invalid_argument("Invalid 'console_appender' out_stream \"" + out_stream
									+ "\", valid name: stdout, stderr");
	}
	return file_no;
}

console_appender::builder console_appender::builder::new_builder() {
	return builder{};
}

console_appender::builder &console_appender::builder::set_out_stream(const std::string &out_stream) {
	this->instance = std::shared_ptr<console_appender>(new console_appender(out_stream));
	return *this;
}

std::shared_ptr<console_appender> console_appender::builder::build() {
	if (this->instance == nullptr) {
		throw std::runtime_error("Call console_appender::builder::new_builder() first");
	}
	return this->instance;
}

console_appender::console_appender(const std::string &out_stream) {
	this->file_no = stream_name_to_file_no(out_stream);
}

void console_appender::log(const char *msg, size_t msg_len) {
	std::lock_guard lock_guard(this->lock);
	(void)write(this->file_no, msg, msg_len);
}

log_lock console_appender_config::instance_lock;
std::shared_ptr<console_appender> console_appender_config::instance = nullptr;

std::shared_ptr<console_appender> console_appender_config::get_instance(const console_appender_config &config) {
	if (instance == nullptr) {
		std::lock_guard lock(instance_lock);
		if (instance == nullptr) {
			instance = console_appender::builder::new_builder().set_out_stream(config.out_stream).build();
		}
	}
	return instance;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const console_appender_config &obj) {
	json = boost::json::object{{"out_stream", obj.out_stream}};
}

console_appender_config log4cpp::tag_invoke(boost::json::value_to_tag<console_appender_config>,
											boost::json::value const &json) {
	console_appender_config config;
	config.out_stream = boost::json::value_to<std::string>(json.at("out_stream"));
	return config;
}
