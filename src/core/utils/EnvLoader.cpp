#include "core/utils/EnvLoader.h"

#include <cctype>
#include <cstdlib>
#include <fstream>

using namespace std;

namespace finsight::core::utils {

optional<filesystem::path> EnvLoader::findEnvFile(const filesystem::path& start) {
    filesystem::path current = filesystem::absolute(start);

    while (true) {
        const filesystem::path candidate = current / ".env";
        if (filesystem::exists(candidate) && filesystem::is_regular_file(candidate)) {
            return candidate;
        }

        if (current == current.root_path()) {
            break;
        }
        current = current.parent_path();
    }

    return nullopt;
}

bool EnvLoader::loadFromNearestFile(const filesystem::path& start) {
    const auto envPath = findEnvFile(start);
    if (!envPath.has_value()) {
        return false;
    }

    return loadFile(*envPath);
}

bool EnvLoader::loadFile(const filesystem::path& path) {
    ifstream input(path);
    if (!input) {
        return false;
    }

    string line;
    while (getline(input, line)) {
        const string trimmedLine = trim(line);
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            continue;
        }

        const size_t separator = trimmedLine.find('=');
        if (separator == string::npos) {
            continue;
        }

        string key = trim(trimmedLine.substr(0, separator));
        string value = stripQuotes(trim(trimmedLine.substr(separator + 1)));
        if (key.empty()) {
            continue;
        }

        setEnvValue(key, value);
    }

    return true;
}

string EnvLoader::get(const string& key, const string& defaultValue) {
    const char* value = getenv(key.c_str());
    return value == nullptr ? defaultValue : value;
}

string EnvLoader::trim(const string& value) {
    size_t start = 0;
    while (start < value.size() && isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }

    size_t end = value.size();
    while (end > start && isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }

    return value.substr(start, end - start);
}

string EnvLoader::stripQuotes(const string& value) {
    if (value.size() >= 2) {
        const char first = value.front();
        const char last = value.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return value.substr(1, value.size() - 2);
        }
    }
    return value;
}

void EnvLoader::setEnvValue(const string& key, const string& value) {
#ifdef _WIN32
    _putenv_s(key.c_str(), value.c_str());
#else
    setenv(key.c_str(), value.c_str(), 1);
#endif
}

}  // namespace finsight::core::utils
