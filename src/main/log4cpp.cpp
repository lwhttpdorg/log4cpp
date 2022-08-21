#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cstdarg>
#include <pthread.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include "../include/log4cpp.hpp"
#include "LogConfiger.h"

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

static std::string to_string(LogLevel level)
{
	std::string str;
	switch (level)
	{
		case LogLevel::FATAL:
			str = "FATAL";
			break;
		case LogLevel::ERROR:
			str = "ERROR";
			break;
		case LogLevel::WARN:
			str = "WARN";
			break;
		case LogLevel::INFO:
			str = "INFO";
			break;
		case LogLevel::DEBUG:
			str = "DEBUG";
			break;
		case LogLevel::TRACE:
			str = "TRACE";
			break;
	}
	return str;
}

static size_t log4c_vscnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, va_list args)
{
	int i = vsnprintf(buf, size, fmt, args);
	return (static_cast<size_t>(i) >= size)?(size - 1):i;
}

static size_t log4c_scnprintf(char *__restrict buf, size_t size, const char *__restrict fmt, ...)
{
	va_list args;
	int i;
	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return (static_cast<size_t>(i) >= size)?(size - 1):i;
}

/************************* Outputter *************************/
ConsoleOutputter::ConsoleOutputter(LogLevel level)
{
	this->logLevel = level;
}

size_t Outputter::makePrefix(LogLevel level, char *buf, size_t len)
{
	size_t used_len = 0;
	timeval tv{};
	gettimeofday(&tv, nullptr);
	time_t tm_now = tv.tv_sec;
	tm *local = localtime(&tm_now);
	unsigned short ms = tv.tv_usec / 1000;
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "%d-%d-%d %02d:%02d:%02d.%3d %s ",
	                            1900 + local->tm_year, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min,
	                            local->tm_sec, ms,
	                            local->tm_zone);
	char thread_name[16];
	thread_name[0] = '\0';
#ifdef _GNU_SOURCE
	pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
#elif defined(__linux__)
	prctl(PR_GET_NAME, (unsigned long)thread_name);
#endif
	if (thread_name[0] == '\0')
	{
		log4c_scnprintf(thread_name, sizeof(thread_name), "%u", gettid());
	}
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "[%16s]: ", thread_name);
	used_len += log4c_scnprintf(buf + used_len, len - used_len, "[%-5s] -- ", to_string(level).c_str());
	return used_len;
}

void ConsoleOutputter::log(LogLevel level, const char *fmt, va_list args)
{
	if (level <= this->logLevel)
	{
		char buffer[LOG_LINE_MAX];
		size_t used_len = 0, buf_len = sizeof(buffer);
		buffer[0] = '\0';
		used_len += makePrefix(level, buffer, buf_len);
		used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
		used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
		pthread_spinlock_t lock = LockSingleton::getInstance().spinlock;
		pthread_spin_lock(&lock);
		write(STDOUT_FILENO, buffer, used_len);
		pthread_spin_unlock(&lock);
	}
}

void ConsoleOutputter::log(LogLevel level, const char *fmt, ...)
{
	if (level <= this->logLevel)
	{
		char buffer[LOG_LINE_MAX];
		size_t used_len = 0, buf_len = sizeof(buffer);
		buffer[0] = '\0';
		used_len += makePrefix(level, buffer, buf_len);
		va_list args;
		va_start(args, fmt);
		used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
		va_end(args);
		used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
		pthread_spinlock_t lock = LockSingleton::getInstance().spinlock;
		pthread_spin_lock(&lock);
		write(STDOUT_FILENO, buffer, used_len);
		pthread_spin_unlock(&lock);
	}
}

FileOutputter::FileOutputter(const std::string &file, bool async, bool append)
{
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	this->fd = open(file.c_str(), O_RDWR | O_APPEND | O_CLOEXEC | O_CREAT, mode);
	if (this->fd == -1)
	{
		std::string what("Can not open log file, ");
		what.append(strerror(errno));
		what.append("(" + std::to_string(errno) + ")");
		throw std::runtime_error(what);
	}
	this->filePath = file;
	this->async = async;
	this->append = append;
}

FileOutputter::~FileOutputter()
{
	if (this->fd != -1)
	{
		close(this->fd);
	}
}

void FileOutputter::log(LogLevel level, const char *fmt, va_list args)
{
	char buffer[LOG_LINE_MAX];
	size_t used_len = 0, buf_len = sizeof(buffer);
	buffer[0] = '\0';
	used_len += makePrefix(level, buffer, buf_len);
	used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
	used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
	pthread_spinlock_t lock = LockSingleton::getInstance().spinlock;
	pthread_spin_lock(&lock);
	write(this->fd, buffer, used_len);
	pthread_spin_unlock(&lock);
}

