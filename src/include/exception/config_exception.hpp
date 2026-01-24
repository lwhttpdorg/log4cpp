#pragma once

#include <stdexcept>

namespace log4cpp::config {
    class invalid_config_exception: public std::runtime_error {
    public:
        explicit invalid_config_exception(const std::string &msg) :
            std::runtime_error("Invalid log4cpp configuration: " + msg) {
        }
    };
} // namespace log4cpp::config
