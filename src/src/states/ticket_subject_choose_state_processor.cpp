#include "ticket_subject_choose_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>
#include <regex>

namespace NOctoshell {

TReactions TTicketSubjectChooseStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("ticket_subject_choose_processor").information("on_start from user %" PRIu64, update.UserId);

    TReaction reaction;
    reaction.ForceReply = true;
    reaction.Text = "main.tickets.subject.message";
    return {std::move(reaction)};
}

TReactions TTicketSubjectChooseStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    Poco::Logger::get("ticket_subject_choose_processor").information("on_update from user %" PRIu64, update.UserId);

    // user entered the subject
    (*state.mutable_extradata())["subject"] = update.Text;
    state.set_state(TUserState_EState_TICKET_MESSAGE_CHOOSE);
    return {};
}

} // namespace NOctoshell
