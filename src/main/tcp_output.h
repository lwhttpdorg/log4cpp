#pragma once

#include "log_net.h"

namespace log4cpp {
	class tcp_output_config {
	public:
		net::net_addr local_addr{};
		unsigned short port{0};
	};
}
