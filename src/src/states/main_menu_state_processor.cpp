#include "main_menu_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>
#include <sstream>

namespace NOctoshell {

namespace {

TReaction ConstructInformationReaction() {
    TReaction reaction;
    reaction.Text = "main.information";
    return reaction;
}

TReaction ConstructUserProjectsReaction(TOctoshell& octoshell, const TUserState& state) {
    TReaction reaction;

    const std::string response = octoshell.SendQueryWithAuth(state, {{"method", "user_projects"}});

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        auto projArr = object->getArray("projects");

        std::stringstream ss;
        ss << "main.projects.header " << projArr->size() << "\n";
        for (size_t i = 0; i < projArr->size(); ++i) {
            auto proj = projArr->getObject(i);
            ss << "\n";
            ss << "main.projects.number" << i + 1 << "\n";
            ss << "main.projects.login " << "\"" << proj->getValue<std::string>("login") << "\"\n";
            ss << "main.projects.title " << "\"" << proj->getValue<std::string>("title") << "\"\n";
            if (proj->getValue<bool>("owner")) {
                ss << "main.projects.is-owner" << "\n";
            } else {
                ss << "main.projects.is-not-owner" << "\n";
            }
        }

        reaction.Text = ss.str();
    }

    return reaction;
}

} // namespace

TReactions TMainMenuStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("main_menu_processor").information("on_start from user %" PRIu64, update.UserId);

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
    logger.information("on_update from user %" PRIu64, update.UserId);

    const std::string code = TryGetTemplate(update.Text, state.language(), Ctx_.Translate());
    logger.information("Pressed button is \"%s\"", code);

    if (code == "main.button.to-auth-settings") {
        state.set_state(TUserState_EState_AUTH_SETTINGS);
        return {};
    }

    if (code == "main.button.show-user-projects") {
        return {ConstructUserProjectsReaction(Ctx_.Octoshell(), state)};
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
