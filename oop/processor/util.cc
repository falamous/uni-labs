#include <algorithm>
#include <vector>
#include <cinttypes>
#include <bits/stdc++.h>
#include "util.h"

/**
 * Return the string with all characters converted to lowercase.
 * @param[in] s
 */
std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

/**
 * Return a go-like pair int, bool, where int is the string converted to int
 * and bool represents if it was successful.
 * @param[in] s
 */
std::pair<uint32_t, bool> string_to_int(std::string s) {
    uint32_t res;
    long long read;

    if (sscanf(s.c_str(), "%u%lln", &res, &read) != 1 || read != s.length()) {
        return std::make_pair(
                0,
                false
                );
    }
    return std::make_pair(
            res,
            true
            );
}

/**
 * Return the string with all whitespace characters filtered out.
 * @param[in] s
 */
std::string filterwhitespace(std::string s) {
    std::string res;
    for(auto c: s) {
        if (!isblank(c)) {
            res += c;
        }
    }
    return res;
}

/**
 * Return the string with whitespace deleted on the right.
 * @param[in] s
 */
std::string rstrip(std::string s) {
    for(ssize_t i = s.size() - 1; i >= 0; i--) {
        if (!isblank(s[i])) {
            return s.substr(0, i + 1);
        }
    }
    return "";
}

/**
 * Return the string with whitespace deleted on the left.
 * @param[in] s
 */
std::string lstrip(std::string s) {
    for(ssize_t i = 0; i < s.size(); i++) {
        if (!isblank(s[i])) {
            return s.substr(i);
        }
    }
    return "";
}

/**
 * Returm the string with whitespace deleted on both sides.
 * @param[in] s
 */
std::string strip(std::string s) {
    return lstrip(rstrip(s));
}

/**
 * Return the list of string parts separated by the delimiter.
 * @param[in] s
 * @param[in] delim
 * @param[out] res
 */
std::vector<std::string> string_split(std::string s, std::string delim) {
    std::vector<std::string> res;
    size_t pos;
    size_t prev_pos = 0;

    while ((pos = s.find(delim, prev_pos)) != std::string::npos) {
        res.push_back(s.substr(prev_pos, pos));
        prev_pos = pos + 1;
    }
    res.push_back(s.substr(prev_pos));

    return res;
}
