#include "TextCodec.h"

namespace finsight::data::json {

std::string escape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '\t':
            escaped += "\\t";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

std::string unescape(const std::string& value) {
    std::string unescaped;
    unescaped.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] == '\\' && index + 1 < value.size()) {
            ++index;
            switch (value[index]) {
            case 't':
                unescaped.push_back('\t');
                break;
            case 'n':
                unescaped.push_back('\n');
                break;
            case 'r':
                unescaped.push_back('\r');
                break;
            case '\\':
                unescaped.push_back('\\');
                break;
            default:
                unescaped.push_back(value[index]);
                break;
            }
        } else {
            unescaped.push_back(value[index]);
        }
    }
    return unescaped;
}

std::string encodeRow(const std::vector<std::string>& fields) {
    std::string row;
    for (std::size_t index = 0; index < fields.size(); ++index) {
        if (index > 0) {
            row.push_back('\t');
        }
        row += escape(fields[index]);
    }
    return row;
}

std::vector<std::string> decodeRow(const std::string& row) {
    std::vector<std::string> fields;
    std::string current;
    bool escaped = false;

    for (char ch : row) {
        if (escaped) {
            current.push_back('\\');
            current.push_back(ch);
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '\t') {
            fields.push_back(unescape(current));
            current.clear();
            continue;
        }
        current.push_back(ch);
    }

    if (escaped) {
        current.push_back('\\');
    }
    fields.push_back(unescape(current));
    return fields;
}

std::string encodeList(const std::vector<std::string>& values) {
    std::string encoded;
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            encoded.push_back('|');
        }
        encoded += escape(values[index]);
    }
    return encoded;
}

std::vector<std::string> decodeList(const std::string& value) {
    std::vector<std::string> result;
    std::string current;
    bool escaped = false;

    for (char ch : value) {
        if (escaped) {
            current.push_back('\\');
            current.push_back(ch);
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '|') {
            result.push_back(unescape(current));
            current.clear();
            continue;
        }
        current.push_back(ch);
    }

    if (escaped) {
        current.push_back('\\');
    }
    if (!current.empty() || !value.empty()) {
        result.push_back(unescape(current));
    }
    return result;
}

}  // namespace finsight::data::json
