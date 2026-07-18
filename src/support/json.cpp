#include "pmzf/support/json.hpp"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace pmzf {

namespace {
class JsonParser {
public:
    explicit JsonParser(std::string_view source)
        : data_(source), index_(0) {
    }

    JsonValue parse() {
        skip_whitespace();
        JsonValue value = parse_value();
        skip_whitespace();
        if (!is_end()) {
            throw std::runtime_error("unexpected trailing data in JSON input");
        }
        return value;
    }

private:
    std::string_view data_;
    std::size_t index_;

    bool is_end() const {
        return index_ >= data_.size();
    }

    char peek() const {
        return data_[index_];
    }

    char get() {
        return data_[index_++];
    }

    void expect(char expected) {
        if (is_end() || get() != expected) {
            throw std::runtime_error("invalid JSON syntax");
        }
    }

    void skip_whitespace() {
        while (!is_end() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
            ++index_;
        }
    }

    JsonValue parse_value() {
        if (is_end()) {
            throw std::runtime_error("unexpected end of JSON input");
        }

        const char token = peek();
        switch (token) {
        case 'n':
            return parse_null();
        case 't':
            return parse_true();
        case 'f':
            return parse_false();
        case '"':
            return JsonValue(parse_string());
        case '[':
            return JsonValue(parse_array());
        case '{':
            return JsonValue(parse_object());
        default:
            if (token == '-' || std::isdigit(static_cast<unsigned char>(token)) != 0) {
                return JsonValue(parse_number());
            }
            throw std::runtime_error("invalid JSON value");
        }
    }

    JsonValue parse_null() {
        expect('n');
        expect('u');
        expect('l');
        expect('l');
        return JsonValue(nullptr);
    }

    JsonValue parse_true() {
        expect('t');
        expect('r');
        expect('u');
        expect('e');
        return JsonValue(true);
    }

    JsonValue parse_false() {
        expect('f');
        expect('a');
        expect('l');
        expect('s');
        expect('e');
        return JsonValue(false);
    }

    double parse_number() {
        const std::size_t start = index_;
        if (peek() == '-') {
            ++index_;
        }
        if (is_end()) {
            throw std::runtime_error("invalid JSON number");
        }
        if (peek() == '0') {
            ++index_;
        }
        else {
            if (std::isdigit(static_cast<unsigned char>(peek())) == 0) {
                throw std::runtime_error("invalid JSON number");
            }
            while (!is_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
                ++index_;
            }
        }
        if (!is_end() && peek() == '.') {
            ++index_;
            if (is_end() || std::isdigit(static_cast<unsigned char>(peek())) == 0) {
                throw std::runtime_error("invalid JSON number");
            }
            while (!is_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
                ++index_;
            }
        }
        if (!is_end() && (peek() == 'e' || peek() == 'E')) {
            ++index_;
            if (!is_end() && (peek() == '+' || peek() == '-')) {
                ++index_;
            }
            if (is_end() || std::isdigit(static_cast<unsigned char>(peek())) == 0) {
                throw std::runtime_error("invalid JSON number");
            }
            while (!is_end() && std::isdigit(static_cast<unsigned char>(peek())) != 0) {
                ++index_;
            }
        }

        const std::string_view segment = data_.substr(start, index_ - start);
        char* end_ptr = nullptr;
        const double value = std::strtod(segment.data(), &end_ptr);
        if (end_ptr != segment.data() + segment.size()) {
            throw std::runtime_error("invalid JSON number");
        }
        return value;
    }

