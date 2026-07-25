#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "common/json.hpp"
#include "common/log_net.hpp"

namespace {

    using log4cpp::common::net_addr;
    using log4cpp::common::net_family;
    using log4cpp::common::prefer_stack;
    using log4cpp::common::sock_addr;

    TEST(log_net_test, converts_preferred_stack_values) {
        std::string value;

        log4cpp::common::to_string(prefer_stack::IPv4, value);
        EXPECT_EQ(value, "IPv4");
        log4cpp::common::to_string(prefer_stack::IPv6, value);
        EXPECT_EQ(value, "IPv6");
        log4cpp::common::to_string(prefer_stack::AUTO, value);
        EXPECT_EQ(value, "AUTO");
        log4cpp::common::to_string(static_cast<prefer_stack>(255), value);
        EXPECT_EQ(value, "Unknown");

        prefer_stack prefer{};
        log4cpp::common::from_string("iPv4", prefer);
        EXPECT_EQ(prefer, prefer_stack::IPv4);
        log4cpp::common::from_string("IPV6", prefer);
        EXPECT_EQ(prefer, prefer_stack::IPv6);
        log4cpp::common::from_string("Auto", prefer);
        EXPECT_EQ(prefer, prefer_stack::AUTO);
        EXPECT_THROW(log4cpp::common::from_string("invalid", prefer), std::invalid_argument);
    }

    TEST(log_net_test, parses_compares_and_formats_ip_addresses) {
        const net_addr default_address;
        const net_addr ipv4("127.0.0.1");
        const net_addr ipv4_copy(std::string("127.0.0.1"));
        const net_addr ipv4_other("127.0.0.2");
        const net_addr ipv6("::1");
        const net_addr ipv6_copy("0:0:0:0:0:0:0:1");
        const net_addr ipv6_other("::2");

        EXPECT_EQ(default_address.family, net_family::NET_IPv4);
        EXPECT_EQ(default_address.to_string(), "0.0.0.0");
        EXPECT_EQ(ipv4.to_string(), "127.0.0.1");
        EXPECT_EQ(ipv6.to_string(), "::1");
        EXPECT_EQ(ipv4, ipv4_copy);
        EXPECT_NE(ipv4, ipv4_other);
        EXPECT_EQ(ipv6, ipv6_copy);
        EXPECT_NE(ipv6, ipv6_other);
        EXPECT_NE(ipv4, ipv6);
        EXPECT_THROW(net_addr("not-an-address"), std::invalid_argument);

        net_addr invalid_family;
        invalid_family.family = static_cast<net_family>(255);
        EXPECT_THROW(static_cast<void>(invalid_family.to_string()), std::invalid_argument);
    }

    TEST(log_net_test, resolves_literal_addresses_without_dns) {
        EXPECT_EQ(net_addr::resolve("192.0.2.1", prefer_stack::IPv6), net_addr("192.0.2.1"));
        EXPECT_EQ(net_addr::resolve("2001:db8::1", prefer_stack::IPv4), net_addr("2001:db8::1"));
    }

    TEST(log_net_test, converts_addresses_to_and_from_json) {
        const net_addr original("203.0.113.7");
        log4cpp::json_value json;
        log4cpp::common::to_json(json, original);
        EXPECT_EQ(json.get<std::string>(), "203.0.113.7");

        net_addr decoded;
        log4cpp::common::from_json(json, decoded);
        EXPECT_EQ(decoded, original);
        EXPECT_THROW(log4cpp::common::from_json(log4cpp::json_value(42), decoded), std::runtime_error);
    }

    TEST(log_net_test, supports_socket_address_value_semantics) {
        const sock_addr default_address;
        const sock_addr from_c_string("127.0.0.1", 9443);
        const sock_addr from_string(std::string("127.0.0.1"), 9443);
        const sock_addr different_port("127.0.0.1", 9444);

        EXPECT_EQ(default_address.to_string(), "0.0.0.0@0");
        EXPECT_EQ(from_c_string.to_string(), "127.0.0.1@9443");
        EXPECT_EQ(from_c_string, from_string);
        EXPECT_NE(from_c_string, different_port);
    }

} // namespace
