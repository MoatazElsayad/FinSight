#include "JsonSerializer.h"

#include <sstream>
#include <algorithm>

namespace finsight::network::protocol {

std::string JsonSerializer::escapeString(const std::string& value) {
    std::string escaped;
    for (char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

std::string JsonSerializer::serialize(const std::unordered_map<std::string, std::string>& object) {
    std::ostringstream stream;
    stream << "{";
    bool first = true;
    for (const auto& pair : object) {
        if (!first) {
            stream << ",";
        }
        stream << "\"" << escapeString(pair.first) << "\":\"" << escapeString(pair.second) << "\"";
        first = false;
    }
    stream << "}";
    return stream.str();
}

std::string JsonSerializer::serialize(const std::vector<std::unordered_map<std::string, std::string>>& array) {
    std::ostringstream stream;
    stream << "[";
    for (size_t i = 0; i < array.size(); ++i) {
        if (i > 0) {
            stream << ",";
        }
        stream << serialize(array[i]);
    }
    stream << "]";
    return stream.str();
}

std::string JsonSerializer::getString(const std::string& json, const std::string& key) {
    auto search = "\"" + key + "\":\"";
    auto pos = json.find(search);
    if (pos == std::string::npos) {
        search = "\"" + key + "\": \"";
        pos = json.find(search);
    }
    if (pos == std::string::npos) {
        return "";
    }
    pos += search.size();
    
    std::string result;
    bool escaped = false;
    for (size_t i = pos; i < json.size(); ++i) {
        char ch = json[i];
        if (escaped) {
            switch (ch) {
            case 'n':
                result.push_back('\n');
                break;
            case 'r':
                result.push_back('\r');
                break;
            case 't':
                result.push_back('\t');
                break;
            default:
                result.push_back(ch);
                break;
            }
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            break;
        }
        result.push_back(ch);
    }
    return result;
}

int JsonSerializer::getInt(const std::string& json, const std::string& key) {
    auto search = "\"" + key + "\":";
    auto pos = json.find(search);
    if (pos == std::string::npos) {
        return 0;
    }
    pos += search.size();
    
    std::string numStr;
    for (size_t i = pos; i < json.size(); ++i) {
        char ch = json[i];
        if ((ch >= '0' && ch <= '9') || ch == '-' || ch == '.') {
            numStr.push_back(ch);
        } else {
            break;
        }
    }
    return std::stoi(numStr);
}

bool JsonSerializer::getBool(const std::string& json, const std::string& key) {
    auto search = "\"" + key + "\":";
    auto pos = json.find(search);
    if (pos == std::string::npos) {
        return false;
    }
    pos += search.size();
    
    std::string trueSearch = "true";
    if (json.substr(pos, trueSearch.size()) == trueSearch) {
        return true;
    }
    return false;
}

bool JsonSerializer::hasKey(const std::string& json, const std::string& key) {
    auto search = "\"" + key + "\":";
    return json.find(search) != std::string::npos;
}

}  // namespace finsight::network::protocol