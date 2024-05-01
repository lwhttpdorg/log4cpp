#ifdef _MSC_VER
#define  _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32

#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif
#endif

#include <boost/json.hpp>
#include <boost/algorithm/string.hpp>

#include "../include/log4cpp.hpp"
#include "log4cpp_config.h"
#include "console_output.h"
#include "file_output.h"

using namespace log4cpp;

/************************** log_level *****************************/
const char *LOG_LEVEL_FATAL = "FATAL";
const char *LOG_LEVEL_ERROR = "ERROR";
const char *LOG_LEVEL_WARN = "WARN";
const char *LOG_LEVEL_INFO = "INFO";
const char *LOG_LEVEL_DEBUG = "DEBUG";
const char *LOG_LEVEL_TRACE = "TRACE";

std::string log4cpp::to_string(log_level level) {
	std::string str;
	switch (level) {
		case log_level::FATAL:
			str = LOG_LEVEL_FATAL;
			break;
		case log_level::ERROR:
			str = LOG_LEVEL_ERROR;
			break;
		case log_level::WARN:
			str = LOG_LEVEL_WARN;
			break;
		case log_level::INFO:
			str = LOG_LEVEL_INFO;
			break;
		case log_level::DEBUG:
			str = LOG_LEVEL_DEBUG;
			break;
		case log_level::TRACE:
			str = LOG_LEVEL_TRACE;
			break;
	}
	return str;
}

log_level log4cpp::from_string(const std::string &s) {
	log_level level;
	auto tmp = boost::algorithm::to_upper_copy(s);
	if (tmp == LOG_LEVEL_FATAL) {
		level = log_level::FATAL;
	}
	else if (tmp == LOG_LEVEL_ERROR) {
		level = log_level::ERROR;
	}
	else if (tmp == LOG_LEVEL_WARN) {
		level = log_level::WARN;
	}
	else if (tmp == LOG_LEVEL_INFO) {
		level = log_level::INFO;
	}
	else if (tmp == LOG_LEVEL_DEBUG) {
		level = log_level::DEBUG;
	}
	else if (tmp == LOG_LEVEL_TRACE) {
		level = log_level::TRACE;
	}
	else {
		throw std::invalid_argument("invalid loglevel: " + s);
	}
	return level;
}

/**************************log*****************************/
logger::logger() {
	this->level = log_level::WARN;
}

logger::logger(const std::string &log_name, log_level _level) {
	this->name = log_name;
	this->level = _level;
}

void logger::log(log_level _level, const char *fmt, va_list args) {
	for (auto &l:this->outputs) {
		l->log(_level, fmt, args);
	}
}

void logger::trace(const char *__restrict fmt, ...) {
	if (this->level >= log_level::TRACE) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::TRACE, fmt, args);
		va_end(args);
	}
}

void logger::info(const char *__restrict fmt, ...) {
	if (this->level >= log_level::INFO) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::INFO, fmt, args);
		va_end(args);
	}
}

void logger::debug(const char *__restrict fmt, ...) {
	if (this->level >= log_level::DEBUG) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::DEBUG, fmt, args);
		va_end(args);
	}
}

void logger::warn(const char *__restrict fmt, ...) {
	if (this->level >= log_level::WARN) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::WARN, fmt, args);
		va_end(args);
	}
}

void logger::error(const char *__restrict fmt, ...) {
	if (this->level >= log_level::ERROR) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::ERROR, fmt, args);
		va_end(args);
	}
}

void logger::fatal(const char *__restrict fmt, ...) {
	if (this->level >= log_level::FATAL) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::FATAL, fmt, args);
		va_end(args);
	}
}

logger::logger(const logger &other) {
	this->name = other.name;
	this->level = other.level;
	this->outputs = other.outputs;
}

logger::logger(logger &&other) noexcept {
	this->name = std::move(other.name);
	this->level = other.level;
	this->outputs = std::move(other.outputs);
}

logger &logger::operator=(const logger &other) {
	if (this != &other) {
		this->name = other.name;
		this->level = other.level;
		this->outputs = other.outputs;
	}
	return *this;
}

logger &logger::operator=(logger &&other) noexcept {
	if (this != &other) {
		this->name = std::move(other.name);
		this->level = other.level;
		this->outputs = std::move(other.outputs);
	}
	return *this;
}
