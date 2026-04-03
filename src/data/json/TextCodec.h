#pragma once

#include <string>
#include <vector>

namespace finsight::data::json {

std::string escape(const std::string& value);
std::string unescape(const std::string& value);
std::string encodeRow(const std::vector<std::string>& fields);
std::vector<std::string> decodeRow(const std::string& row);
std::string encodeList(const std::vector<std::string>& values);
std::vector<std::string> decodeList(const std::string& value);

}  // namespace finsight::data::json
