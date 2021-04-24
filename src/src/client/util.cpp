#include "util.h"

#include <regex>

namespace NOctoshell {

std::string UrlQuote(std::string s, bool escapeUnderscore) {
    s = std::regex_replace(s, std::regex(" "), "%20");
    s = std::regex_replace(s, std::regex("\n"), "%0A");
    s = std::regex_replace(s, std::regex("\r"), "%0D");

    std::string res;
    if (escapeUnderscore) {
        bool inCode = false;
        for (const auto c : s) {
            if (c == '`') {
                inCode = !inCode;
            }
            if (c == '_' && !inCode) {
                res += "\\";
            }
            res += c;
        }
    } else {
        res = s;
    }

    return res;
}

std::vector<std::string> DivideMessage(const std::string& msg, size_t msgSize) {
    std::vector<std::string> res;

    std::string curMessage = "";
    std::string curLine = "";
    for (size_t i = 0; i < msg.size(); ++i) {
        curLine.push_back(msg[i]);
        if (i == msg.size() - 1 || msg[i] == '\n') {
            if (curMessage.size() + curLine.size() > msgSize) {
                res.push_back(std::move(curMessage));
                curMessage = curLine;
            } else {
                curMessage += curLine;
            }
            curLine = "";
        }
    }
    if (!curLine.empty()) {
        if (curMessage.size() + curLine.size() > msgSize) {
            res.push_back(std::move(curMessage));
            curMessage = curLine;
        } else {
            curMessage += curLine;
        }
    }
    if (!curMessage.empty()) {
        res.push_back(std::move(curMessage));
    }

    return res;
}

} // namespace NOctoshell
