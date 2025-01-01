#pragma once

#include <string>
#include "log4cpp.hpp"

namespace log4cpp {
	class pattern {
	private:
		// The pattern to format the log message. e.g. ${yyyy}-${MM}-${dd} %{hh}:${mm}:${ss} [${t}]: [${l}] -- ${M}
		std::string _pattern;

	public:
		pattern();

		explicit pattern(const std::string &pattern);

		pattern(const pattern &other) = default;

		pattern(pattern &&other) noexcept;

		pattern &operator=(const pattern &other);

		pattern &operator=(pattern &&other) noexcept;

		[[nodiscard]] std::string format(const std::string &log_name, log4cpp::log_level level,
		                                 const std::string &msg) const;
	};
}
