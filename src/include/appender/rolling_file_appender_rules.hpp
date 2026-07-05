#pragma once

#include <cstddef>  // for size_t
#include <cstdint>  // for uint64_t
#include <optional> // for std::optional
#include <string>   // for std::string

#include "config/appender.hpp" // for rolling_policy, rolling_policy_type

namespace log4cpp::appender {
    class rolling_file_appender_rules {
    public:
        [[nodiscard]] static bool uses_time_bucket(const std::optional<config::rolling_policy> &rolling) {
            return rolling
                   && (rolling->policy == config::rolling_policy_type::TIME
                       || rolling->policy == config::rolling_policy_type::SIZE_TIME);
        }

        [[nodiscard]] static bool should_roll(const std::optional<config::rolling_policy> &rolling,
                                              uint64_t current_size, size_t msg_len,
                                              const std::string &opened_time_bucket,
                                              const std::string &current_time_bucket) {
            if (!rolling) {
                return false;
            }

            switch (rolling->policy) {
                case config::rolling_policy_type::SIZE:
                    return reaches_file_size(*rolling, current_size, msg_len);
                case config::rolling_policy_type::TIME:
                    return current_time_bucket != opened_time_bucket;
                case config::rolling_policy_type::SIZE_TIME:
                    return reaches_file_size(*rolling, current_size, msg_len);
                case config::rolling_policy_type::NONE:
                case config::rolling_policy_type::ON_START:
                    return false;
            }

            return false;
        }

        [[nodiscard]] static std::string archive_prefix(const std::optional<config::rolling_policy> &rolling,
                                                        const std::string &filename,
                                                        const std::string &opened_time_bucket,
                                                        const std::string &current_time_bucket) {
            if (!uses_time_bucket(rolling)) {
                return filename;
            }

            const std::string &bucket = opened_time_bucket.empty() ? current_time_bucket : opened_time_bucket;
            return filename + "." + bucket;
        }

    private:
        [[nodiscard]] static bool reaches_file_size(const config::rolling_policy &rolling, uint64_t current_size,
                                                    size_t msg_len) {
            return current_size > 0
                   && (current_size >= rolling.file_size || msg_len > rolling.file_size - current_size);
        }
    };
} // namespace log4cpp::appender
