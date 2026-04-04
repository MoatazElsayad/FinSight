#pragma once

#include <filesystem>
#include <optional>
#include <string>

using namespace std;

namespace finsight::core::utils {

// Loads key-value pairs from a .env file into the process environment.
class EnvLoader {
public:
    // Searches upward from the current working directory for the first .env file.
    static optional<filesystem::path> findEnvFile(const filesystem::path& start = filesystem::current_path());

    // Loads the discovered .env file if one exists.
    static bool loadFromNearestFile(const filesystem::path& start = filesystem::current_path());

    // Loads one explicit .env file path into the process environment.
    static bool loadFile(const filesystem::path& path);

    // Reads one environment variable with a fallback default value.
    static string get(const string& key, const string& defaultValue = "");

private:
    // Removes surrounding whitespace from one text value.
    static string trim(const string& value);

    // Removes optional quotes around one .env value.
    static string stripQuotes(const string& value);

    // Writes one key-value pair into the process environment.
    static void setEnvValue(const string& key, const string& value);
};

}  // namespace finsight::core::utils
