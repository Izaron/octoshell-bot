#include "translate.h"

#include <queue>
#include <regex>

#include <Poco/Logger.h>

namespace NOctoshell {

namespace {

std::unordered_map<std::string, TLangTranslate> ConstructLangMap() {
    return {
        {"ru", TLangTranslate{"ru"}},
        {"en", TLangTranslate{"en"}},
    };
}

std::vector<std::pair<std::string, std::string>> ConstructValues(const Poco::Util::PropertyFileConfiguration& conf) {
    std::vector<std::pair<std::string, std::string>> res;

    std::queue<std::string> q;
    q.push("");
    while (!q.empty()) {
        std::string prefix = q.front();
        q.pop();

        std::vector<std::string> child;
        conf.keys(prefix, child);

        if (child.empty()) {
            std::string value = conf.getString(prefix);
            res.emplace_back(std::move(prefix), std::move(value));
        } else {
            for (auto&& v : child) {
                if (!prefix.empty()) {
                    q.push(prefix + "." + std::move(v));
                } else {
                    q.push(std::move(v));
                }
            }
        }
    }

    return res;
}

const TLangTranslate* FindLangTranslate(const TUserState_ELanguage lang, const TTranslate& translate) {
    switch (lang) {
    case TUserState_ELanguage_EN:
        return &translate.LangTranslate("en");
    case TUserState_ELanguage_RU:
        return &translate.LangTranslate("ru");
    default:
        return static_cast<const TLangTranslate*>(nullptr);
    }
}

} // namespace

TLangTranslate::TLangTranslate(const std::string& lang)
    : Map_{new Poco::Util::PropertyFileConfiguration(lang + ".properties")}
    , Values_{ConstructValues(*Map_)}
{
}

std::string TLangTranslate::Get(const std::string& key) const {
    return Map_->getString(key);
}

const std::vector<std::pair<std::string, std::string>>& TLangTranslate::Values() const {
    return Values_;
}

TTranslate::TTranslate()
    : LangMap_{ConstructLangMap()}
{
}

std::string TTranslate::Get(const std::string& lang, const std::string& key) const {
    return LangTranslate(lang).Get(key);
}

const TLangTranslate& TTranslate::LangTranslate(const std::string& lang) const {
    return LangMap_.find(lang)->second;
}

void TranslateReaction(TReaction& reaction, const TUserState_ELanguage lang, const TTranslate& translate) {
    const TLangTranslate* lt = FindLangTranslate(lang, translate);
    if (!lt) {
        Poco::Logger::get("translate").error("unknown language");
        return;
    }

    const auto func = [val = lt->Values()](std::string& text) {
        for (const auto& [key, value] : val) {
            if (text.find(key) != std::string::npos) {
                text = std::regex_replace(text, std::regex(key), value);
            }
        }
    };

    func(reaction.Text);
    for (auto& row : reaction.Keyboard) {
        for (auto& text : row) {
            func(text);
        }
    }
}

std::string TryGetTemplate(const std::string& text, const TUserState_ELanguage lang, const TTranslate& translate) {
    const TLangTranslate* lt = FindLangTranslate(lang, translate);
    if (!lt) {
        Poco::Logger::get("translate").error("unknown language");
        return "";
    }

    for (const auto& [key, value] : lt->Values()) {
        if (text.find(value) != std::string::npos) {
            return key;
        }
    }
    return "";
}

} // namespace NOctoshell
