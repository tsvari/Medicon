#include "JsonParameterFormatter.h"
#include <nlohmann/json.hpp>
#include <format>

using json = nlohmann::json;

// ============================================================================
// JsonParameterFormatter Instance Methods
// ============================================================================

std::string JsonParameterFormatter::toJson(bool pretty) const {
    return toJsonString(*this, pretty);
}

void JsonParameterFormatter::fromJson(std::string_view jsonString) {
    clear();  // Clear existing parameters
    auto params = fromJsonString(jsonString);
    for (const auto& [name, value] : params) {
        addParameter(name, FormatterValue{value});
    }
}

bool JsonParameterFormatter::tryFromJson(std::string_view jsonString) noexcept {
    try {
        fromJson(jsonString);
        return true;
    } catch (...) {
        return false;
    }
}

// ============================================================================
// JsonParameterFormatter Static Utilities
// ============================================================================

std::map<std::string, std::string>
JsonParameterFormatter::fromJsonString(std::string_view jsonString) {
    json doc;

    try {
        doc = json::parse(jsonString);
    } catch (const json::parse_error& e) {
        throw JsonFormatterException(
            std::format("{} at byte {}: {}", ERR_INVALID_JSON, e.byte, e.what())
            );
    }

    // Ensure it's an object (not array, string, etc.)
    if (!doc.is_object()) {
        throw JsonFormatterException(ERR_JSON_NOT_OBJECT);
    }

    std::map<std::string, std::string> result;

    for (auto& [key, value] : doc.items()) {
        // Convert all values to strings
        if (value.is_string()) {
            result[key] = value.get<std::string>();
        } else if (value.is_number_integer()) {
            result[key] = std::to_string(value.get<int64_t>());
        } else if (value.is_number_float()) {
            result[key] = std::to_string(value.get<double>());
        } else if (value.is_boolean()) {
            result[key] = value.get<bool>() ? "true" : "false";
        } else if (value.is_null()) {
            result[key] = "NULL";
        } else {
            // For complex types (arrays, objects), serialize to string
            result[key] = value.dump();
        }
    }

    return result;
}

std::optional<std::map<std::string, std::string>>
JsonParameterFormatter::tryFromJsonString(std::string_view jsonString) noexcept {
    try {
        return fromJsonString(jsonString);
    } catch (...) {
        return std::nullopt;
    }
}

std::string
JsonParameterFormatter::toJsonString(const std::map<std::string, std::string>& parameters,
                                     bool pretty) {
    json doc = json::object();

    for (const auto& [key, value] : parameters) {
        doc[key] = value;
    }

    return pretty ? doc.dump(4) : doc.dump();
}

std::string
JsonParameterFormatter::toJsonString(const TypeToStringFormatter& formatter, bool pretty) {
    return toJsonString(formatter.toMap(), pretty);
}

bool JsonParameterFormatter::isValidJson(std::string_view jsonString) noexcept {
    try {
        auto doc = json::parse(jsonString);
        return doc.is_object();
    } catch (...) {
        return false;
    }
}

std::string JsonParameterFormatter::getParseError(std::string_view jsonString) {
    try {
        json::parse(jsonString);
        return "No error - JSON is valid";
    } catch (const json::parse_error& e) {
        size_t startPos = (e.byte >= 20) ? (e.byte - 20) : 0;
        size_t length = std::min(size_t(40), jsonString.size() - startPos);
        return std::format("Parse error at byte {}: {}\nNear: {}",
                           e.byte,
                           e.what(),
                           jsonString.substr(startPos, length));
    } catch (const std::exception& e) {
        return std::format("Error: {}", e.what());
    }
}
