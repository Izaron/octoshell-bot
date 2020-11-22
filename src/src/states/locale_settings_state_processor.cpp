#include "locale_settings_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>

#include <inttypes.h>

namespace NOctoshell {

TReactions TLocaleSettingsStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("locale_settings_processor").information("on_start from user %" PRIu64, update.UserId);

    TReaction reaction;
    reaction.Text = "locale.message";
    reaction.Keyboard = {{"locale.button.russian", "locale.button.english"}};
    return {std::move(reaction)};
}

TReactions TLocaleSettingsStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("locale_settings_processor");
    logger.information("main menu on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    if (code == "locale.button.russian") {
        state.set_state(TUserState_EState_MAIN_MENU);
        state.set_language(TUserState_ELanguage_RU);
        return {};
    }

    if (code == "locale.button.english") {
        state.set_state(TUserState_EState_MAIN_MENU);
        state.set_language(TUserState_ELanguage_EN);
        return {};
    }

    return {};
}

} // namespace NOctoshell
