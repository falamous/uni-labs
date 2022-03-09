#pragma once

#include <string>
#include <vector>

std::string to_lower(std::string s);
std::pair<uint32_t, bool> string_to_int(std::string);
std::string filterwhitespace(std::string s);
std::vector<std::string> string_split(std::string s, std::string delim);
std::string lstrip(std::string s);
std::string rstrip(std::string s);
std::string strip(std::string s);
