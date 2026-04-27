#pragma once

#include <cstdint>
#include <initializer_list>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace log4cpp {

    // ===================== Exceptions =====================

    class json_parse_error: public std::runtime_error {
    public:
        explicit json_parse_error(const std::string &msg) : std::runtime_error(msg) {
        }
    };

    // ===================== Forward declarations =====================

    class json_value;
    using json_array = std::vector<json_value>;
    using json_object = std::map<std::string, json_value>;

    // ===================== Internal node hierarchy =====================

    namespace detail {

        /**
         * @brief Abstract base for polymorphic JSON storage nodes.
         */
        class json_node {
        public:
            virtual ~json_node() = default;
            [[nodiscard]] virtual std::unique_ptr<json_node> clone() const = 0;
            virtual void dump(std::ostringstream &oss) const = 0;
            [[nodiscard]] virtual bool equals(const json_node &other) const = 0;

            [[nodiscard]] virtual bool is_null() const {
                return false;
            }
            [[nodiscard]] virtual bool is_boolean() const {
                return false;
            }
            [[nodiscard]] virtual bool is_number() const {
                return false;
            }
            [[nodiscard]] virtual bool is_string() const {
                return false;
            }
            [[nodiscard]] virtual bool is_array() const {
                return false;
            }
            [[nodiscard]] virtual bool is_object() const {
                return false;
            }
        };

        class null_node final: public json_node {
        public:
            [[nodiscard]] std::unique_ptr<json_node> clone() const override;
            void dump(std::ostringstream &oss) const override;
            [[nodiscard]] bool equals(const json_node &other) const override;
            [[nodiscard]] bool is_null() const override {
                return true;
            }
        };

        class boolean_node final: public json_node {
        public:
            explicit boolean_node(bool v) : value_(v) {
            }
            [[nodiscard]] bool value() const {
                return value_;
            }

            [[nodiscard]] std::unique_ptr<json_node> clone() const override;
            void dump(std::ostringstream &oss) const override;
            [[nodiscard]] bool equals(const json_node &other) const override;
            [[nodiscard]] bool is_boolean() const override {
                return true;
            }

        private:
            bool value_;
        };

        class number_node final: public json_node {
        public:
            explicit number_node(int64_t v) : int_val_(v), type_(integer) {
            }
            explicit number_node(uint64_t v) : uint_val_(v), type_(unsigned_int) {
            }
            explicit number_node(double v) : float_val_(v), type_(floating) {
            }

            [[nodiscard]] int64_t as_int64() const;
            [[nodiscard]] uint64_t as_uint64() const;
            [[nodiscard]] double as_double() const;

            [[nodiscard]] std::unique_ptr<json_node> clone() const override;
            void dump(std::ostringstream &oss) const override;
            [[nodiscard]] bool equals(const json_node &other) const override;
            [[nodiscard]] bool is_number() const override {
                return true;
            }

        private:
            enum num_type : uint8_t { integer, unsigned_int, floating };
            union {
                int64_t int_val_;
                uint64_t uint_val_;
                double float_val_;
            };
            num_type type_;
        };

        class string_node final: public json_node {
        public:
            explicit string_node(std::string v) : value_(std::move(v)) {
            }
            [[nodiscard]] const std::string &value() const {
                return value_;
            }

            [[nodiscard]] std::unique_ptr<json_node> clone() const override;
            void dump(std::ostringstream &oss) const override;
            [[nodiscard]] bool equals(const json_node &other) const override;
            [[nodiscard]] bool is_string() const override {
                return true;
            }

        private:
            std::string value_;
        };

        class array_node final: public json_node {
        public:
            explicit array_node(json_array v) : value_(std::move(v)) {
            }
            [[nodiscard]] const json_array &value() const {
                return value_;
            }
            [[nodiscard]] json_array &value() {
                return value_;
            }

            [[nodiscard]] std::unique_ptr<json_node> clone() const override;
            void dump(std::ostringstream &oss) const override;
            [[nodiscard]] bool equals(const json_node &other) const override;
            [[nodiscard]] bool is_array() const override {
                return true;
            }

        private:
            json_array value_;
        };

        class object_node final: public json_node {
        public:
            explicit object_node(json_object v) : value_(std::move(v)) {
            }
            [[nodiscard]] const json_object &value() const {
                return value_;
            }
            [[nodiscard]] json_object &value() {
                return value_;
            }

            [[nodiscard]] std::unique_ptr<json_node> clone() const override;
            void dump(std::ostringstream &oss) const override;
            [[nodiscard]] bool equals(const json_node &other) const override;
            [[nodiscard]] bool is_object() const override {
                return true;
            }

        private:
            json_object value_;
        };

    } // namespace detail

    // ===================== json_value: the public value class =====================

    /**
     * @brief A JSON value with polymorphic value semantics.
     *
     * Supports null, boolean, number (int64/uint64/double), string, array, and object.
     * Copyable, movable, and directly usable in STL containers.
     */
    class json_value {
    public:
        // ---- Constructors ----
        json_value();
        json_value(std::nullptr_t);
        json_value(bool v);
        json_value(int v);
        json_value(int64_t v);
        json_value(uint64_t v);
        json_value(unsigned int v);
        json_value(unsigned short v);
        json_value(double v);
        json_value(const char *v);
        json_value(const std::string &v);
        json_value(std::string &&v);
        json_value(const json_array &v);
        json_value(json_array &&v);
        json_value(const json_object &v);
        json_value(json_object &&v);
        json_value(std::initializer_list<std::pair<std::string, json_value>> init);

        // ---- Copy / Move ----
        json_value(const json_value &other);
        json_value(json_value &&other) noexcept;
        json_value &operator=(const json_value &other);
        json_value &operator=(json_value &&other) noexcept;
        ~json_value();

        // ---- Type queries ----
        [[nodiscard]] bool is_null() const;
        [[nodiscard]] bool is_boolean() const;
        [[nodiscard]] bool is_number() const;
        [[nodiscard]] bool is_string() const;
        [[nodiscard]] bool is_array() const;
        [[nodiscard]] bool is_object() const;

        // ---- Value access ----
        template<typename T>
        T get() const;

        template<typename T>
        void get_to(T &v) const {
            v = get<T>();
        }

        // ---- Object access ----
        [[nodiscard]] bool contains(const std::string &key) const;
        const json_value &at(const std::string &key) const;
        json_value &operator[](const std::string &key);
        const json_value &operator[](const std::string &key) const;

        // ---- Array access ----
        const json_value &operator[](size_t idx) const;
        json_value &operator[](size_t idx);
        [[nodiscard]] size_t size() const;

        // ---- Serialization ----
        [[nodiscard]] std::string dump() const;

        // ---- Parsing ----
        static json_value parse(const std::string &input);
        static json_value parse(std::istream &is);

        // ---- Stream operators ----
        friend std::istream &operator>>(std::istream &is, json_value &j);
        friend std::ostream &operator<<(std::ostream &os, const json_value &j);

        // ---- Comparison ----
        friend bool operator==(const json_value &lhs, const json_value &rhs);
        friend bool operator!=(const json_value &lhs, const json_value &rhs);

    private:
        std::unique_ptr<detail::json_node> node_;
    };

    // ===================== Template specialization declarations =====================

    template<>
    std::string json_value::get<std::string>() const;
    template<>
    int json_value::get<int>() const;
    template<>
    int64_t json_value::get<int64_t>() const;
    template<>
    uint64_t json_value::get<uint64_t>() const;
    template<>
    unsigned short json_value::get<unsigned short>() const;
    template<>
    double json_value::get<double>() const;
    template<>
    bool json_value::get<bool>() const;
    template<>
    json_array json_value::get<json_array>() const;
    template<>
    json_object json_value::get<json_object>() const;
    template<>
    std::vector<std::string> json_value::get<std::vector<std::string>>() const;

    // ---- Generic get<T> via ADL from_json ----
    template<typename T>
    T json_value::get() const {
        T val{};
        from_json(*this, val);
        return val;
    }
} // namespace log4cpp
