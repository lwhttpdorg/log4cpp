#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "common/json.hpp"

namespace {

    using log4cpp::json_array;
    using log4cpp::json_object;
    using log4cpp::json_parse_error;
    using log4cpp::json_value;

    TEST(json_value_test, supports_scalar_types_and_numeric_conversions) {
        const json_value default_null;
        const json_value explicit_null(nullptr);
        const json_value boolean(true);
        const json_value integer(-42);
        const json_value signed_integer(int64_t{-43});
        const json_value unsigned_integer(uint64_t{44});
        const json_value unsigned_int_value(static_cast<unsigned int>(45));
        const json_value unsigned_short_value(static_cast<unsigned short>(46));
        const json_value floating(12.5);
        const json_value c_string("text");
        const std::string text = "copied";
        const json_value copied_string(text);
        json_value moved_string(std::string("moved"));

        EXPECT_TRUE(default_null.is_null());
        EXPECT_TRUE(explicit_null.is_null());
        EXPECT_TRUE(boolean.is_boolean());
        EXPECT_TRUE(integer.is_number());
        EXPECT_TRUE(c_string.is_string());
        EXPECT_FALSE(default_null.is_number());

        EXPECT_TRUE(boolean.get<bool>());
        EXPECT_EQ(integer.get<int>(), -42);
        EXPECT_EQ(signed_integer.get<int64_t>(), -43);
        EXPECT_EQ(unsigned_integer.get<uint64_t>(), 44U);
        EXPECT_EQ(unsigned_int_value.get<uint64_t>(), 45U);
        EXPECT_EQ(unsigned_short_value.get<unsigned short>(), 46U);
        EXPECT_DOUBLE_EQ(floating.get<double>(), 12.5);
        EXPECT_EQ(c_string.get<std::string>(), "text");
        EXPECT_EQ(copied_string.get<std::string>(), "copied");

        std::string moved_text;
        moved_string.get_to(moved_text);
        EXPECT_EQ(moved_text, "moved");

        EXPECT_THROW(boolean.get<int>(), std::runtime_error);
        EXPECT_THROW(integer.get<bool>(), std::runtime_error);
        EXPECT_THROW(integer.get<std::string>(), std::runtime_error);
    }

    TEST(json_value_test, supports_object_and_array_access) {
        json_value object{{"number", 7}, {"name", "logger"}};
        EXPECT_TRUE(object.is_object());
        EXPECT_EQ(object.size(), 2U);
        EXPECT_TRUE(object.contains("number"));
        EXPECT_FALSE(object.contains("missing"));
        EXPECT_EQ(object.at("number").get<int>(), 7);
        EXPECT_EQ(std::as_const(object)["name"].get<std::string>(), "logger");
        EXPECT_THROW(object.at("missing"), std::out_of_range);

        json_value created;
        created["enabled"] = true;
        EXPECT_TRUE(created.is_object());
        EXPECT_TRUE(created["enabled"].get<bool>());

        json_value array(json_array{1, "two", false});
        EXPECT_TRUE(array.is_array());
        EXPECT_EQ(array.size(), 3U);
        EXPECT_EQ(array[0].get<int>(), 1);
        array[1] = "changed";
        EXPECT_EQ(std::as_const(array)[1].get<std::string>(), "changed");
        EXPECT_THROW(array[3], std::out_of_range);

        const json_object object_copy = object.get<json_object>();
        EXPECT_EQ(object_copy.at("name").get<std::string>(), "logger");
        const json_array array_copy = array.get<json_array>();
        EXPECT_EQ(array_copy.size(), 3U);

        const json_value strings(json_array{"a", "b"});
        EXPECT_EQ(strings.get<std::vector<std::string>>(), (std::vector<std::string>{"a", "b"}));

        const json_value scalar(1);
        EXPECT_FALSE(scalar.contains("key"));
        EXPECT_EQ(scalar.size(), 0U);
        EXPECT_THROW(scalar.at("key"), std::runtime_error);
        EXPECT_THROW(scalar["key"], std::runtime_error);
        EXPECT_THROW(scalar[0], std::runtime_error);
        EXPECT_THROW(scalar.get<json_array>(), std::runtime_error);
        EXPECT_THROW(scalar.get<json_object>(), std::runtime_error);
        EXPECT_THROW(scalar.get<std::vector<std::string>>(), std::runtime_error);
    }

