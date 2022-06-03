#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdarg>
#include <pthread.h>
#include <iostream>

#include "utils.hpp"
#include "log4cpp.hpp"

class LockSingleton
{
public:
	static LockSingleton &getInstance()
	{
		static LockSingleton instance;
		return instance;
	}

	LockSingleton(const LockSingleton &obj) = delete;

	LockSingleton &operator=(const LockSingleton &) = delete;

private:
	LockSingleton()
	{
		pthread_spin_init(&spinlock, PTHREAD_PROCESS_PRIVATE);
	}

	virtual ~LockSingleton()
	{
		pthread_spin_destroy(&spinlock);
	}

public:
	pthread_spinlock_t spinlock{};
};

static std::string to_string(log_level level)
{
	std::string str;
	switch (level)
	{
		case log_level::FATAL:
			str = "FATAL";
			break;
		case log_level::ERROR:
			str = "ERROR";
			break;
		case log_level::WARN:
			str = "WARN";
			break;
		case log_level::INFO:
			str = "INFO";
			break;
		case log_level::DEBUG:
			str = "DEBUG";
			break;
		case log_level::TRACE:
			str = "TRACE";
			break;
	}
	return str;
}

logger::logger(log_level lv)
{
	this->level = lv;
	this->log_fd = STDOUT_FILENO;
	this->log_prefix = "";
}

logger::logger(const std::string &log_file, log_level lv)
{
	this->level = lv;
	mode_t mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH;
	int fd = open(log_file.c_str(), O_RDWR|O_APPEND|O_CLOEXEC|O_CREAT, mode);
	if (fd != -1)
	{
		this->log_fd = fd;
	}
	else
	{
		std::cerr << "can not open or create log file " << log_file << ", Use STDOUT instead: " << strerror(errno)
		          << "(" << errno << ")"
		          << std::endl;
		this->log_fd = STDOUT_FILENO;
	}
	this->log_prefix = "";
}

logger::~logger()
{
	close(log_fd);
}

logger::logger(int fd, log_level lv)
{
	this->level = lv;
	if (fd != -1)
	{
		this->log_fd = fd;
	}
	else
	{
		std::cerr << "invalid log fd " << fd << std::endl;
	}
	this->log_prefix = "";
}

ssize_t logger::get_outset(log_level lv, char *buf, ssize_t len)
{
	ssize_t used_len = 0;
	time_t time_now;
	time(&time_now);
	tm *tm_now = localtime(&time_now);
	used_len += scnprintf(buf + used_len, len - used_len, "%d-%d-%d %02d:%02d:%02d %s ",
	                      1900 + tm_now->tm_year,
	                      tm_now->tm_mon + 1,
	                      tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, tm_now->tm_zone);
	used_len += scnprintf(buf + used_len, len - used_len, "%-05s -- ", to_string(lv).c_str());
	if (!this->log_prefix.empty())
	{
		used_len += scnprintf(buf + used_len, len - used_len, "[%08s]: ", this->log_prefix.c_str());
	}
	else
	{
		used_len += scnprintf(buf + used_len, len - used_len, "[%08s]: ", " ");
	}
	return used_len;
}

void logger::log(log_level lv, const char *fmt, ...)
{
	if (lv <= this->level)
	{
		char buf[LOG_LINE_MAX];
		ssize_t used_len = 0, buf_len = sizeof(buf);
		buf[0] = '\0';
		used_len += get_outset(lv, buf, buf_len);
		va_list args;
		va_start(args, fmt);
		used_len += vscnprintf(buf + used_len, buf_len - used_len, fmt, args);
		va_end(args);
		used_len += scnprintf(buf + used_len, buf_len - used_len, "\n");
		pthread_spinlock_t lock = LockSingleton::getInstance().spinlock;
		pthread_spin_lock(&lock);
		write(this->log_fd, buf, used_len);
		pthread_spin_unlock(&lock);
	}
}

void logger::log(log_level lv, const char *fmt, va_list args)
{
	if (lv <= this->level)
	{
		char buf[LOG_LINE_MAX];
		ssize_t used_len = 0, buf_len = sizeof(buf);
		buf[0] = '\0';
		used_len += get_outset(lv, buf, buf_len);
		used_len += vscnprintf(buf + used_len, buf_len - used_len, fmt, args);
		used_len += scnprintf(buf + used_len, buf_len - used_len, "\n");
		pthread_spinlock_t lock = LockSingleton::getInstance().spinlock;
		pthread_spin_lock(&lock);
		write(this->log_fd, buf, used_len);
		pthread_spin_unlock(&lock);
	}
}

void logger::log_fatal(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->log(log_level::FATAL, fmt, args);
	va_end(args);
}

void logger::log_error(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->log(log_level::ERROR, fmt, args);
	va_end(args);
}

void logger::log_warn(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->log(log_level::WARN, fmt, args);
	va_end(args);
}

void logger::log_info(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->log(log_level::INFO, fmt, args);
	va_end(args);
}

void logger::log_debug(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->log(log_level::DEBUG, fmt, args);
	va_end(args);
}

void logger::log_trace(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	this->log(log_level::TRACE, fmt, args);
	va_end(args);
}
