#include <cstdarg>
#include <cstring>
#include <log4cpp/log4cpp.hpp>
#include <regex>
#include <string>

#include "common/log_utils.hpp"
#include "pattern/log_pattern.hpp"

namespace log4cpp::pattern {
    const char *DEFAULT_LOG_PATTERN = "${NM}: ${yyyy}-${MM}-${dd} ${hh}:${mm}:${ss} [${8TH}] [${L}] -- ${W}";

    constexpr unsigned int THREAD_NAME_MAX_LEN = 16;
    constexpr unsigned int THREAD_ID_WIDTH_MAX = 8;

    std::string log_pattern::_pattern = DEFAULT_LOG_PATTERN;

    void log_pattern::set_pattern(const std::string &pattern) {
        _pattern = pattern;
    }

    const char *LOGGER_NAME = "${NM}";
    /* A two digit representation of a year. e.g. 99 or 03 */
    const char *SHORT_YEAR = "${yy}";
    /* A full numeric representation of a year, at least 4 digits, with - for years BCE. e.g. -0055, 0787, 1999, 2003,
     * 10191 */
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
    /* 12-hour format of an hour without leading zeros, with Uppercase Ante meridiem and Post meridiem. 0 through 12
     * e.g. AM 01 or PM 11 */
    const char *SHORT_12HOUR = "${h}";
    /* 12-hour format of an hour with leading zeros, with Uppercase Ante meridiem and Post meridiem. 00 through 12 e.g.
     * AM 01 or PM 11 */
    const char *FULL_12HOUR = "${hh}";
    /* 24-hour format of an hour without leading zeros, 0 through 23. e.g. 1 or 23 */
    const char *SHORT_24HOUR = "${H}";
    /* 24-hour format of an hour with leading zeros. 00 through 23. e.g. 01 or 23 */
    const char *FULL_24HOUR = "${HH}";
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
    /* The regular expression to match the thread name pattern, e.g. ${8TN}. max width is 16. e.g. "main". If the name
     * is empty, use thread id instead */
    const std::regex THREAD_NAME_REGEX(R"(\$\{(\d{1,2})?TN\})");
    /* The regular expression to match the thread id pattern, e.g. ${8TH}. max width is 8. e.g. T12345 */
    const std::regex THREAD_ID_REGEX(R"(\$\{(\d{1,2})?TH\})");
    /* Log level, Value range: FATAL, ERROR, WARN, INFO, DEBUG, TRACE */
    const char *LOG_LEVEL = "${L}";
    /* Log message, e.g.: hello world! */
    const char *LOG_MESSAGE = "${W}";

    enum class HOUR_BASE { HOUR_NONE, HOUR_12, HOUR_24 };

    void format_day(char *buf, size_t len, const std::string &pattern, const tm &now_tm) {
        common::log4c_scnprintf(buf, len, "%s", pattern.c_str());
        if (pattern.find(SHORT_YEAR) != std::string::npos) {
            char year[7];
            common::log4c_scnprintf(year, sizeof(year), "%d", now_tm.tm_year % 100);
            common::log4c_replace(buf, len, SHORT_YEAR, year);
        }
        if (pattern.find(FULL_YEAR) != std::string::npos) {
            char year[7];
            common::log4c_scnprintf(year, sizeof(year), "%04d", 1900 + now_tm.tm_year);
            common::log4c_replace(buf, len, FULL_YEAR, year);
        }
        if (pattern.find(SHORT_MONTH) != std::string::npos) {
            char month[3];
            common::log4c_scnprintf(month, sizeof(month), "%d", now_tm.tm_mon + 1);
            common::log4c_replace(buf, len, SHORT_MONTH, month);
        }
        if (pattern.find(FULL_MONTH) != std::string::npos) {
            char month[3];
            common::log4c_scnprintf(month, sizeof(month), "%02d", now_tm.tm_mon + 1);
            common::log4c_replace(buf, len, FULL_MONTH, month);
        }
        if (pattern.find(ABBR_MONTH) != std::string::npos) {
            common::log4c_replace(buf, len, ABBR_MONTH, MONTH_ABBR_NAME[now_tm.tm_mon]);
        }
        if (pattern.find(SHORT_DAY) != std::string::npos) {
            char day[3];
            common::log4c_scnprintf(day, sizeof(day), "%d", now_tm.tm_mday);
            common::log4c_replace(buf, len, SHORT_DAY, day);
        }
        if (pattern.find(FULL_DAY) != std::string::npos) {
            char day[3];
            common::log4c_scnprintf(day, sizeof(day), "%02d", now_tm.tm_mday);
            common::log4c_replace(buf, len, FULL_DAY, day);
        }
    }