    void append_utf8(std::string& out, char32_t code_point) {
        if (code_point <= 0x7Fu) {
            out.push_back(static_cast<char>(code_point));
        }
        else if (code_point <= 0x7FFu) {
            out.push_back(static_cast<char>(0xC0u | ((code_point >> 6u) & 0x1Fu)));
            out.push_back(static_cast<char>(0x80u | (code_point & 0x3Fu)));
        }
        else if (code_point <= 0xFFFFu) {
            out.push_back(static_cast<char>(0xE0u | ((code_point >> 12u) & 0x0Fu)));
            out.push_back(static_cast<char>(0x80u | ((code_point >> 6u) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | (code_point & 0x3Fu)));
        }
        else {
            out.push_back(static_cast<char>(0xF0u | ((code_point >> 18u) & 0x07u)));
            out.push_back(static_cast<char>(0x80u | ((code_point >> 12u) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | ((code_point >> 6u) & 0x3Fu)));
            out.push_back(static_cast<char>(0x80u | (code_point & 0x3Fu)));
        }
    }

    char32_t parse_hex_quad() {
        if (index_ + 4 > data_.size()) {
            throw std::runtime_error("invalid unicode escape in JSON");
        }
        char32_t value = 0;
        for (std::size_t i = 0; i < 4; ++i) {
            const char ch = data_[index_++];
            value <<= 4u;
            if (ch >= '0' && ch <= '9') {
                value |= static_cast<char32_t>(ch - '0');
            }
            else if (ch >= 'a' && ch <= 'f') {
                value |= static_cast<char32_t>(10 + ch - 'a');
            }
            else if (ch >= 'A' && ch <= 'F') {
                value |= static_cast<char32_t>(10 + ch - 'A');
            }
            else {
                throw std::runtime_error("invalid unicode escape in JSON");
            }
        }
        return value;
    }

    std::string parse_string() {
        expect('"');
        std::string result;
        while (!is_end()) {
            const char ch = get();
            if (ch == '"') {
                return result;
            }
            if (ch == '\\') {
                if (is_end()) {
                    throw std::runtime_error("invalid escape sequence");
                }
                const char escaped = get();
                switch (escaped) {
                case '"':
                    result.push_back('"');
                    break;
                case '\\':
                    result.push_back('\\');
                    break;
                case '/':
                    result.push_back('/');
                    break;
                case 'b':
                    result.push_back('\b');
                    break;
                case 'f':
                    result.push_back('\f');
                    break;
                case 'n':
                    result.push_back('\n');
                    break;
                case 'r':
                    result.push_back('\r');
                    break;
                case 't':
                    result.push_back('\t');
                    break;
                case 'u': {
                    char32_t code_point = parse_hex_quad();
                    if (code_point >= 0xD800u && code_point <= 0xDBFFu) {
                        if (!(index_ + 2 <= data_.size() && data_[index_] == '\\' && data_[index_ + 1] == 'u')) {
                            throw std::runtime_error("invalid unicode surrogate pair");
                        }
                        index_ += 2;
                        char32_t low_surrogate = parse_hex_quad();
                        if (low_surrogate < 0xDC00u || low_surrogate > 0xDFFFu) {
                            throw std::runtime_error("invalid unicode surrogate pair");
                        }
                        code_point = 0x10000u + ((code_point - 0xD800u) << 10u) + (low_surrogate - 0xDC00u);
                    }
                    append_utf8(result, code_point);
                    break;
                }
                default:
                    throw std::runtime_error("invalid escape sequence");
                }
            }
            else {
                result.push_back(ch);
            }
        }
        throw std::runtime_error("unterminated JSON string");
    }

    JsonArray parse_array() {
        expect('[');
        JsonArray values;
        skip_whitespace();
        if (!is_end() && peek() == ']') {
            ++index_;
            return values;
        }
        while (true) {
            skip_whitespace();
            values.push_back(parse_value());
            skip_whitespace();
            if (is_end()) {
                throw std::runtime_error("unterminated JSON array");
            }
            const char token = get();
            if (token == ']') {
                break;
            }
            if (token != ',') {
                throw std::runtime_error("invalid JSON array syntax");
            }
        }
        return values;
    }

    JsonObject parse_object() {
        expect('{');
        JsonObject object;
        skip_whitespace();
        if (!is_end() && peek() == '}') {
            ++index_;
            return object;
        }
        while (true) {
            skip_whitespace();
            if (is_end() || peek() != '"') {
                throw std::runtime_error("invalid JSON object key");
            }
            std::string key = parse_string();
            skip_whitespace();
            expect(':');
            skip_whitespace();
            JsonValue value = parse_value();
            auto inserted = object.emplace(std::move(key), std::move(value));
            if (!inserted.second) {
                throw std::runtime_error("duplicate key in JSON object");
            }
            skip_whitespace();
            if (is_end()) {
                throw std::runtime_error("unterminated JSON object");
            }
            const char token = get();
            if (token == '}') {
                break;
            }
            if (token != ',') {
                throw std::runtime_error("invalid JSON object syntax");
            }
        }
        return object;
    }
};
} // namespace

JsonValue::JsonValue()
    : value_(nullptr) {
}

JsonValue::JsonValue(std::nullptr_t)
    : value_(nullptr) {
}

JsonValue::JsonValue(bool value)
    : value_(value) {
}

JsonValue::JsonValue(double value)
    : value_(value) {
}

JsonValue::JsonValue(std::string value)
    : value_(std::move(value)) {
}

JsonValue::JsonValue(JsonArray value)
    : value_(std::move(value)) {
}

JsonValue::JsonValue(JsonObject value)
    : value_(std::move(value)) {
}

bool JsonValue::is_null() const {
    return std::holds_alternative<std::nullptr_t>(value_);
}

bool JsonValue::is_boolean() const {
    return std::holds_alternative<bool>(value_);
}

bool JsonValue::is_number() const {
    return std::holds_alternative<double>(value_);
}

bool JsonValue::is_string() const {
    return std::holds_alternative<std::string>(value_);
}

bool JsonValue::is_array() const {
    return std::holds_alternative<JsonArray>(value_);
}

bool JsonValue::is_object() const {
    return std::holds_alternative<JsonObject>(value_);
}

bool JsonValue::as_boolean() const {
    return std::get<bool>(value_);
}

double JsonValue::as_number() const {
    return std::get<double>(value_);
}

const std::string& JsonValue::as_string() const {
    return std::get<std::string>(value_);
}

const JsonArray& JsonValue::as_array() const {
    return std::get<JsonArray>(value_);
}

const JsonObject& JsonValue::as_object() const {
    return std::get<JsonObject>(value_);
}

const JsonValue* JsonValue::find(const std::string& key) const {
    if (!is_object()) {
        return nullptr;
    }
    const auto& object = std::get<JsonObject>(value_);
    const auto it = object.find(key);
    if (it == object.end()) {
        return nullptr;
    }
    return &it->second;
}

JsonValue parse_json(std::string_view text) {
    JsonParser parser(text);
    return parser.parse();
}

} // namespace pmzf
