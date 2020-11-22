#pragma once

#include <Poco/Util/PropertyFileConfiguration.h>

#include <string>
#include <unordered_map>

#include "model.h"
#include <proto/user_state.pb.h>

namespace NOctoshell {

class TLangTranslate {
public:
    TLangTranslate(const std::string& lang);

    std::string Get(const std::string& key) const;
    const std::vector<std::pair<std::string, std::string>>& Values() const;

private:
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> Map_;
    std::vector<std::pair<std::string, std::string>> Values_;
};

class TTranslate {
public:
    TTranslate();

    std::string Get(const std::string& lang, const std::string& key) const;
    const TLangTranslate& LangTranslate(const std::string& lang) const;

private:
    std::unordered_map<std::string, TLangTranslate> LangMap_;
};

void TranslateReaction(TReaction& reaction, const TUserState_ELanguage lang, const TTranslate& translate);
std::string TryGetTemplate(const std::string& text, const TUserState_ELanguage lang, const TTranslate& translate);

} // namespace NOctoshell
