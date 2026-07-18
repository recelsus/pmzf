#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace pmzf {

class JsonValue;

using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;

class JsonValue {
public:
    using StorageType = std::variant<std::nullptr_t, bool, double, std::string, JsonArray, JsonObject>;

    JsonValue();
    explicit JsonValue(std::nullptr_t);
    explicit JsonValue(bool value);
    explicit JsonValue(double value);
    explicit JsonValue(std::string value);
    explicit JsonValue(JsonArray value);
    explicit JsonValue(JsonObject value);

    bool is_null() const;
    bool is_boolean() const;
    bool is_number() const;
    bool is_string() const;
    bool is_array() const;
    bool is_object() const;

    bool as_boolean() const;
    double as_number() const;
    const std::string& as_string() const;
    const JsonArray& as_array() const;
    const JsonObject& as_object() const;

    const JsonValue* find(const std::string& key) const;

private:
    StorageType value_;
};

JsonValue parse_json(std::string_view text);

} // namespace pmzf