void FileOutputter::log(LogLevel level, const char *fmt, ...)
{
	char buffer[LOG_LINE_MAX];
	size_t used_len = 0, buf_len = sizeof(buffer);
	buffer[0] = '\0';
	used_len += makePrefix(level, buffer, buf_len);
	va_list args;
	va_start(args, fmt);
	used_len += log4c_vscnprintf(buffer + used_len, buf_len - used_len, fmt, args);
	va_end(args);
	used_len += log4c_scnprintf(buffer + used_len, buf_len - used_len, "\n");
	pthread_spinlock_t lock = LockSingleton::getInstance().spinlock;
	pthread_spin_lock(&lock);
	write(this->fd, buffer, used_len);
	pthread_spin_unlock(&lock);
}

/**************************Logger*****************************/
Logger::Logger()
{
	this->logLevel = LogLevel::ERROR;
	this->consoleOutputter = nullptr;
	this->fileOutputter = nullptr;
	this->consoleOutputterEnabled = false;
	this->fileOutputterEnabled = false;
}

Logger::Logger(const std::string &name)
{
	this->name = name;
	this->logLevel = LogLevel::ERROR;
	this->consoleOutputter = nullptr;
	this->fileOutputter = nullptr;
	this->consoleOutputterEnabled = false;
	this->fileOutputterEnabled = false;
}

Logger::~Logger()
{
	this->consoleOutputter = nullptr;
	this->fileOutputter = nullptr;
	this->consoleOutputterEnabled = false;
	this->fileOutputterEnabled = false;
}

void Logger::fatal(const char *__restrict fmt, ...)
{
	if (this->logLevel >= LogLevel::FATAL)
	{
		if (this->consoleOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->consoleOutputter->log(LogLevel::FATAL, fmt, args);
			va_end(args);
		}
		if (this->fileOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->fileOutputter->log(LogLevel::FATAL, fmt, args);
			va_end(args);
		}
	}
}

void Logger::error(const char *__restrict fmt, ...)
{
	if (this->logLevel >= LogLevel::ERROR)
	{
		if (this->consoleOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->consoleOutputter->log(LogLevel::ERROR, fmt, args);
			va_end(args);
		}
		if (this->fileOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->fileOutputter->log(LogLevel::ERROR, fmt, args);
			va_end(args);
		}
	}
}

void Logger::warn(const char *__restrict fmt, ...)
{
	if (this->logLevel >= LogLevel::WARN)
	{
		if (this->consoleOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->consoleOutputter->log(LogLevel::WARN, fmt, args);
			va_end(args);
		}
		if (this->fileOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->fileOutputter->log(LogLevel::WARN, fmt, args);
			va_end(args);
		}
	}
}

void Logger::info(const char *__restrict fmt, ...)
{
	if (this->logLevel >= LogLevel::INFO)
	{
		if (this->consoleOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->consoleOutputter->log(LogLevel::INFO, fmt, args);
			va_end(args);
		}
		if (this->fileOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->fileOutputter->log(LogLevel::INFO, fmt, args);
			va_end(args);
		}
	}
}

void Logger::debug(const char *__restrict fmt, ...)
{
	if (this->logLevel >= LogLevel::DEBUG)
	{
		if (this->consoleOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->consoleOutputter->log(LogLevel::DEBUG, fmt, args);
			va_end(args);
		}
		if (this->fileOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->fileOutputter->log(LogLevel::DEBUG, fmt, args);
			va_end(args);
		}
	}
}

void Logger::trace(const char *__restrict fmt, ...)
{
	if (this->logLevel >= LogLevel::TRACE)
	{
		if (this->consoleOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->consoleOutputter->log(LogLevel::TRACE, fmt, args);
			va_end(args);
		}
		if (this->fileOutputter != nullptr)
		{
			va_list args;
			va_start(args, fmt);
			this->fileOutputter->log(LogLevel::TRACE, fmt, args);
			va_end(args);
		}
	}
}

/*********************** LoggerBuilder ***********************/
class LoggerBuilder
{
public:
	class Builder
	{
	public:
		Builder &setName(const std::string &name);

		Builder &setLogLevel(LogLevel level);

		Builder &setConsoleOutputter(Outputter *consoleOutputter);

		Builder &setFileOutputter(Outputter *fileOutputter);

		Logger build();

	private:
		Logger logger;
	};

	friend class LoggerManager;

public:
	static std::string getYamlFilePath();

	static void setYamlFilePath(const std::string &yaml);

	static Builder newBuilder();

	static Logger getLogger(const std::string &name);

private:
	LoggerBuilder();

private:
	static Log4CppConfiger log4CppConfiger;
	static std::string yamlFilePath;
};

Log4CppConfiger LoggerBuilder::log4CppConfiger;
std::string LoggerBuilder::yamlFilePath;

