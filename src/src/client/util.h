#pragma once

#include <vector>
#include <string>

namespace NOctoshell {

std::string UrlQuote(std::string s, bool escapeUnderscore = false);
std::vector<std::string> DivideMessage(const std::string& msg, size_t msgSize);

} // namespace NOctoshell
