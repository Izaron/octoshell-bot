#include "ticket_topic_choose_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>
#include <regex>

namespace NOctoshell {

TReactions TTicketTopicChooseStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("ticket_topic_choose_processor").information("on_start from user %" PRIu64, update.UserId);
    TReaction reaction;

    const std::string response = Ctx_.Octoshell().SendQueryWithAuth(state, {{"method", "topics"}});

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        reaction.Text = "main.tickets.topics.message";

        const std::string locale = state.language() == TUserState_ELanguage_EN ? "en" : "ru";
        auto topicsArr = object->getArray("topics");

        for (size_t i = 0; i < topicsArr->size(); ++i) {
            auto topic = topicsArr->getObject(i);
            reaction.Keyboard.push_back({topic->getValue<std::string>("name_" + locale)});
        }
    }
    reaction.Keyboard.push_back({"auth.button.back"});
    return {std::move(reaction)};
}

TReactions TTicketTopicChooseStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("ticket_topic_choose_processor");
    logger.information("on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    // user wants to exit
    if (code == "auth.button.back") {
        state.set_state(TUserState_EState_MAIN_MENU);
        return {};
    }

    // user selected the topic (we suppose that he clicked at the menu)
    (*state.mutable_extradata())["topic"] = update.Text;
    state.set_state(TUserState_EState_TICKET_CLUSTER_CHOOSE);
    return {};
}

} // namespace NOctoshell
