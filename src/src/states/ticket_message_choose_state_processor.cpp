#include "ticket_message_choose_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>
#include <regex>

namespace NOctoshell {

TReactions TTicketMessageChooseStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("ticket_message_choose_processor").information("on_start from user %" PRIu64, update.UserId);

    TReaction reaction;
    reaction.ForceReply = true;
    reaction.Text = "main.tickets.message.message";
    return {std::move(reaction)};
}

TReactions TTicketMessageChooseStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("ticket_message_choose_processor");
    logger.information("on_update from user %" PRIu64, update.UserId);

    // user entered the message
    (*state.mutable_extradata())["message"] = update.Text;
    state.set_state(TUserState_EState_MAIN_MENU);

    // send request for creating ticket
    std::unordered_map<std::string, std::string> params;
    for (const auto& key : {"project", "topic", "cluster", "subject", "message"}) {
        params[key] = state.extradata().at(key);
    }
    params["method"] = "create_ticket";

    const std::string response = Ctx_.Octoshell().SendQueryWithAuth(state, params);
    logger.information("Ticket creating response: \"%s\"", response);

    return {};
}

} // namespace NOctoshell
