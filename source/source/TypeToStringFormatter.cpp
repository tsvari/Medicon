#include "TypeToStringFormatter.h"
#include <cassert>
#include <sstream>
#include <format>
#include <random>
#include <algorithm>
#include <iomanip>

// ============================================================================
// Time Formatting Utilities
// ============================================================================

using std::map;
using std::string;
using std::string_view;
namespace timeFormatter {

string toString(std::chrono::milliseconds timePoint, DataInfo::Type type) {
    auto seconds = std::chrono::floor<std::chrono::seconds>(
        std::chrono::sys_time<std::chrono::milliseconds>{timePoint}
        );

    try {
        switch (type) {
        case DataInfo::DateTime:
            return std::format("{:%Y-%m-%d %H:%M:%S}", seconds);
        case DataInfo::DateTimeNoSec:
            return std::format("{:%Y-%m-%d %H:%M}", seconds);
        case DataInfo::Date:
            return std::format("{:%Y-%m-%d}", seconds);
        case DataInfo::Time:
            return std::format("{:%H:%M:%S}", seconds);
        default:
            throw FormatterException(ERR_WRONG_DATE_TIME_TYPE);
        }
    } catch (const std::format_error& e) {
        throw FormatterException(
            std::format("{} - {}", ERR_CHRONO_FORMAT, e.what())
            );
    }
}

string toString(int64_t milliseconds, DataInfo::Type type) {
    return toString(std::chrono::milliseconds{milliseconds}, type);
}

std::chrono::milliseconds fromString(string_view formatted, DataInfo::Type type) {
    std::chrono::sys_time<std::chrono::milliseconds> timePoint;
    string inputStr(formatted);

    // For Time type without date, prepend a date for parsing
    if (type == DataInfo::Time) {
        inputStr = std::format("1970-01-01 {}", formatted);
    }

    std::istringstream ss(inputStr);

    switch (type) {
    case DataInfo::DateTime:
        ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", timePoint);
        break;
    case DataInfo::DateTimeNoSec:
        ss >> std::chrono::parse("%Y-%m-%d %H:%M", timePoint);
        break;
    case DataInfo::Date:
        ss >> std::chrono::parse("%Y-%m-%d", timePoint);
        break;
    case DataInfo::Time:
        ss >> std::chrono::parse("%Y-%m-%d %H:%M:%S", timePoint);
        break;
    default:
        throw FormatterException(ERR_WRONG_DATE_TIME_TYPE);
    }

    if (ss.fail()) {
        throw FormatterException(
            std::format("{} Input: '{}'", ERR_STRING_FORMAT, formatted)
            );
    }

    return timePoint.time_since_epoch();
}

std::chrono::milliseconds now() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
        );
}

string generateUniqueId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 255);

    std::vector<uint8_t> bytes(32);  // 256-bit unique ID
    std::generate(bytes.begin(), bytes.end(), [&]() { return dist(gen); });

    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    for (uint8_t byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

} // namespace timeFormatter

// ============================================================================
// TypeToStringFormatter Implementation
// ============================================================================

void TypeToStringFormatter::addParameter(string_view name, const FormatterValue& value) {
    DataInfo info;
    info.param = name;

    std::visit([&info](auto&& val) {
        using T = std::decay_t<decltype(val)>;

        if constexpr (std::is_same_v<T, int>) {
            info.value = std::to_string(val);
            info.type = DataInfo::Int;
        } else if constexpr (std::is_same_v<T, int64_t>) {
            info.value = std::to_string(val);
            info.type = DataInfo::Int64;
        } else if constexpr (std::is_same_v<T, double>) {
            info.value = std::to_string(val);
            info.type = DataInfo::Double;
        } else if constexpr (std::is_same_v<T, string>) {
            info.value = val;
            info.type = DataInfo::String;
        } else if constexpr (std::is_same_v<T, bool>) {
            info.value = std::to_string(val ? 1 : 0);
            info.type = DataInfo::Bool;
        } else {
            throw FormatterException(ERR_WRONG_TYPE);
        }
    }, value);

    addToList(std::move(info));
}

void TypeToStringFormatter::addParameter(string_view name,
                                         std::chrono::milliseconds value,
                                         DataInfo::Type type) {
    if (!timeFormatter::isDateTimeType(type)) {
        throw FormatterException(
            std::format("{} for parameter '{}'", ERR_WRONG_DATE_TIME_TYPE, name)
            );
    }

    DataInfo info;
    info.param = name;
    info.value = timeFormatter::toString(value, type);
    info.type = type;

    addToList(std::move(info));
}

void TypeToStringFormatter::addParameter(string_view name,
                                         string_view value,
                                         DataInfo::Type type) {
    if (!timeFormatter::isDateTimeType(type)) {
        throw FormatterException(
            std::format("{} for parameter '{}'", ERR_WRONG_DATE_TIME_TYPE, name)
            );
    }

    // Validate by parsing and re-formatting
    auto timePoint = timeFormatter::fromString(value, type);

    DataInfo info;
    info.param = name;
    info.value = timeFormatter::toString(timePoint, type);
    info.type = type;

    addToList(std::move(info));
}

std::optional<string_view> TypeToStringFormatter::getValue(string_view name) const {
    if (const auto* info = findByName(name)) {
        return info->value;
    }
    return std::nullopt;
}

string_view TypeToStringFormatter::getValueOrThrow(string_view name) const {
    if (auto value = getValue(name)) {
        return *value;
    }
    throw FormatterException(std::format("{}{}", ERR_WRONG_KEY_PARAMETER, name));
}

std::optional<DataInfo> TypeToStringFormatter::getInfo(string_view name) const {
    if (const auto* info = findByName(name)) {
        return *info;
    }
    return std::nullopt;
}

const DataInfo& TypeToStringFormatter::getInfoOrThrow(string_view name) const {
    if (const auto* info = findByName(name)) {
        return *info;
    }
    throw FormatterException(std::format("{}{}", ERR_WRONG_KEY_PARAMETER, name));
}

std::chrono::milliseconds TypeToStringFormatter::getAsTime(string_view name) const {
    const auto& info = getInfoOrThrow(name);

    if (!timeFormatter::isDateTimeType(info.type)) {
        throw FormatterException(
            std::format("{} for parameter '{}'", ERR_WRONG_DATE_TIME_TYPE, name)
            );
    }

    // fromString already handles Time type by prepending date
    return timeFormatter::fromString(info.value, info.type);
}

map<string, string> TypeToStringFormatter::toMap() const {
    map<string, string> result;
    for (const auto& info : m_dataList) {
        result.emplace(info.param, info.value);
    }
    return result;
}

bool TypeToStringFormatter::contains(string_view name) const {
    return findByName(name) != nullptr;
}

void TypeToStringFormatter::addToList(DataInfo info) {
    size_t index = m_dataList.size();
    string nameKey = info.param;  // Copy for map key
    m_dataList.push_back(std::move(info));
    m_lookupMap[std::move(nameKey)] = index;
}

const DataInfo* TypeToStringFormatter::findByName(string_view name) const {
    auto it = m_lookupMap.find(string(name));
    if (it != m_lookupMap.end() && it->second < m_dataList.size()) {
        return &m_dataList[it->second];
    }
    return nullptr;
}
