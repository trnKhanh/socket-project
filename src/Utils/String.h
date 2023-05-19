#pragma once
#include <string>
#include <vector>

namespace String {
    int split(const std::string &str, const std::string &c, std::vector<std::string> &res);
    int join(const std::vector<std::string> &strs, const std::string &c, std::string &res);
}