    TEST(json_value_test, dumps_escaped_strings_and_round_trips_containers) {
        std::string escaped = "\"\\\b\f\n\r\t";
        escaped.push_back('\x01');
        const json_value string_value(escaped);
        EXPECT_EQ(string_value.dump(), R"("\"\\\b\f\n\r\t\u0001")");

        const json_value value{
            {"null", nullptr},
            {"boolean", true},
            {"number", 3.5},
            {"array", json_array{1, 2}},
            {"object", json_object{{"key", "value"}}},
        };
        EXPECT_EQ(json_value::parse(value.dump()), value);
        EXPECT_NE(value, json_value(nullptr));
    }

    TEST(json_value_test, parses_literals_numbers_escapes_and_unicode) {
        const json_value value = json_value::parse(
            R"( { "true": true, "false": false, "null": null, "negative": -12, "unsigned": 9223372036854775808,)"
            R"( "decimal": 1.25, "exponent": 2.5e+2, "escaped": "\"\\\/\b\f\n\r\t",)"
            R"( "unicode": "\u0041\u00a2\u20ac", "empty-array": [], "empty-object": {} } )");

        EXPECT_TRUE(value["true"].get<bool>());
        EXPECT_FALSE(value["false"].get<bool>());
        EXPECT_TRUE(value["null"].is_null());
        EXPECT_EQ(value["negative"].get<int64_t>(), -12);
        EXPECT_EQ(value["unsigned"].get<uint64_t>(), uint64_t{9223372036854775808ULL});
        EXPECT_DOUBLE_EQ(value["decimal"].get<double>(), 1.25);
        EXPECT_DOUBLE_EQ(value["exponent"].get<double>(), 250.0);
        EXPECT_EQ(value["escaped"].get<std::string>(), "\"\\/\b\f\n\r\t");
        EXPECT_EQ(value["unicode"].get<std::string>(), "A¢€");
        EXPECT_EQ(value["empty-array"].size(), 0U);
        EXPECT_EQ(value["empty-object"].size(), 0U);
    }

    TEST(json_value_test, reports_malformed_input) {
        const std::vector<std::string> malformed{
            "",   "-",     "truth",     "nullable",    R"("unterminated)",       "\"\\",       R"("\x")", R"("\u12")",
            "[1", "[1 2]", R"({"key")", R"({"key":1)", R"({"key":1 "other":2})", "1 trailing",
        };

        for (const auto &input: malformed) {
            EXPECT_THROW(json_value::parse(input), json_parse_error) << input;
        }
    }

    TEST(json_value_test, supports_copy_move_and_stream_operations) {
        json_value original{{"key", json_array{1, 2, 3}}};
        json_value copied(original);
        EXPECT_EQ(copied, original);

        json_value copy_assigned;
        copy_assigned = original;
        copy_assigned = copy_assigned;
        EXPECT_EQ(copy_assigned, original);

        json_value moved(std::move(copied));
        EXPECT_EQ(moved, original);
        EXPECT_EQ(copied.dump(), "null");

        json_value move_assigned;
        move_assigned = std::move(copy_assigned);
        move_assigned = std::move(move_assigned);
        EXPECT_EQ(move_assigned, original);

        json_value copied_from_moved(copied);
        EXPECT_TRUE(copied_from_moved.is_null());
        copied_from_moved = copied;
        EXPECT_TRUE(copied_from_moved.is_null());
        EXPECT_NE(copied, copied_from_moved);

        std::istringstream input(R"([1,true,"three"])");
        json_value streamed;
        input >> streamed;
        EXPECT_EQ(streamed.size(), 3U);

        std::ostringstream output;
        output << streamed;
        EXPECT_EQ(output.str(), R"([1,true,"three"])");
    }

} // namespace
