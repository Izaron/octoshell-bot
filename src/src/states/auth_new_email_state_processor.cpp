#include "auth_new_email_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>

#include <inttypes.h>
#include <regex>

namespace NOctoshell {

namespace {

static const std::string MAIL_REGEX_STR = R"((?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9]))\.){3}(?:(2(5[0-5]|[0-4][0-9])|1[0-9][0-9]|[1-9]?[0-9])|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\]))";
static const auto MAIL_REGEX = std::regex(MAIL_REGEX_STR);

} // namespace

TReactions TAuthNewEmailStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("auth_new_email_processor").information("on_start from user %" PRIu64, update.UserId);

    TReaction reaction;
    reaction.Text = "auth.email.message";
    reaction.ForceReply = true;
    return {std::move(reaction)};
}

TReactions TAuthNewEmailStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("auth_new_email_processor");
    logger.information("on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    state.set_state(TUserState_EState_AUTH_SETTINGS);
    if (std::regex_match(update.Text, MAIL_REGEX)) {
        state.set_email(update.Text);
        return {};
    } else {
        TReaction reaction;
        reaction.Text = "auth.email.wrong.message";
        return {std::move(reaction)};
    }
}

} // namespace NOctoshell
