#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace finsight::network::protocol {

class JsonSerializer {
public:
    static std::string escapeString(const std::string& value);
    
    static std::string serialize(const std::unordered_map<std::string, std::string>& object);
    
    static std::string serialize(const std::vector<std::unordered_map<std::string, std::string>>& array);
    
    static std::string getString(const std::string& json, const std::string& key);
    static int getInt(const std::string& json, const std::string& key);
    static bool getBool(const std::string& json, const std::string& key);
    static bool hasKey(const std::string& json, const std::string& key);
};

}  // namespace finsight::network::protocol