LoggerBuilder::LoggerBuilder()
{
	std::string defaultYaml = "./log4cpp.yml";
	if (-1 != access(defaultYaml.c_str(), F_OK))
	{
		LoggerBuilder::yamlFilePath = defaultYaml;
		LoggerBuilder::log4CppConfiger.loadYamlConfig(LoggerBuilder::yamlFilePath);
	}
}

void LoggerBuilder::setYamlFilePath(const std::string &yaml)
{
	if (-1 != access(yaml.c_str(), F_OK))
	{
		LoggerBuilder::yamlFilePath = yaml;
		LoggerBuilder::log4CppConfiger.loadYamlConfig(LoggerBuilder::yamlFilePath);
		for (Logger &logger:LoggerBuilder::log4CppConfiger.loggers)
		{
			if (logger.consoleOutputterEnabled)
			{
				logger.consoleOutputter = LoggerBuilder::log4CppConfiger.consoleOutputter;
			}
			if (logger.fileOutputterEnabled)
			{
				logger.fileOutputter = LoggerBuilder::log4CppConfiger.fileOutputter;
			}
		}
	}
	else
	{
		std::string what("Can not open the YAML file, ");
		what.append(strerror(errno));
		what.append("(" + std::to_string(errno) + ")");
		throw std::runtime_error(what);
	}
}

std::string LoggerBuilder::getYamlFilePath()
{
	return LoggerBuilder::yamlFilePath;
}

LoggerBuilder::Builder LoggerBuilder::newBuilder()
{
	return LoggerBuilder::Builder{};
}

Logger LoggerBuilder::getLogger(const std::string &name)
{
	auto it = std::find_if(log4CppConfiger.loggers.begin(), log4CppConfiger.loggers.end(),
	                       [&name](Logger const &logger) { return logger.name == name; });
	if (it != log4CppConfiger.loggers.cend())
	{
		Builder builder = LoggerBuilder::newBuilder();
		builder.setName(name);
		builder.setLogLevel(it->logLevel);
		builder.setConsoleOutputter(it->consoleOutputter);
		builder.setFileOutputter(it->fileOutputter);
		return builder.build();
	}
	else
	{
		Outputter *consoleOutputter = log4CppConfiger.rootLogger.consoleOutputter;
		Outputter *fileOutputter = log4CppConfiger.rootLogger.fileOutputter;
		Builder builder = LoggerBuilder::newBuilder();
		builder.setName(name);
		builder.setLogLevel(LogLevel::ERROR);
		builder.setConsoleOutputter(consoleOutputter);
		builder.setFileOutputter(fileOutputter);
		return builder.build();
	}
}

LoggerBuilder::Builder &LoggerBuilder::Builder::setName(const std::string &name)
{
	this->logger.name = name;
	return *this;
}

LoggerBuilder::Builder &LoggerBuilder::Builder::setLogLevel(LogLevel level)
{
	this->logger.logLevel = level;
	return *this;
}

LoggerBuilder::Builder &LoggerBuilder::Builder::setConsoleOutputter(Outputter *consoleOutputter)
{
	this->logger.consoleOutputter = consoleOutputter;
	return *this;
}

LoggerBuilder::Builder &LoggerBuilder::Builder::setFileOutputter(Outputter *fileOutputter)
{
	this->logger.fileOutputter = fileOutputter;
	return *this;
}

Logger LoggerBuilder::Builder::build()
{
	return this->logger;
}

/***********************LoggerManager*************************/
pthread_spinlock_t LoggerManager::spinlock;
std::unordered_map<std::string, Logger> LoggerManager::loggers;
LoggerManager::InnerInit LoggerManager::init;

LoggerManager::InnerInit::InnerInit()
{
	pthread_spin_init(&LoggerManager::spinlock, 0);
	std::string yamlFile = "./log4cpp.yml";
}

LoggerManager::InnerInit::~InnerInit()
{
	pthread_spin_destroy(&LoggerManager::spinlock);
	while (!LoggerManager::loggers.empty())
	{
		auto begin = LoggerManager::loggers.begin();
		LoggerManager::loggers.erase(begin);
	}
}

Logger LoggerManager::getLogger(const std::string &name)
{
	Logger logger;
	if (LoggerManager::loggers.find(name) == LoggerManager::loggers.end())
	{
		pthread_spin_lock(&LoggerManager::spinlock);
		if (LoggerManager::loggers.find(name) == LoggerManager::loggers.end())
		{
			Logger tmpLogger = LoggerBuilder::getLogger(name);
			LoggerManager::loggers.insert({name, tmpLogger});
		}
		logger = LoggerManager::loggers.at(name);
		pthread_spin_unlock(&LoggerManager::spinlock);
	}
	else
	{
		logger = LoggerManager::loggers.at(name);
	}
	return logger;
}

void LoggerManager::setYamlFilePath(const std::string &yaml)
{
	LoggerBuilder::setYamlFilePath(yaml);
}
