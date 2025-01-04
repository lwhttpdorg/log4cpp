#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <chrono>
#include <csignal>
#include <regex>
#include <cstdarg>
#include <cstring>
#include <array>

#if defined(_WIN32)

#include <windows.h>

#endif

#ifdef _MSC_VER
#include <processthreadsapi.h>
#endif

#ifdef __linux__

#include <unistd.h>
#include <sys/prctl.h>

#endif

#include "layout_pattern.h"
#include "log_utils.h"

namespace log4cpp {
	const char *DEFAULT_LAYOUT_PATTERN = "${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss} [${8TH}] [${L}] -- ${W}";

	constexpr unsigned int THREAD_NAME_MAX_LEN = 16;
	constexpr unsigned int THREAD_ID_WIDTH_MAX = 8;
	constexpr std::array<const char *, 12> MONTH_NAME = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	std::string layout_pattern::_pattern = DEFAULT_LAYOUT_PATTERN;

	void layout_pattern::set_pattern(const std::string &pattern) {
		_pattern = pattern;
	}


	unsigned long get_thread_name_id(char *thread_name, size_t len) {
		thread_name[0] = '\0';
#if defined(_MSC_VER)
		(void)len;
		unsigned long tid = GetCurrentThreadId();
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

	/* A two digit representation of a year. e.g. 99 or 03 */
	const char *SHORT_YEAR = "${yy}";
	/* A full numeric representation of a year, at least 4 digits, with - for years BCE. e.g. -0055, 0787, 1999, 2003, 10191 */
	const char *FULL_YEAR = "${yyyy}";
	/* Numeric representation of a month, without leading zeros. 1 through 12 */
	const char *SHORT_MONTH = "${M}";
	/* Numeric representation of a month, with leading zeros. 01 through 12 */
	const char *FULL_MONTH = "${MM}";
	/* A short textual representation of a month, three letters. Jan through Dec */
	const char *ABBR_MONTH = "${MMM}";
	/* Day of the month without leading zeros. 1 to 31 */
	const char *SHORT_DAY = "${d}";
	/* Day of the month, 2 digits with leading zeros. 01 to 31 */
	const char *FULL_DAY = "${dd}";
	/* 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem. e.g. AM 01 or PM 11 */
	const char *SHORT_HOUR = "${h}";
	/* 24-hour format of an hour with leading zeros. 00 through 23 */
	const char *FULL_HOUR = "${hh}";
	/* Minutes without leading zeros. 1 to 59 */
	const char *SHORT_MINUTES = "${m}";
	/* Minutes with leading zeros. 01 to 59 */
	const char *FULL_MINUTES = "${mm}";
	/* Seconds without leading zeros. 1 to 59 */
	const char *SHORT_SECOND = "${s}";
	/* Seconds with leading zeros. 01 to 59 */
	const char *FULL_SECOND = "${ss}";
	/* Milliseconds with leading zeros. 001 to 999 */
	const char *MILLISECOND = "${ms}";
	/* The regular expression to match the thread name pattern, e.g. ${8TN}. max width is 16. e.g. "main". If the name is empty, use thread id instead */
	const std::regex THREAD_NAME_REGEX(R"(\$\{(\d{1,2})?TN\})");
	/* The regular expression to match the thread id pattern, e.g. ${8TH}. max width is 8. e.g. T12345 */
	const std::regex THREAD_ID_REGEX(R"(\$\{(\d{1,2})?TH\})");
	/* Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE */
	const char *LOG_LEVEL = "${L}";
	/* Log message, e.g.: hello world! */
	const char *LOG_MESSAGE = "${W}";

	size_t layout_pattern::format_with_pattern(char *buf, size_t len, log_level level, const char *msg) {
		const auto now = std::chrono::system_clock::now();
		const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
		const std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
		const tm *now_tm = std::localtime(&now_time_t);
		log4c_scnprintf(buf, len, "%s", _pattern.c_str());
		if (_pattern.find(SHORT_YEAR) != std::string::npos) {
			char year[3];
			log4c_scnprintf(year, 3, "%02d", now_tm->tm_year % 100);
			replace(buf, len, SHORT_YEAR, year);
		}
		if (_pattern.find(FULL_YEAR) != std::string::npos) {
			char year[5];
			log4c_scnprintf(year, 5, "%04d", 1900 + now_tm->tm_year);
			replace(buf, len, FULL_YEAR, year);
		}
		if (_pattern.find(SHORT_MONTH) != std::string::npos) {
			char month[3];
			log4c_scnprintf(month, 3, "%02d", now_tm->tm_mon + 1);
			replace(buf, len, SHORT_MONTH, month);
		}
		if (_pattern.find(FULL_MONTH) != std::string::npos) {
			char month[3];
			log4c_scnprintf(month, 3, "%02d", now_tm->tm_mon + 1);
			replace(buf, len, FULL_MONTH, month);
		}
		if (_pattern.find(ABBR_MONTH) != std::string::npos) {
			replace(buf, len, ABBR_MONTH, MONTH_NAME[now_tm->tm_mon]);
		}
		if (_pattern.find(SHORT_DAY) != std::string::npos) {
			char day[3];
			log4c_scnprintf(day, 3, "%02d", now_tm->tm_mday);
			replace(buf, len, SHORT_DAY, day);
		}
		if (_pattern.find(FULL_DAY) != std::string::npos) {
			char day[3];
			log4c_scnprintf(day, 3, "%02d", now_tm->tm_mday);
			replace(buf, len, FULL_DAY, day);
		}
		if (_pattern.find(SHORT_HOUR) != std::string::npos) {
			replace(buf, len, SHORT_HOUR, (now_tm->tm_hour < 12) ? "AM" : "PM");
		}
		if (_pattern.find(FULL_HOUR) != std::string::npos) {
			char hour[3];
			log4c_scnprintf(hour, 3, "%02d", now_tm->tm_hour);
			replace(buf, len, FULL_HOUR, hour);
		}
		if (_pattern.find(SHORT_MINUTES) != std::string::npos) {
			char minutes[3];
			log4c_scnprintf(minutes, 3, "%02d", now_tm->tm_min);
			replace(buf, len, SHORT_MINUTES, minutes);
		}
		if (_pattern.find(FULL_MINUTES) != std::string::npos) {
			char minutes[3];
			log4c_scnprintf(minutes, 3, "%02d", now_tm->tm_min);
			replace(buf, len, FULL_MINUTES, minutes);
		}
		if (_pattern.find(SHORT_SECOND) != std::string::npos) {
			char seconds[3];
			log4c_scnprintf(seconds, 3, "%02d", now_tm->tm_sec);
			replace(buf, len, SHORT_SECOND, seconds);
		}
		if (_pattern.find(FULL_SECOND) != std::string::npos) {
			char seconds[3];
			log4c_scnprintf(seconds, 3, "%02d", now_tm->tm_sec);
			replace(buf, len, FULL_SECOND, seconds);
		}
		if (_pattern.find(MILLISECOND) != std::string::npos) {
			char millisecond[4];
			log4c_scnprintf(millisecond, 4, "%03d", static_cast<int>(now_ms.count()));
			replace(buf, len, MILLISECOND, millisecond);
		}
		if (std::smatch match; std::regex_search(_pattern, match, THREAD_NAME_REGEX)) {
			size_t width = THREAD_NAME_MAX_LEN;
			if (match[1].matched) {
				const std::string width_str = match[1].str();
				width = std::stoul(width_str);
			}
			if (THREAD_NAME_MAX_LEN < width) {
				width = THREAD_NAME_MAX_LEN;
			}
			char _name[THREAD_NAME_MAX_LEN];
			const unsigned long tid = get_thread_name_id(_name, sizeof(_name));
			char thread_name[THREAD_NAME_MAX_LEN];
			if (_name[0] != '\0') {
				log4c_scnprintf(thread_name, sizeof(thread_name), "%-*s", width, _name);
			}
			else {
				log4c_scnprintf(thread_name, sizeof(thread_name), "T%0*u", width, tid);
			}
			const std::string full_match_str = match[0];
			replace(buf, len, full_match_str.c_str(), thread_name);
		}
		if (std::smatch match; std::regex_search(_pattern, match, THREAD_ID_REGEX)) {
			size_t width = THREAD_ID_WIDTH_MAX;
			if (match[1].matched) {
				const std::string width_str = match[1].str();
				width = std::stoul(width_str);
			}
			if (THREAD_ID_WIDTH_MAX < width) {
				width = THREAD_ID_WIDTH_MAX;
			}

			char thread_name[THREAD_NAME_MAX_LEN];
			const unsigned long tid = get_thread_name_id(thread_name, sizeof(thread_name));

			char thread_id[THREAD_NAME_MAX_LEN + 1];
			thread_id[0] = '\0';
			log4c_scnprintf(thread_id, sizeof(thread_id), "T%0*u", width, tid);
			const std::string full_match_str = match[0];
			replace(buf, len, full_match_str.c_str(), thread_id);
		}
		// replace ${L} with log level, log level fixed length is 5, align left, fill with space
		if (_pattern.find(LOG_LEVEL) != std::string::npos) {
			char log_level[6];
			log4c_scnprintf(log_level, 6, "%-5s", to_string(level).c_str());
			replace(buf, len, LOG_LEVEL, log_level);
		}
		if (_pattern.find(LOG_MESSAGE) != std::string::npos) {
			replace(buf, len, LOG_MESSAGE, msg);
		}
		return 0;
	}

	size_t layout_pattern::format(char *buf, size_t buf_len, log_level level, const char *fmt, va_list args) {
		char message[LOG_LINE_MAX];
		message[0] = '\0';
		log4c_vscnprintf(message, sizeof(message), fmt, args);

		format_with_pattern(buf, buf_len, level, message);
		size_t used_len = strlen(buf);
		used_len += log4c_scnprintf(buf + used_len, buf_len - used_len, "\n");
		return used_len;
	}

	size_t layout_pattern::format(char *buf, size_t buf_len, log_level level, const char *fmt, ...) {
		char message[LOG_LINE_MAX];
		message[0] = '\0';
		va_list args;
		va_start(args, fmt);
		log4c_vscnprintf(message, sizeof(message), fmt, args);
		va_end(args);
		format_with_pattern(buf, buf_len, level, message);
		size_t used_len = strlen(buf);
		used_len += log4c_scnprintf(buf + used_len, buf_len - used_len, "\n");
		return used_len;
	}
}
