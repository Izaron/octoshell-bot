#include "ticket_project_choose_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>
#include <regex>

namespace NOctoshell {

TReactions TTicketProjectChooseStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("ticket_project_choose_processor").information("on_start from user %" PRIu64, update.UserId);
    TReaction reaction;

    const std::string response = Ctx_.Octoshell().SendQueryWithAuth(state, {{"method", "user_projects"}});

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        reaction.Text = "main.tickets.projects.message";
        auto projArr = object->getArray("projects");
        for (size_t i = 0; i < projArr->size(); ++i) {
            auto proj = projArr->getObject(i);
            reaction.Keyboard.push_back({proj->getValue<std::string>("title")});
        }
    }
    reaction.Keyboard.push_back({"auth.button.back"});
    return {std::move(reaction)};
}

TReactions TTicketProjectChooseStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("ticket_project_choose_processor");
    logger.information("on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    // user wants to exit
    if (code == "auth.button.back") {
        state.set_state(TUserState_EState_MAIN_MENU);
        return {};
    }

    // user selected the project (we suppose that he clicked at the menu)
    (*state.mutable_extradata())["project"] = update.Text;
    state.set_state(TUserState_EState_TICKET_TOPIC_CHOOSE);
    return {};
}

} // namespace NOctoshell
