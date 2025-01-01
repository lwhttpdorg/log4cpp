#include "pattern.h"

#include <regex>

namespace log4cpp {
	pattern::pattern() { this->_pattern = "${yyyy}-${MM}-${dd} %{hh}:${mm}:${ss} [${t}]: [${l}] -- ${M}"; }

	pattern::pattern(const std::string &p) { this->_pattern = p; }

	pattern::pattern(pattern &&other) noexcept : _pattern(std::move(other._pattern)) {
	}

	pattern &pattern::operator=(const pattern &other) {
		if (this != &other) {
			_pattern = other._pattern;
		}
		return *this;
	}

	pattern &pattern::operator=(pattern &&other) noexcept {
		if (this != &other) {
			_pattern = std::move(other._pattern);
		}
		return *this;
	}

	constexpr unsigned int THREAD_NAME_MAX_LEN = 16;
	/**
	${yy}: A two digit representation of a year. Examples: 99 or 03
	${yyyy}: A full numeric representation of a year, at least 4 digits, with - for years BCE. Examples: -0055, 0787, 1999, 2003, 10191
	${M}: Numeric representation of a month, without leading zeros. 1 through 12
	${MM}: Numeric representation of a month, with leading zeros. 01 through 12
	${MMM}: A short textual representation of a month, three letters. Jan through Dec
	${d}: Day of the month without leading zeros. 1 to 31
	${dd}: Day of the month, 2 digits with leading zeros. 01 to 31
	%{h}: 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem. Examples: AM 01 or PM 11
	%{hh}: 24-hour format of an hour with leading zeros. 00 through 23
	${m}: Minutes without leading zeros. 1 to 59
	${mm}: Minutes with leading zeros. 01 to 59
	${s}: Seconds without leading zeros. 1 to 59
	${ss}: Seconds with leading zeros. 01 to 59
	${t}: The ID of this thread, without leading zeros. Examples: 1 or 1024
	${4t}: The ID of this thread, 4 digits with leading zeros. Examples: 0001 or 1024
	${l}: Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE
	${M}: Log content, Examples: hello world!
	*/
	std::string pattern::format(const std::string &log_name, log4cpp::log_level level, const std::string &msg) const {
		std::string formatted = _pattern;
		std::string level_str = to_string(level);
		std::string month_str;
		std::string day_str;
		std::string hour_str;
		std::string minute_str;
		std::string second_str;
		std::string thread_id_str;

		// Get the current time
		time_t now = time(nullptr);
		tm *ltm = localtime(&now);

		// Format the month
		if (ltm->tm_mon + 1 < 10) {
			month_str = "0" + std::to_string(ltm->tm_mon + 1);
		}
		else {
			month_str = std::to_string(ltm->tm_mon + 1);
		}

		// Format the day
		if (ltm->tm_mday < 10) {
			day_str = "0" + std::to_string(ltm->tm_mday);
		}
		else {
			day_str = std::to_string(ltm->tm_mday);
		}

		// Format the hour
		if (ltm->tm_hour < 10) {
			hour_str = "0" + std::to_string(ltm->tm_hour);
		}
		else {
			hour_str = std::to_string(ltm->tm_hour);
		}

		// Format the minute
		if (ltm->tm_min < 10) {
			minute_str = "0" + std::to_string(ltm->tm_min);
		}
		else {
			minute_str = std::to_string(ltm->tm_min);
		}

		// Format the second
		if (ltm->tm_sec < 10) {
			second_str = "0" + std::to_string(ltm->tm_sec);
		}
		else {
			second_str = std::to_string(ltm->tm_sec);
		}

		// Format the thread id
		char thread_name[THREAD_NAME_MAX_LEN];
		thread_name[0] = '\0';
#if defined(_MSC_VER) || defined(_WIN32)
		unsigned long tid = GetCurrentThreadId();
#endif
#ifdef __linux__
		unsigned long tid = gettid();
#endif

#ifdef _PTHREAD_H
		pthread_getname_np(pthread_self(), thread_name, sizeof(thread_name));
#elif __linux__
		prctl(PR_GET_NAME, reinterpret_cast<unsigned long>(thread_name));
#endif
		if (thread_name[0] != '\0') {
			thread_id_str = thread_name;
		}
		else {
			thread_id_str = std::to_string(tid);
		}
		// replace the pattern with the actual values
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{yyyy\})"), std::to_string(1900 + ltm->tm_year));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{yy\})"),
		                               std::to_string(1900 + ltm->tm_year).substr(2, 2));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{MM\})"), month_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{M\})"), std::to_string(ltm->tm_mon + 1));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{MMM\})"), month_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{dd\})"), day_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{d\})"), std::to_string(ltm->tm_mday));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{hh\})"), hour_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{h\})"), std::to_string(ltm->tm_hour));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{mm\})"), minute_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{m\})"), std::to_string(ltm->tm_min));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{ss\})"), second_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{s\})"), std::to_string(ltm->tm_sec));
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{t\})"), thread_id_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{4t\})"), thread_id_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{l\})"), level_str);
		formatted = std::regex_replace(formatted, std::regex(R"(\$\{M\})"), msg);

		return formatted;
	}
}
