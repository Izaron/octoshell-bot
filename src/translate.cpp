#include "translate.h"

#include <Poco/Logger.h>

namespace NOctoshell {

namespace {

std::unordered_map<std::string, TLangTranslate> ConstructLangMap() {
    return {
        {"ru", TLangTranslate{"ru"}},
        {"en", TLangTranslate{"en"}},
    };
}

} // namespace

TLangTranslate::TLangTranslate(const std::string& lang)
    : Map_{new Poco::Util::PropertyFileConfiguration(lang + ".properties")}
{
}

std::string TLangTranslate::Get(const std::string& key) const {
    return Map_->getString(key);
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

void TranslateReaction(TReaction& reaction, const TUserState& state, const TTranslate& translate) {
    const TLangTranslate* lt = [lang = state.language(), &translate]() {
        switch (lang) {
        case TUserState_ELanguage_EN:
            return &translate.LangTranslate("en");
        case TUserState_ELanguage_RU:
            return &translate.LangTranslate("ru");
        default:
            return static_cast<const TLangTranslate*>(nullptr);
        }
    }();

    if (!lt) {
        Poco::Logger::get("translate").error("unknown language");
        return;
    }

    reaction.Text = lt->Get(reaction.Text);
    for (auto& row : reaction.Keyboard) {
        for (auto& text : row) {
            text = lt->Get(text);
        }
    }
}

} // namespace NOctoshell
