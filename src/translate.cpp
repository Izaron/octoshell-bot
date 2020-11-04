#include "translate.h"

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

} // namespace NOctoshell
