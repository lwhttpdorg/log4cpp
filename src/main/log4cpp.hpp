#ifndef LOG4CPP_HPP
#define LOG4CPP_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>

#include <yaml-cpp/yaml.h>

#define LOG_LINE_MAX 512

enum class LogLevel
{
	FATAL = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4,
	TRACE = 5
};

class Outputter
{
public:
	static size_t makePrefix(LogLevel level, char *__restrict buf, size_t len);

	virtual void log(LogLevel level, const char *__restrict fmt, va_list args) = 0;

	virtual void log(LogLevel level, const char *__restrict fmt, ...) = 0;

	virtual ~Outputter() = default;
};

class ConsoleOutputter : public Outputter
{
public:
	explicit ConsoleOutputter(LogLevel level = LogLevel::ERROR);

	void log(LogLevel level, const char *__restrict fmt, va_list args) override;

	void log(LogLevel level, const char *__restrict fmt, ...) override;

public:
	LogLevel logLevel;
};

class FileOutputter : public Outputter
{
public:
	explicit FileOutputter(const std::string &file, bool async = true, bool append = true);

	void log(LogLevel level, const char *__restrict fmt, va_list args) override;

	void log(LogLevel level, const char *__restrict fmt, ...) override;

	~FileOutputter() override;

public:
	std::string filePath;
	bool async;
	bool append;
private:
	int fd;
};

struct RootLogger
{
	std::string pattern;
	LogLevel logLevel;
	bool consoleOutputterEnabled;
	ConsoleOutputter *consoleOutputter;
	bool fileOutputterEnabled;
	FileOutputter *fileOutputter;

	RootLogger()
	{
		this->logLevel = LogLevel::ERROR;
		this->consoleOutputterEnabled = false;
		this->consoleOutputter = nullptr;
		this->fileOutputterEnabled = false;
		this->fileOutputter = nullptr;
	}
};

class Logger
{
public:
	Logger();

	explicit Logger(const std::string &string);

	void fatal(const char *__restrict fmt, ...);

	void error(const char *__restrict fmt, ...);

	void warn(const char *__restrict fmt, ...);

	void info(const char *__restrict fmt, ...);

	void debug(const char *__restrict fmt, ...);

	void trace(const char *__restrict fmt, ...);

	virtual ~Logger();

	friend struct YAML::convert<Logger>;

	friend class LoggerBuilder;

	friend class LoggerManager;

private:
	std::string name;
	LogLevel logLevel;
	bool consoleOutputterEnabled;
	Outputter *consoleOutputter;
	bool fileOutputterEnabled;
	Outputter *fileOutputter;
};

/*********************** LoggerBuilder ***********************/
class LoggerManager
{
public:
	static void setYamlFilePath(const std::string &yaml);

	static Logger getLogger(const std::string &name);

private:
	LoggerManager() = default;

private:
	class InnerInit
	{
	public:
		InnerInit();

		~InnerInit();
	};

private:
	static InnerInit init;
	static pthread_spinlock_t spinlock;
	static std::unordered_map<std::string, Logger> loggers;
};

#endif //LOG4CPP_HPP
