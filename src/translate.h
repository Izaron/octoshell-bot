#pragma once

#include <Poco/Util/PropertyFileConfiguration.h>

#include <string>
#include <unordered_map>

namespace NOctoshell {

class TLangTranslate {
public:
    TLangTranslate(const std::string& lang);
    std::string Get(const std::string& key) const;

private:
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> Map_;
};

class TTranslate {
public:
    TTranslate();

    std::string Get(const std::string& lang, const std::string& key) const;
    const TLangTranslate& LangTranslate(const std::string& lang) const;

private:
    std::unordered_map<std::string, TLangTranslate> LangMap_;
};

} // namespace NOctoshell
