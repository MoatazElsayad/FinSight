#include "TextCodec.h"

using namespace std;

namespace finsight::data::json {

// Escapes tabs, newlines, and slashes before persistence.
string escape(const string& value) {
    string escaped;
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

// Restores escaped characters after reading stored text.
string unescape(const string& value) {
    string unescaped;
    unescaped.reserve(value.size());
    for (size_t index = 0; index < value.size(); ++index) {
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

// Joins many fields into one tab-separated row.
string encodeRow(const vector<string>& fields) {
    string row;
    for (size_t index = 0; index < fields.size(); ++index) {
        if (index > 0) {
            row.push_back('\t');
        }
        row += escape(fields[index]);
    }
    return row;
}

// Splits one stored row back into separate fields.
vector<string> decodeRow(const string& row) {
    vector<string> fields;
    string current;
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

// Joins a list of strings into one escaped field.
string encodeList(const vector<string>& values) {
    string encoded;
    for (size_t index = 0; index < values.size(); ++index) {
        if (index > 0) {
            encoded.push_back('|');
        }
        encoded += escape(values[index]);
    }
    return encoded;
}

// Splits one escaped list field back into separate values.
vector<string> decodeList(const string& value) {
    vector<string> result;
    string current;
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
