#include "main_menu_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>

#include <inttypes.h>

namespace NOctoshell {

namespace {

TReaction ConstructInformationReaction() {
    TReaction reaction;
    reaction.Text = "main.information";
    return reaction;
}

} // namespace

TReactions TMainMenuStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("main_menu_processor").information("main menu on_start from user %" PRIu64, update.UserId);

    const static TReaction::TKeyboard keyboardTemplate = {
        {"main.button.show-user-projects"},
        {"main.button.show-tickets", "main.button.create-tickets"},
        {"main.button.to-auth-settings"},
        {"main.button.to-locale-settings"},
        {"main.button.information"},
    };

    TReaction reaction;
    reaction.Text = "main.message";
    reaction.Keyboard = keyboardTemplate;
    return {std::move(reaction)};
}

TReactions TMainMenuStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("main_menu_processor");
    logger.information("main menu on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    if (code == "main.button.to-auth-settings") {
        state.set_state(TUserState_EState_AUTH_SETTINGS);
        return {};
    }

    if (code == "main.button.show-user-projects") {
        // TODO: show
        return {};
    }

    if (code == "main.button.show-tickets") {
        // TODO: show
        return {};
    }

    if (code == "main.button.create-tickets") {
        state.set_state(TUserState_EState_TICKET_PROJECT_CHOOSE);
        return {};
    }

    if (code == "main.button.to-locale-settings") {
        state.set_state(TUserState_EState_LOCALE_SETTINGS);
        return {};
    }

    if (code == "main.button.information") {
        return {ConstructInformationReaction()};
    }

    return {};
}

} // namespace NOctoshell
