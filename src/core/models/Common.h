#pragma once

#include <algorithm>
#include <cctype>
#include <compare>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace finsight::core::models {

struct Date {
    int year {1970};
    int month {1};
    int day {1};

    [[nodiscard]] auto operator<=>(const Date&) const = default;

    [[nodiscard]] std::string toString() const {
        std::ostringstream stream;
        stream << std::setfill('0')
               << std::setw(4) << year << '-'
               << std::setw(2) << month << '-'
               << std::setw(2) << day;
        return stream.str();
    }

    [[nodiscard]] static Date fromString(const std::string& value) {
        Date date {};
        char sep1 = '\0';
        char sep2 = '\0';
        std::istringstream stream(value);
        if (!(stream >> date.year >> sep1 >> date.month >> sep2 >> date.day) ||
            sep1 != '-' || sep2 != '-') {
            throw std::invalid_argument("Date must be in YYYY-MM-DD format.");
        }
        if (date.month < 1 || date.month > 12 || date.day < 1 || date.day > 31) {
            throw std::invalid_argument("Date contains invalid month or day.");
        }
        return date;
    }
};

struct YearMonth {
    int year {1970};
    int month {1};

    [[nodiscard]] auto operator<=>(const YearMonth&) const = default;
};

inline std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

inline bool containsCaseInsensitive(const std::string& text, const std::string& fragment) {
    return toLower(text).find(toLower(fragment)) != std::string::npos;
}

inline bool inMonth(const Date& date, const YearMonth& period) {
    return date.year == period.year && date.month == period.month;
}

}  // namespace finsight::core::models