    void format_time(char *buf, size_t len, const std::string &pattern, const tm &now_tm, unsigned short ms) {
        HOUR_BASE hour_base = HOUR_BASE::HOUR_NONE;
        char time_str[16];
        size_t tm_len = 0;
        size_t pattern_start = std::string::npos, pattern_end = std::string::npos;
        size_t pos = pattern.find(SHORT_12HOUR);
        if (std::string::npos != pos) {
            hour_base = HOUR_BASE::HOUR_12;
            pattern_start = pos;
            if (now_tm.tm_hour > 12) {
                tm_len +=
                    common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%d", now_tm.tm_hour - 12);
            }
            else {
                tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%d", now_tm.tm_hour);
            }
            pattern_end = pos + strlen(SHORT_12HOUR);
        }
        pos = pattern.find(SHORT_24HOUR);
        if (std::string::npos != pos) {
            hour_base = HOUR_BASE::HOUR_24;
            pattern_start = pos;
            tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%d", now_tm.tm_hour);
            pattern_end = pos + strlen(SHORT_24HOUR);
        }
        pos = pattern.find(FULL_12HOUR);
        if (std::string::npos != pos) {
            hour_base = HOUR_BASE::HOUR_12;
            pattern_start = pos;
            if (now_tm.tm_hour > 12) {
                tm_len +=
                    common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%02d", now_tm.tm_hour - 12);
            }
            else {
                tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%02d", now_tm.tm_hour);
            }
            pattern_end = pos + strlen(FULL_12HOUR);
        }
        pos = pattern.find(FULL_24HOUR);
        if (std::string::npos != pos) {
            hour_base = HOUR_BASE::HOUR_24;
            pattern_start = pos;
            tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%02d", now_tm.tm_hour);
            pattern_end = pos + strlen(FULL_24HOUR);
        }
        pos = pattern.find(SHORT_MINUTES);
        if (std::string::npos != pos) {
            char delimiter = pattern[pos - 1];
            pattern_start = std::string::npos == pattern_start ? pos : pattern_start;
            tm_len +=
                common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%c%d", delimiter, now_tm.tm_min);
            pattern_end = pos + strlen(SHORT_MINUTES);
        }
        pos = pattern.find(FULL_MINUTES);
        if (std::string::npos != pos) {
            char delimiter = pattern[pos - 1];
            tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%c%02d", delimiter,
                                              now_tm.tm_min);
            pattern_end = pos + strlen(FULL_MINUTES);
        }
        pos = pattern.find(SHORT_SECOND);
        if (std::string::npos != pos) {
            char delimiter = pattern[pos - 1];
            tm_len +=
                common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%c%d", delimiter, now_tm.tm_sec);
            pattern_end = pos + strlen(SHORT_SECOND);
        }
        pos = pattern.find(FULL_SECOND);
        if (std::string::npos != pos) {
            char delimiter = pattern[pos - 1];
            tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%c%02d", delimiter,
                                              now_tm.tm_sec);
            pattern_end = pos + strlen(FULL_SECOND);
        }
        pos = pattern.find(MILLISECOND);
        if (std::string::npos != pos) {
            char delimiter = pattern[pos - 1];
            tm_len += common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, "%c%03d", delimiter, ms);
            pattern_end = pos + strlen(MILLISECOND);
        }
        if (HOUR_BASE::HOUR_12 == hour_base) {
            if (now_tm.tm_hour < 12) {
                common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, " AM");
            }
            else {
                common::log4c_scnprintf(time_str + tm_len, sizeof(time_str) - tm_len, " PM");
            }
        }
        size_t replace_len = pattern_end - pattern_start;
        char replace_str[32];
        strncpy(replace_str, pattern.c_str() + pattern_start, replace_len);
        replace_str[replace_len] = '\0';
        common::log4c_replace(buf, len, replace_str, time_str);
    }

    void format_daytime(char *buf, size_t len, const std::string &pattern, const tm &now_tm, unsigned short ms) {
        format_day(buf, len, pattern, now_tm);
        format_time(buf, len, pattern, now_tm, ms);
    }

    size_t log_pattern::format_with_pattern(char *buf, size_t len, const char *name, log_level level, const char *msg) {
        tm now_tm{};
        unsigned short ms;
        common::get_time_now(now_tm, ms);

        format_daytime(buf, len, _pattern, now_tm, ms);

        // replace ${NM} with logger name
        if (_pattern.find(LOGGER_NAME) != std::string::npos) {
            char logger_name[8];
            common::log4c_scnprintf(logger_name, sizeof(logger_name), "%-7s", name);
            common::log4c_replace(buf, len, LOGGER_NAME, logger_name);
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
                common::log4c_scnprintf(thread_name, sizeof(thread_name), "%-*s", width, _name);
            }
            else {
                common::log4c_scnprintf(thread_name, sizeof(thread_name), "T%0*lu", width, tid);
            }
            const std::string full_match_str = match[0];
            common::log4c_replace(buf, len, full_match_str.c_str(), thread_name);
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
            common::log4c_scnprintf(thread_id, sizeof(thread_id), "T%0*lu", width, tid);
            const std::string full_match_str = match[0];
            common::log4c_replace(buf, len, full_match_str.c_str(), thread_id);
        }
        // replace ${L} with log level, log level fixed length is 5, align left, fill with space
        if (_pattern.find(LOG_LEVEL) != std::string::npos) {
            char log_level[6];
            common::log4c_scnprintf(log_level, sizeof(log_level), "%-5s", level_to_string(level).c_str());
            common::log4c_replace(buf, len, LOG_LEVEL, log_level);
        }
        if (_pattern.find(LOG_MESSAGE) != std::string::npos) {
            common::log4c_replace(buf, len, LOG_MESSAGE, msg);
        }
        return 0;
    }

    size_t log_pattern::format(char *buf, size_t buf_len, const char *name, log_level level, const char *fmt,
                               va_list args) {
        char message[LOG_LINE_MAX];
        message[0] = '\0';
        common::log4c_vscnprintf(message, sizeof(message), fmt, args);

        format_with_pattern(buf, buf_len, name, level, message);
        size_t used_len = strlen(buf);
        used_len += common::log4c_scnprintf(buf + used_len, buf_len - used_len, "\n");
        return used_len;
    }

    size_t log_pattern::format(char *buf, size_t buf_len, const char *name, log_level level, const char *fmt, ...) {
        char message[LOG_LINE_MAX];
        message[0] = '\0';
        va_list args;
        va_start(args, fmt);
        common::log4c_vscnprintf(message, sizeof(message), fmt, args);
        va_end(args);
        format_with_pattern(buf, buf_len, name, level, message);
        size_t used_len = strlen(buf);
        used_len += common::log4c_scnprintf(buf + used_len, buf_len - used_len, "\n");
        return used_len;
    }
}
