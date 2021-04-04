#include "auth_settings_state_processor.h"
#include "../auth_status.h"
#include "../context.h"
#include "../translate.h"

#include <Poco/Logger.h>

#include <inttypes.h>
#include <sstream>

namespace NOctoshell {

namespace {

std::string ConstructShowSettingsText(const TUserState& state) {
    std::stringstream ss;
    ss << "auth.settings.header" << std::endl;
    ss << "auth.settings.email" << ": " << (state.email().empty() ? "auth.blank-email" : state.email()) << std::endl;
    ss << "auth.settings.token" << ": " << (state.token().empty() ? "auth.blank-token" : state.token()) << std::endl;
    return ss.str();
}

TReaction ConstructShowSettingsReaction(const TUserState& state) {
    TReaction reaction;
    reaction.Text = ConstructShowSettingsText(state);
    return reaction;
}

} // namespace

TReactions TAuthSettingsStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("auth_settings_processor").information("on_start from user %" PRIu64, update.UserId);

    const static TReaction::TKeyboard keyboardTemplate = {
        {"auth.button.change-email", "auth.button.change-token"},
        {"auth.button.show-settings"},
        {"auth.button.check-connection"},
        {"auth.button.back"},
    };

    TReaction reaction;
    reaction.Text = "auth.message";
    reaction.Keyboard = keyboardTemplate;
    return {std::move(reaction)};
}

TReactions TAuthSettingsStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("auth_settings_processor");
    logger.information("on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    if (code == "auth.button.change-email") {
        state.set_state(TUserState_EState_AUTH_NEW_EMAIL);
        return {};
    }

    if (code == "auth.button.change-token") {
        state.set_state(TUserState_EState_AUTH_NEW_TOKEN);
        return {};
    }

    if (code == "auth.button.show-settings") {
        return {ConstructShowSettingsReaction(state)};
    }

    if (code == "auth.button.check-connection") {
        const EAuthStatus authStatus = TryAuth(Ctx_.Octoshell(), state);
        TReaction reaction;
        reaction.Text = AuthToTemplateMap.at(authStatus);
        return {std::move(reaction)};
    }

    if (code == "auth.button.back") {
        state.set_state(TUserState_EState_MAIN_MENU);
        return {};
    }

    return {};
}

} // namespace NOctoshell
