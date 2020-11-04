#include "main_menu_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>

#include <inttypes.h>

namespace NOctoshell {

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

    TranslateReaction(reaction, state, Ctx_.Translate());

    return {std::move(reaction)};
}

TReactions TMainMenuStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("main_menu_processor");
    logger.information("main menu on_update from user %" PRIu64, update.UserId);

    return {};
}

} // namespace NOctoshell
