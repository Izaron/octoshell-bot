#include "auth_new_token_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>

#include <inttypes.h>
#include <regex>

namespace NOctoshell {

TReactions TAuthNewTokenStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("auth_new_token_processor").information("on_start from user %" PRIu64, update.UserId);

    TReaction reaction;
    reaction.Text = "auth.token.message";
    reaction.ForceReply = true;
    return {std::move(reaction)};
}

TReactions TAuthNewTokenStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("auth_new_token_processor");
    logger.information("on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    state.set_state(TUserState_EState_AUTH_SETTINGS);
    if (!update.Text.empty()) {
        state.set_token(update.Text);
    }
    return {};
}

} // namespace NOctoshell
