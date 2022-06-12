#ifndef LOG4CPP_LOG4CPP_HPP
#define LOG4CPP_LOG4CPP_HPP

#include <string>

#define LOG_LINE_MAX 512

enum class log_level
{
	FATAL = 0,
	ERROR = 1,
	WARN = 2,
	INFO = 3,
	DEBUG = 4,
	TRACE = 5
};

class logger
{
public:
	explicit logger(log_level lv = log_level::WARN);

	explicit logger(const std::string &log_file, log_level level = log_level::WARN);

	explicit logger(int fd, log_level lv = log_level::WARN);

	virtual ~logger();

	[[nodiscard]] int get_log_fd() const
	{
		return this->log_fd;
	}

	void set_log_fd(int fd)
	{
		if (fd != -1)
		{
			this->log_fd = fd;
		}
	}

	void set_log_level(log_level lv)
	{
		this->level = lv;
	}

	[[nodiscard]] log_level get_log_level() const
	{
		return this->level;
	}

	std::string get_prefix()
	{
		return this->prefix;
	}

	void set_prefix(std::string describe)
	{
		this->prefix = std::move(describe);
	}

	void log(log_level lv, const char *__restrict fmt, ...);

	void log_fatal(const char *__restrict fmt, ...);

	void log_error(const char *__restrict fmt, ...);

	void log_warn(const char *__restrict fmt, ...);

	void log_info(const char *__restrict fmt, ...);

	void log_debug(const char *__restrict fmt, ...);

	void log_trace(const char *__restrict fmt, ...);

private:
	void log(log_level lv, const char *__restrict fmt, va_list args);

	size_t get_outset(log_level lv, char *__restrict buf, size_t len);

private:
	log_level level;
	int log_fd;
	std::string prefix;
};

#endif //LOG4CPP_LOG4CPP_HPP
