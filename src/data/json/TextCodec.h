#pragma once

#include <string>
#include <vector>

using namespace std;

namespace finsight::data::json {

// Escapes control characters before writing text to a file.
string escape(const string& value);
// Restores escaped control characters after reading text from a file.
string unescape(const string& value);
// Encodes one row of fields into a tab-separated line.
string encodeRow(const vector<string>& fields);
// Decodes one tab-separated line back into fields.
vector<string> decodeRow(const string& row);
// Encodes a string list into one storage field.
string encodeList(const vector<string>& values);
// Decodes a stored list field back into strings.
vector<string> decodeList(const string& value);

}  // namespace finsight::data::json
