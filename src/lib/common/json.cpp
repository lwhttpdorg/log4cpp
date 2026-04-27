#include "common/json.hpp"

#include <cstdio>
#include <limits>

namespace log4cpp {

    // =====================================================================
    // Helper: dump a JSON string with proper escaping
    // =====================================================================

    static void dump_string(std::ostringstream &oss, const std::string &s) {
        oss << '"';
        for (char c: s) {
            switch (c) {
                case '"':
                    oss << "\\\"";
                    break;
                case '\\':
                    oss << "\\\\";
                    break;
                case '\b':
                    oss << "\\b";
                    break;
                case '\f':
                    oss << "\\f";
                    break;
                case '\n':
                    oss << "\\n";
                    break;
                case '\r':
                    oss << "\\r";
                    break;
                case '\t':
                    oss << "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                        oss << buf;
                    }
                    else {
                        oss << c;
                    }
                    break;
            }
        }
        oss << '"';
    }

    // =====================================================================
    // detail::null_node
    // =====================================================================

    namespace detail {

        std::unique_ptr<json_node> null_node::clone() const {
            return std::make_unique<null_node>();
        }

        void null_node::dump(std::ostringstream &oss) const {
            oss << "null";
        }

        bool null_node::equals(const json_node &other) const {
            return other.is_null();
        }

        // =====================================================================
        // detail::boolean_node
        // =====================================================================

        std::unique_ptr<json_node> boolean_node::clone() const {
            return std::make_unique<boolean_node>(value_);
        }

        void boolean_node::dump(std::ostringstream &oss) const {
            oss << (value_ ? "true" : "false");
        }

        bool boolean_node::equals(const json_node &other) const {
            if (!other.is_boolean()) {
                return false;
            }
            return value_ == static_cast<const boolean_node &>(other).value_;
        }

        // =====================================================================
        // detail::number_node
        // =====================================================================

        int64_t number_node::as_int64() const {
            switch (type_) {
                case integer:
                    return int_val_;
                case unsigned_int:
                    return static_cast<int64_t>(uint_val_);
                case floating:
                    return static_cast<int64_t>(float_val_);
            }
            return 0;
        }

        uint64_t number_node::as_uint64() const {
            switch (type_) {
                case integer:
                    return static_cast<uint64_t>(int_val_);
                case unsigned_int:
                    return uint_val_;
                case floating:
                    return static_cast<uint64_t>(float_val_);
            }
            return 0;
        }

        double number_node::as_double() const {
            switch (type_) {
                case integer:
                    return static_cast<double>(int_val_);
                case unsigned_int:
                    return static_cast<double>(uint_val_);
                case floating:
                    return float_val_;
            }
            return 0.0;
        }

        std::unique_ptr<json_node> number_node::clone() const {
            switch (type_) {
                case integer:
                    return std::make_unique<number_node>(int_val_);
                case unsigned_int:
                    return std::make_unique<number_node>(uint_val_);
                case floating:
                    return std::make_unique<number_node>(float_val_);
            }
            return std::make_unique<number_node>(int64_t{0});
        }

        void number_node::dump(std::ostringstream &oss) const {
            switch (type_) {
                case integer:
                    oss << int_val_;
                    break;
                case unsigned_int:
                    oss << uint_val_;
                    break;
                case floating:
                    oss << float_val_;
                    break;
            }
        }

        bool number_node::equals(const json_node &other) const {
            if (!other.is_number()) {
                return false;
            }
            const auto &o = static_cast<const number_node &>(other);
            return as_double() == o.as_double();
        }

        // =====================================================================
        // detail::string_node
        // =====================================================================

        std::unique_ptr<json_node> string_node::clone() const {
            return std::make_unique<string_node>(value_);
        }

        void string_node::dump(std::ostringstream &oss) const {
            dump_string(oss, value_);
        }

        bool string_node::equals(const json_node &other) const {
            if (!other.is_string()) {
                return false;
            }
            return value_ == static_cast<const string_node &>(other).value_;
        }

        // =====================================================================
        // detail::array_node
        // =====================================================================

        std::unique_ptr<json_node> array_node::clone() const {
            return std::make_unique<array_node>(value_);
        }

        void array_node::dump(std::ostringstream &oss) const {
            oss << '[';
            for (size_t i = 0; i < value_.size(); ++i) {
                if (i > 0) {
                    oss << ',';
                }
                oss << value_[i].dump();
            }
            oss << ']';
        }

        bool array_node::equals(const json_node &other) const {
            if (!other.is_array()) {
                return false;
            }
            const auto &o = static_cast<const array_node &>(other);
            return value_ == o.value_;
        }

        // =====================================================================
        // detail::object_node
        // =====================================================================

        std::unique_ptr<json_node> object_node::clone() const {
            return std::make_unique<object_node>(value_);
        }

        void object_node::dump(std::ostringstream &oss) const {
            oss << '{';
            bool first = true;
            for (const auto &[k, v]: value_) {
                if (!first) {
                    oss << ',';
                }
                first = false;
                dump_string(oss, k);
                oss << ':';
                oss << v.dump();
            }
            oss << '}';
        }

        bool object_node::equals(const json_node &other) const {
            if (!other.is_object()) {
                return false;
            }
            const auto &o = static_cast<const object_node &>(other);
            return value_ == o.value_;
        }

    } // namespace detail

    // =====================================================================
    // json_value: Constructors
    // =====================================================================

    json_value::json_value() : node_(std::make_unique<detail::null_node>()) {
    }

    json_value::json_value(std::nullptr_t) : node_(std::make_unique<detail::null_node>()) {
    }

    json_value::json_value(bool v) : node_(std::make_unique<detail::boolean_node>(v)) {
    }

    json_value::json_value(int v) : node_(std::make_unique<detail::number_node>(static_cast<int64_t>(v))) {
    }

    json_value::json_value(int64_t v) : node_(std::make_unique<detail::number_node>(v)) {
    }

    json_value::json_value(uint64_t v) : node_(std::make_unique<detail::number_node>(v)) {
    }

    json_value::json_value(unsigned int v) : node_(std::make_unique<detail::number_node>(static_cast<uint64_t>(v))) {
    }

    json_value::json_value(unsigned short v) : node_(std::make_unique<detail::number_node>(static_cast<uint64_t>(v))) {
    }

    json_value::json_value(double v) : node_(std::make_unique<detail::number_node>(v)) {
    }

    json_value::json_value(const char *v) : node_(std::make_unique<detail::string_node>(std::string(v))) {
    }

    json_value::json_value(const std::string &v) : node_(std::make_unique<detail::string_node>(v)) {
    }

    json_value::json_value(std::string &&v) : node_(std::make_unique<detail::string_node>(std::move(v))) {
    }

    json_value::json_value(const json_array &v) : node_(std::make_unique<detail::array_node>(v)) {
    }

    json_value::json_value(json_array &&v) : node_(std::make_unique<detail::array_node>(std::move(v))) {
    }

    json_value::json_value(const json_object &v) : node_(std::make_unique<detail::object_node>(v)) {
    }

    json_value::json_value(json_object &&v) : node_(std::make_unique<detail::object_node>(std::move(v))) {
    }

    json_value::json_value(std::initializer_list<std::pair<std::string, json_value>> init) {
        json_object obj;
        for (auto &p: init) {
            obj.emplace(p.first, p.second);
        }
        node_ = std::make_unique<detail::object_node>(std::move(obj));
    }

    // =====================================================================
    // json_value: Copy / Move
    // =====================================================================

    json_value::json_value(const json_value &other) :
        node_(other.node_ ? other.node_->clone() : std::make_unique<detail::null_node>()) {
    }

    json_value::json_value(json_value &&other) noexcept : node_(std::move(other.node_)) {
    }

    json_value &json_value::operator=(const json_value &other) {
        if (this != &other) {
            node_ = other.node_ ? other.node_->clone() : std::make_unique<detail::null_node>();
        }
        return *this;
    }

    json_value &json_value::operator=(json_value &&other) noexcept {
        if (this != &other) {
            node_ = std::move(other.node_);
        }
        return *this;
    }

    json_value::~json_value() = default;

    // =====================================================================
    // json_value: Type queries
    // =====================================================================

    bool json_value::is_null() const {
        return node_ && node_->is_null();
    }
    bool json_value::is_boolean() const {
        return node_ && node_->is_boolean();
    }
    bool json_value::is_number() const {
        return node_ && node_->is_number();
    }
    bool json_value::is_string() const {
        return node_ && node_->is_string();
    }
    bool json_value::is_array() const {
        return node_ && node_->is_array();
    }
    bool json_value::is_object() const {
        return node_ && node_->is_object();
    }

    // =====================================================================
    // json_value: Object access
    // =====================================================================

    bool json_value::contains(const std::string &key) const {
        if (!is_object()) {
            return false;
        }
        const auto &obj = static_cast<const detail::object_node &>(*node_).value();
        return obj.find(key) != obj.end();
    }

    const json_value &json_value::at(const std::string &key) const {
        if (!is_object()) {
            throw std::runtime_error("json_value::at() called on non-object");
        }
        const auto &obj = static_cast<const detail::object_node &>(*node_).value();
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw std::out_of_range("key not found: " + key);
        }
        return it->second;
    }

    json_value &json_value::operator[](const std::string &key) {
        if (is_null()) {
            node_ = std::make_unique<detail::object_node>(json_object{});
        }
        if (!is_object()) {
            throw std::runtime_error("json_value::operator[] called on non-object");
        }
        auto &obj = static_cast<detail::object_node &>(*node_).value();
        return obj[key];
    }

    const json_value &json_value::operator[](const std::string &key) const {
        return at(key);
    }

    // =====================================================================
    // json_value: Array access
    // =====================================================================

    const json_value &json_value::operator[](size_t idx) const {
        if (!is_array()) {
            throw std::runtime_error("json_value::operator[] called on non-array");
        }
        return static_cast<const detail::array_node &>(*node_).value().at(idx);
    }

    json_value &json_value::operator[](size_t idx) {
        if (!is_array()) {
            throw std::runtime_error("json_value::operator[] called on non-array");
        }
        return static_cast<detail::array_node &>(*node_).value().at(idx);
    }

    size_t json_value::size() const {
        if (is_array()) {
            return static_cast<const detail::array_node &>(*node_).value().size();
        }
        if (is_object()) {
            return static_cast<const detail::object_node &>(*node_).value().size();
        }
        return 0;
    }

    // =====================================================================
    // json_value: Serialization
    // =====================================================================

    std::string json_value::dump() const {
        std::ostringstream oss;
        if (node_) {
            node_->dump(oss);
        }
        else {
            oss << "null";
        }
        return oss.str();
    }

    // =====================================================================
    // json_value: Stream operators
    // =====================================================================

    std::istream &operator>>(std::istream &is, json_value &j) {
        j = json_value::parse(is);
        return is;
    }

    std::ostream &operator<<(std::ostream &os, const json_value &j) {
        os << j.dump();
        return os;
    }

    // =====================================================================
    // json_value: Comparison
    // =====================================================================

    bool operator==(const json_value &lhs, const json_value &rhs) {
        if (!lhs.node_ && !rhs.node_) {
            return true;
        }
        if (!lhs.node_ || !rhs.node_) {
            return false;
        }
        return lhs.node_->equals(*rhs.node_);
    }

    bool operator!=(const json_value &lhs, const json_value &rhs) {
        return !(lhs == rhs);
    }

    // =====================================================================
    // json_value: Parsing
    // =====================================================================

    static void skip_whitespace(const std::string &s, size_t &pos) {
        while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\n' || s[pos] == '\r')) {
            ++pos;
        }
    }

    static std::string parse_string_raw(const std::string &s, size_t &pos) {
        if (pos >= s.size() || s[pos] != '"') {
            throw json_parse_error("expected '\"'");
        }
        ++pos;
        std::string result;
        while (pos < s.size()) {
            char c = s[pos];
            if (c == '"') {
                ++pos;
                return result;
            }
            if (c == '\\') {
                ++pos;
                if (pos >= s.size()) {
                    throw json_parse_error("unexpected end in string escape");
                }
                switch (s[pos]) {
                    case '"':
                        result += '"';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '/':
                        result += '/';
                        break;
                    case 'b':
                        result += '\b';
                        break;
                    case 'f':
                        result += '\f';
                        break;
                    case 'n':
                        result += '\n';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'u': {
                        if (pos + 4 >= s.size()) {
                            throw json_parse_error("incomplete unicode escape");
                        }
                        std::string hex = s.substr(pos + 1, 4);
                        unsigned long cp = std::stoul(hex, nullptr, 16);
                        pos += 4;
                        if (cp <= 0x7F) {
                            result += static_cast<char>(cp);
                        }
                        else if (cp <= 0x7FF) {
                            result += static_cast<char>(0xC0 | (cp >> 6));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        }
                        else {
                            result += static_cast<char>(0xE0 | (cp >> 12));
                            result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (cp & 0x3F));
                        }
                        break;
                    }
                    default:
                        throw json_parse_error(std::string("invalid escape character: ") + s[pos]);
                }
                ++pos;
            }
            else {
                result += c;
                ++pos;
            }
        }
        throw json_parse_error("unterminated string");
    }

    static json_value parse_value(const std::string &s, size_t &pos);

    static json_value parse_string_node(const std::string &s, size_t &pos) {
        return json_value(parse_string_raw(s, pos));
    }

    static json_value parse_number(const std::string &s, size_t &pos) {
        size_t start = pos;
        bool is_negative = false;
        bool is_float = false;

        if (pos < s.size() && s[pos] == '-') {
            is_negative = true;
            ++pos;
        }
        while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
            ++pos;
        }
        if (pos < s.size() && s[pos] == '.') {
            is_float = true;
            ++pos;
            while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
                ++pos;
            }
        }
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            is_float = true;
            ++pos;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) {
                ++pos;
            }
            while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
                ++pos;
            }
        }

        std::string num_str = s.substr(start, pos - start);
        if (num_str.empty() || num_str == "-") {
            throw json_parse_error("invalid number");
        }

        if (is_float) {
            return json_value(std::stod(num_str));
        }
        if (is_negative) {
            return json_value(static_cast<int64_t>(std::stoll(num_str)));
        }

        uint64_t val = std::stoull(num_str);
        if (val <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
            return json_value(static_cast<int64_t>(val));
        }
        return json_value(val);
    }

    static json_value parse_bool(const std::string &s, size_t &pos) {
        if (s.compare(pos, 4, "true") == 0) {
            pos += 4;
            return json_value(true);
        }
        if (s.compare(pos, 5, "false") == 0) {
            pos += 5;
            return json_value(false);
        }
        throw json_parse_error("invalid boolean value");
    }

    static json_value parse_null(const std::string &s, size_t &pos) {
        if (s.compare(pos, 4, "null") == 0) {
            pos += 4;
            return json_value(nullptr);
        }
        throw json_parse_error("invalid null value");
    }

    static json_value parse_array(const std::string &s, size_t &pos) {
        if (s[pos] != '[') {
            throw json_parse_error("expected '['");
        }
        ++pos;
        json_array arr;
        skip_whitespace(s, pos);
        if (pos < s.size() && s[pos] == ']') {
            ++pos;
            return json_value(std::move(arr));
        }
        while (true) {
            arr.push_back(parse_value(s, pos));
            skip_whitespace(s, pos);
            if (pos >= s.size()) {
                throw json_parse_error("unexpected end in array");
            }
            if (s[pos] == ']') {
                ++pos;
                return json_value(std::move(arr));
            }
            if (s[pos] != ',') {
                throw json_parse_error("expected ',' or ']' in array");
            }
            ++pos;
        }
    }

    static json_value parse_object(const std::string &s, size_t &pos) {
        if (s[pos] != '{') {
            throw json_parse_error("expected '{'");
        }
        ++pos;
        json_object obj;
        skip_whitespace(s, pos);
        if (pos < s.size() && s[pos] == '}') {
            ++pos;
            return json_value(std::move(obj));
        }
        while (true) {
            skip_whitespace(s, pos);
            std::string key = parse_string_raw(s, pos);
            skip_whitespace(s, pos);
            if (pos >= s.size() || s[pos] != ':') {
                throw json_parse_error("expected ':'");
            }
            ++pos;
            obj[key] = parse_value(s, pos);
            skip_whitespace(s, pos);
            if (pos >= s.size()) {
                throw json_parse_error("unexpected end in object");
            }
            if (s[pos] == '}') {
                ++pos;
                return json_value(std::move(obj));
            }
            if (s[pos] != ',') {
                throw json_parse_error("expected ',' or '}' in object");
            }
            ++pos;
        }
    }

    static json_value parse_value(const std::string &s, size_t &pos) {
        skip_whitespace(s, pos);
        if (pos >= s.size()) {
            throw json_parse_error("unexpected end of input");
        }
        switch (s[pos]) {
            case '"':
                return parse_string_node(s, pos);
            case '{':
                return parse_object(s, pos);
            case '[':
                return parse_array(s, pos);
            case 't':
            case 'f':
                return parse_bool(s, pos);
            case 'n':
                return parse_null(s, pos);
            default:
                return parse_number(s, pos);
        }
    }

    json_value json_value::parse(const std::string &input) {
        size_t pos = 0;
        json_value result = parse_value(input, pos);
        skip_whitespace(input, pos);
        if (pos != input.size()) {
            throw json_parse_error("unexpected trailing content");
        }
        return result;
    }

    json_value json_value::parse(std::istream &is) {
        std::string content((std::istreambuf_iterator<char>(is)), std::istreambuf_iterator<char>());
        return parse(content);
    }

    // =====================================================================
    // json_value: get<T> specializations
    // =====================================================================

    template<>
    std::string json_value::get<std::string>() const {
        if (!is_string()) {
            throw std::runtime_error("json_value is not a string");
        }
        return static_cast<const detail::string_node &>(*node_).value();
    }

    template<>
    int json_value::get<int>() const {
        if (!is_number()) {
            throw std::runtime_error("json_value is not a number");
        }
        return static_cast<int>(static_cast<const detail::number_node &>(*node_).as_int64());
    }

    template<>
    int64_t json_value::get<int64_t>() const {
        if (!is_number()) {
            throw std::runtime_error("json_value is not a number");
        }
        return static_cast<const detail::number_node &>(*node_).as_int64();
    }

    template<>
    uint64_t json_value::get<uint64_t>() const {
        if (!is_number()) {
            throw std::runtime_error("json_value is not a number");
        }
        return static_cast<const detail::number_node &>(*node_).as_uint64();
    }

    template<>
    unsigned short json_value::get<unsigned short>() const {
        if (!is_number()) {
            throw std::runtime_error("json_value is not a number");
        }
        return static_cast<unsigned short>(static_cast<const detail::number_node &>(*node_).as_uint64());
    }

    template<>
    double json_value::get<double>() const {
        if (!is_number()) {
            throw std::runtime_error("json_value is not a number");
        }
        return static_cast<const detail::number_node &>(*node_).as_double();
    }

    template<>
    bool json_value::get<bool>() const {
        if (!is_boolean()) {
            throw std::runtime_error("json_value is not a boolean");
        }
        return static_cast<const detail::boolean_node &>(*node_).value();
    }

    template<>
    json_array json_value::get<json_array>() const {
        if (!is_array()) {
            throw std::runtime_error("json_value is not an array");
        }
        return static_cast<const detail::array_node &>(*node_).value();
    }

    template<>
    json_object json_value::get<json_object>() const {
        if (!is_object()) {
            throw std::runtime_error("json_value is not an object");
        }
        return static_cast<const detail::object_node &>(*node_).value();
    }

    template<>
    std::vector<std::string> json_value::get<std::vector<std::string>>() const {
        if (!is_array()) {
            throw std::runtime_error("json_value is not an array");
        }
        const auto &arr = static_cast<const detail::array_node &>(*node_).value();
        std::vector<std::string> result;
        result.reserve(arr.size());
        for (const auto &elem: arr) {
            result.push_back(elem.get<std::string>());
        }
        return result;
    }

} // namespace log4cpp
