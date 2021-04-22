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

        std::vector<Poco::JSON::Object::Ptr> projVector(projArr->size());
        for (size_t i = 0; i < projArr->size(); ++i) {
            projVector[i] = projArr->getObject(i);
        }
        std::sort(projVector.begin(), projVector.end(), [](const Poco::JSON::Object::Ptr& a, const Poco::JSON::Object::Ptr& b) -> bool {
            if (a->has("updated_at") && b->has("updated_at")) {
                return a->getValue<std::string>("updated_at") > b->getValue<std::string>("updated_at");
            } else if (a->has("updated_at")) {
                return true;
            } else if (b->has("updated_at")) {
                return false;
            } else {
                return a->getValue<std::string>("title") > b->getValue<std::string>("title");
            }
        });

        std::stringstream ss;
        ss << "main.projects.header " << projArr->size() << "\n";
        for (size_t i = 0; i < projVector.size(); ++i) {
            auto proj = projVector[i];
            if (proj->has("state")) {
                std::string state = proj->getValue<std::string>("state");
                if (state == "blocked" || state == "finished") {
                    continue;
                }
            }
            ss << "\n";
            ss << "main.projects.number" << i + 1 << "\n";
            ss << "main.projects.login " << "\"" << proj->getValue<std::string>("login") << "\"\n";
            ss << "main.projects.title " << "\"" << proj->getValue<std::string>("title") << "\"\n";
            if (proj->has("state")) {
                std::string state = proj->getValue<std::string>("state");
                ss << "main.projects.state: *" << state << "*\n";
            }
            if (proj->getValue<bool>("owner")) {
                ss << "main.projects.is-owner" << "\n";
            } else {
                ss << "main.projects.is-not-owner" << "\n";
            }
            if (proj->has("updated_at")) {
                ss << "main.projects.updated-at" << ": `" << proj->getValue<std::string>("updated_at") << "`\n";
            }
        }

        reaction.Text = ss.str();
    }

    return reaction;
}

TReaction ConstructUserJobsReaction(TOctoshell& octoshell, const Poco::Util::PropertyFileConfiguration& config, const TUserState& state) {
    const size_t maxJobs = config.getInt("octoshell.max_user_jobs");

    TReaction reaction;

    const std::string response = octoshell.SendQueryWithAuth(state, {
        {"method", "user_jobs"},
        {"limit", std::to_string(maxJobs)},
    });

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        auto jobsArr = object->getArray("jobs");

        std::vector<Poco::JSON::Object::Ptr> jobsVector(jobsArr->size());
        for (size_t i = 0; i < jobsArr->size(); ++i) {
            jobsVector[i] = jobsArr->getObject(i);
        }
        std::sort(jobsVector.begin(), jobsVector.end(), [](const Poco::JSON::Object::Ptr& a, const Poco::JSON::Object::Ptr& b) -> bool {
            return a->getValue<int>("id") > b->getValue<int>("id");
        });

        std::stringstream ss;
        if (object->has("jobs_count")) {
            ss << "main.jobs.header: " << object->getValue<int>("jobs_count") << "\n\n";
        } else {
            ss << "main.jobs.header: " << jobsArr->size() << "\n\n";
        }
        for (size_t i = 0; i < std::min(jobsVector.size(), maxJobs); ++i) {
            auto& job = jobsVector[i];
            if (job->has("id")) {
                ss << "main.jobs.number" << " *" << job->getValue<int>("id") << "*";
                if (job->has("state")) {
                    ss << " (" << "main.jobs.state" << " *" << job->getValue<std::string>("state") << "*)";
                }
                ss << "\n";
            }
            if (job->has("submit_time")) {
                ss << "main.jobs.submitted" << ": `" << job->getValue<std::string>("submit_time") << "`\n";
            }
            if (job->has("state")) {
                const std::string state = job->getValue<std::string>("state");
                if (state != "PENDING") {
                    if (job->has("start_time")) {
                        ss << "main.jobs.started" << ": `" << job->getValue<std::string>("start_time") << "`\n";
                    }
                    if (job->has("end_time")) {
                        ss << "main.jobs.ended" << ": `" << job->getValue<std::string>("end_time") << "`\n";
                    }
                }
            }
            if (job->has("get_duration_hours")) {
                ss << "main.jobs.duration-hours" << ": " << job->getValue<double>("get_duration_hours") << "\n";
            }
            if (job->has("num_cores") && job->has("num_nodes")) {
                ss << "main.jobs.num-nodes" << ": " << job->getValue<int>("num_nodes") << ", ";
                ss << "main.jobs.num-cores" << ": " << job->getValue<int>("num_cores") << "\n";
            }
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
            if (job->has("command")) {
                ss << "\n" << "main.jobs.command" << ": `" << job->getValue<std::string>("command") << "`\n";
            }
            ss << "\n\n";
        }

        reaction.Text = ss.str();
    }

    return reaction;
}

TReaction ConstructTicketsReaction(TOctoshell& octoshell, const Poco::Util::PropertyFileConfiguration& config, const TUserState& state) {
    const size_t maxTickets = config.getInt("octoshell.max_user_tickets");

    auto& logger = Poco::Logger::get("main_menu_processor");

    TReaction reaction;
    const std::string response = octoshell.SendQueryWithAuth(state, {
        {"method", "user_tickets"},
        {"limit", std::to_string(maxTickets)},
    });

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    std::string status = object->getValue<std::string>("status");
    if (status == "fail") {
        reaction.Text = "main.fail-auth";
    } else {
        auto ticketsArr = object->getArray("tickets");

        int ticketsCount = 0;
        std::stringstream ss;
        if (object->has("tickets_count")) {
            ss << "main.tickets.header" << " " << object->getValue<int>("tickets_count") << "\n";
        } else {
            ss << "main.tickets.header" << " " << ticketsArr->size() << "\n";
        }
        for (size_t i = 0; i < std::min(ticketsArr->size(), 2 * maxTickets); ++i) {
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
                if (ticket->has("updated_at")) {
                    ss << "main.tickets.updated-at" << ": `" << ticket->getValue<std::string>("updated_at") << "`\n";
                }

                ++ticketsCount;
            } catch (...) {
                logger.warning("skip ticket %d", i);
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
        return {ConstructUserJobsReaction(Ctx_.Octoshell(), Ctx_.Config(), state)};
    }

    if (code == "main.button.show-tickets") {
        return {ConstructTicketsReaction(Ctx_.Octoshell(), Ctx_.Config(), state)};
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
