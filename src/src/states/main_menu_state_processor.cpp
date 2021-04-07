#include "main_menu_state_processor.h"
#include "../translate.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>
#include <sstream>

namespace NOctoshell {

namespace {

std::string GetStringSafe(Poco::JSON::Object::Ptr json, const std::string& key, const std::string defaultValue = "<empty>") {
    try {
        return json->getValue<std::string>(key);
    } catch (...) {
        return defaultValue;
    }
}

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

TReaction ConstructUserJobsReaction(TOctoshell& octoshell, const TUserState& state) {
    constexpr size_t MAXIMAL_JOBS = 5;

    TReaction reaction;

    const std::string response = octoshell.SendQueryWithAuth(state, {{"method", "user_jobs"}});

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        auto jobsArr = object->getArray("jobs");

        std::stringstream ss;
        ss << "main.jobs.header: " << jobsArr->size() << "\n\n";
        for (size_t i = 0; i < std::min(jobsArr->size(), MAXIMAL_JOBS); ++i) {
            auto job = jobsArr->getObject(i)->getObject("table");
            ss << "main.jobs.number" << i + 1 << "\n";
            ss << "main.jobs.state" << ": " << job->getValue<std::string>("state") << "\n";
            ss << "main.jobs.id" << ": " << job->getValue<int>("id") << "\n";
            ss << "main.jobs.num-cores" << ": " << job->getValue<int>("num_cores") << "\n";
            ss << "main.jobs.num-nodes" << ": " << job->getValue<int>("num_nodes") << "\n";
            ss << "main.jobs.duration-hours" << ": " << job->getValue<double>("get_duration_hours") << "\n";
            if (job->has("rules")) {
                std::stringstream rulesSs;
                const auto rules = job->getObject("rules");
                for (const auto& pair : *rules) {
                    rulesSs << pair.second.extract<std::string>() << "; ";
                }

                const std::string rulesStr = rulesSs.str();
                if (!rulesStr.empty()) {
                    ss << "main.jobs.rules" << ": " << rulesStr;
                }
            }
            ss << "\n\n";
        }

        reaction.Text = ss.str();
    }

    return reaction;
}

TReaction ConstructTicketsReaction(TOctoshell& octoshell, const TUserState& state) {
    auto& logger = Poco::Logger::get("main_menu_processor");

    TReaction reaction;
    const std::string response = octoshell.SendQueryWithAuth(state, {{"method", "user_tickets"}});

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        auto ticketsArr = object->getArray("tickets");

        std::stringstream ss;
        int ticketsCount = 0;

        for (size_t i = 0; i < ticketsArr->size(); ++i) {
            try {
                auto ticket = ticketsArr->getObject(i);

                const std::string locale = state.language() == TUserState_ELanguage_EN ? "en" : "ru";
                const std::string who = "main.tickets.who-is-" + ticket->getValue<std::string>("who");
                const std::string topicName = GetStringSafe(ticket, "topic_name_" + locale);
                const std::string projectTitle = GetStringSafe(ticket, "project_title");
                const std::string clusterName = GetStringSafe(ticket, "cluster_name_" + locale);
                const std::string subject = ticket->getValue<std::string>("subject");
                const std::string message = ticket->getValue<std::string>("message");
                const std::string state = ticket->getValue<std::string>("state");

                ss << "\n";
                ss << "main.tickets.number" << ticketsCount + 1 << "\n";
                ss << "main.tickets.who-status" << ": " << who << "\n";
                ss << "main.tickets.topic-title" << ": " << topicName << "\n";
                ss << "main.tickets.project-title" << ": " << projectTitle << "\n";
                ss << "main.tickets.cluster-title" << ": " << clusterName << "\n";
                ss << "main.tickets.subject-title" << ": " << subject << "\n";
                ss << "main.tickets.state-title" << ": " << "main.tickets.state." << state << "\n";
                ss << "main.tickets.desc" << ": " << message << "\n";

                ++ticketsCount;
            } catch (...) {
                logger.warning("skip ticket %d", i);
            }
        }

        std::stringstream finalSs;
        finalSs << "main.tickets.header" << " " << ticketsCount << "\n" << ss.str();
        reaction.Text = finalSs.str();
    }

    return reaction;
}

} // namespace

TReactions TMainMenuStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    Poco::Logger::get("main_menu_processor").information("on_start from user %" PRIu64, update.UserId);

    const static TReaction::TKeyboard keyboardTemplate = {
        {"main.button.show-user-projects", "main.button.show-user-jobs"},
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

    if (code == "main.button.show-user-jobs") {
        return {ConstructUserJobsReaction(Ctx_.Octoshell(), state)};
    }

    if (code == "main.button.show-tickets") {
        return {ConstructTicketsReaction(Ctx_.Octoshell(), state)};
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
