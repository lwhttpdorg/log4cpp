#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef _WIN32

#include <windows.h>

#ifdef ERROR
#undef ERROR
#endif
#endif

#ifdef _MSC_VER
#include <processthreadsapi.h>
#include "log_utils.h"
#elif defined(__GNUC__)

#include <pthread.h>

#else
#include <sys/prctl.h>
#endif

#ifdef __linux__

#include <unistd.h>

#endif

#include <cstdarg>

#include <boost/algorithm/string.hpp>
#include <boost/json.hpp>

#include "../include/log4cpp.hpp"
#include "file_appender.h"
#include "layout_pattern.h"

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
	if (const auto tmp = boost::algorithm::to_upper_copy(s); tmp == LOG_LEVEL_FATAL) {
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

/************************thread name*************************/
unsigned long log4cpp::get_thread_name_id(char *thread_name, size_t len) {
	thread_name[0] = '\0';
#ifdef _MSC_VER
	(void)len;
	unsigned long tid = GetCurrentThreadId();
	PWSTR thread_name_ptr;
	HRESULT hresult = GetThreadDescription(GetCurrentThread(), &thread_name_ptr);
	if (SUCCEEDED(hresult)) {
		std::wstring w_name(thread_name_ptr);
		LocalFree(thread_name_ptr);
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, w_name.c_str(), -1, nullptr, 0, nullptr, nullptr);
		std::string name(size_needed, '\0');
		WideCharToMultiByte(CP_UTF8, 0, w_name.c_str(), -1, &name[0], size_needed, nullptr, nullptr);
		log4c_scnprintf(thread_name, len, "%s", name.c_str());
	}
#endif

#ifdef __GNUC__
	unsigned long tid = pthread_self();
	pthread_getname_np(pthread_self(), thread_name, len);
#elif __linux__
	unsigned long tid = gettid();
	(void)len;
	prctl(PR_GET_NAME, reinterpret_cast<unsigned long>(thread_name));
#endif
	return tid;
}

void log4cpp::set_thread_name(const char *name) {
#ifdef _MSC_VER
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
	std::wstring wchar_str(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, name, -1, &wchar_str[0], size_needed);
	SetThreadDescription(GetCurrentThread(), wchar_str.c_str());
#endif
#ifdef __GNUC__
	pthread_setname_np(pthread_self(), name);
#elif __linux__
	prctl(PR_SET_NAME, reinterpret_cast<unsigned long>("child"));
#endif
}

/**************************layout*****************************/
layout::layout() {
	this->level = log_level::WARN;
}

layout::layout(const std::string &log_name, log_level _level) {
	this->name = log_name;
	this->level = _level;
}

void layout::log(log_level _level, const char *fmt, va_list args) const {
	char buffer[LOG_LINE_MAX];
	buffer[0] = '\0';
	const size_t used_len = layout_pattern::format(buffer, sizeof(buffer), _level, fmt, args);
	for (auto &l: this->appenders) {
		l->log(buffer, used_len);
	}
}

void layout::trace(const char *__restrict fmt, ...) const {
	if (this->level >= log_level::TRACE) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::TRACE, fmt, args);
		va_end(args);
	}
}

void layout::info(const char *__restrict fmt, ...) const {
	if (this->level >= log_level::INFO) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::INFO, fmt, args);
		va_end(args);
	}
}

void layout::debug(const char *__restrict fmt, ...) const {
	if (this->level >= log_level::DEBUG) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::DEBUG, fmt, args);
		va_end(args);
	}
}

void layout::warn(const char *__restrict fmt, ...) const {
	if (this->level >= log_level::WARN) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::WARN, fmt, args);
		va_end(args);
	}
}

void layout::error(const char *__restrict fmt, ...) const {
	if (this->level >= log_level::ERROR) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::ERROR, fmt, args);
		va_end(args);
	}
}

void layout::fatal(const char *__restrict fmt, ...) const {
	if (this->level >= log_level::FATAL) {
		va_list args;
		va_start(args, fmt);
		this->log(log_level::FATAL, fmt, args);
		va_end(args);
	}
}

layout::layout(const layout &other) {
	this->name = other.name;
	this->level = other.level;
	this->appenders = other.appenders;
}

layout::layout(layout &&other) noexcept {
	this->name = std::move(other.name);
	this->level = other.level;
	this->appenders = std::move(other.appenders);
}

layout &layout::operator=(const layout &other) {
	if (this != &other) {
		this->name = other.name;
		this->level = other.level;
		this->appenders = other.appenders;
	}
	return *this;
}

layout &layout::operator=(layout &&other) noexcept {
	if (this != &other) {
		this->name = std::move(other.name);
		this->level = other.level;
		this->appenders = std::move(other.appenders);
	}
	return *this;
}
