#ifdef _MSC_VER
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <fcntl.h>
#include <cstdarg>
#include <stdexcept>

#ifdef _MSC_VER
#include <windows.h>
#ifdef ERROR
#undef ERROR
#endif

#define STDOUT_FILENO _fileno(stdout)
#define F_OK 0

#endif

#if defined(_WIN32) || defined(_MSC_VER)

#include <direct.h>
#include <io.h>

#endif

#ifdef __MINGW32__

#include <sys/stat.h>

#endif

#if defined(__linux__)

#include <unistd.h>
#include <sys/stat.h>

#endif

#include "log4cpp.hpp"
#include "file_appender.h"

#include "layout_pattern.h"


using namespace log4cpp;

file_appender::builder &file_appender::builder::set_file(const std::string &file) {
	if (this->instance == nullptr) {
		throw std::runtime_error("Call file_appender::builder::new_builder() first");
	}
	this->config.file_path = file;
	return *this;
}

std::shared_ptr<file_appender> file_appender::builder::build() {
	if (this->instance == nullptr) {
		throw std::runtime_error("Call file_appender::builder::new_builder() first");
	}
	if (const auto pos = this->config.file_path.find_last_of('/'); pos != std::string::npos) {
		std::string path = this->config.file_path.substr(0, pos);
		if (0 != access(path.c_str(), F_OK)) {
#if defined(_MSC_VER) || defined(_WIN32)
			(void)_mkdir(path.c_str());
#endif
#if defined(__linux__)
			mkdir(path.c_str(), 0755);
#endif
		}
	}
	int openFlags = O_RDWR | O_CREAT | O_APPEND;
#if defined(_MSC_VER) || defined(_WIN32)
	int mode = _S_IREAD | _S_IWRITE;
#endif

#ifdef __linux__
	openFlags |= O_CLOEXEC;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
	this->instance->fd = open(this->config.file_path.c_str(), openFlags, mode);
	if (this->instance->fd == -1) {
		std::string what("Can not open log file: ");
		what.append(strerror(errno));
		what.append("(" + std::to_string(errno) + ")");
		throw std::runtime_error(what);
	}
	return this->instance;
}

file_appender::builder file_appender::builder::new_builder() {
	builder builder = file_appender::builder{};
	builder.instance = std::shared_ptr<file_appender>(new file_appender());
	return builder;
}

file_appender::~file_appender() {
	if (this->fd != -1) {
		close(this->fd);
	}
}

void file_appender::log(log_level level, const char *fmt, va_list args) {
	char buffer[LOG_LINE_MAX];
	buffer[0] = '\0';
	const size_t used_len = layout_pattern::format(buffer, sizeof(buffer), level, fmt, args);
	std::lock_guard lock_guard(this->lock);
	(void)write(this->fd, buffer, used_len);
}

void file_appender::log(log_level level, const char *fmt, ...) {
	char buffer[LOG_LINE_MAX];
	buffer[0] = '\0';
	const size_t used_len = layout_pattern::format(buffer, sizeof(buffer), level, fmt);
	std::lock_guard lock_guard(this->lock);
	(void)write(this->fd, buffer, used_len);
}

log_lock file_appender_config::instance_lock;
std::shared_ptr<file_appender> file_appender_config::instance = nullptr;

std::shared_ptr<file_appender> file_appender_config::get_instance(const file_appender_config &config) {
	if (instance == nullptr) {
		std::lock_guard lock(instance_lock);
		if (instance == nullptr) {
			instance = file_appender::builder::new_builder().set_file(config.file_path).build();
		}
	}
	return instance;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const file_appender_config &obj) {
	json = boost::json::object{
		{"filePath", obj.file_path}
	};
}

file_appender_config log4cpp::tag_invoke(boost::json::value_to_tag<file_appender_config>,
                                         boost::json::value const &json) {
	file_appender_config config;
	config.file_path = boost::json::value_to<std::string>(json.at("filePath"));
	return config;
}
