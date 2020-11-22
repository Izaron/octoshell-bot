#include "states_holder.h"
#include "state_processor.h"

#include "auth_settings_state_processor.h"
#include "auth_new_email_state_processor.h"
#include "auth_new_token_state_processor.h"
#include "locale_settings_state_processor.h"
#include "main_menu_state_processor.h"

#include <inttypes.h>
#include <unordered_map>

#include <Poco/Logger.h>

namespace NOctoshell {

namespace {

using TProcessorsMap = std::unordered_map<int, std::unique_ptr<IStatesProcessor>>;

TProcessorsMap ConstructStatesProcessors(TContext& ctx) {
    TProcessorsMap map;

    map.emplace(TUserState_EState_MAIN_MENU, std::make_unique<TMainMenuStatesProcessor>(ctx));
    map.emplace(TUserState_EState_LOCALE_SETTINGS, std::make_unique<TLocaleSettingsStatesProcessor>(ctx));
    map.emplace(TUserState_EState_AUTH_SETTINGS, std::make_unique<TAuthSettingsStatesProcessor>(ctx));
    map.emplace(TUserState_EState_AUTH_NEW_EMAIL, std::make_unique<TAuthNewEmailStatesProcessor>(ctx));
    map.emplace(TUserState_EState_AUTH_NEW_TOKEN, std::make_unique<TAuthNewTokenStatesProcessor>(ctx));

    return map;
}

IStatesProcessor* FindStateProcessor(TUserState& state, const TProcessorsMap& map) {
    const auto iter = map.find(state.state());
    if (iter == map.end()) {
        return nullptr;
    }
    return iter->second.get();
}

} // namespace

TStatesHolder::TStatesHolder(TContext& ctx)
    : Ctx_{ctx}
{
}

TReactions TStatesHolder::ProcessUpdate(TUpdate update, TUserState& state) {
    const static auto processorsMap = ConstructStatesProcessors(Ctx_);

    auto& logger = Poco::Logger::get("states_holder");
    logger.information("processing update from user %" PRIu64, update.UserId);

    TReactions reactions;
    for (const auto func : {&IStatesProcessor::OnUpdate, &IStatesProcessor::OnStart}) {
        if (const auto processor = FindStateProcessor(state, processorsMap)) {
            auto r = (processor->*func)(update, state);
            std::move(r.begin(), r.end(), std::inserter(reactions, reactions.end()));
        } else {
            logger.error("unimplemented state %d!", static_cast<int>(state.state()));
        }
    }

    return reactions;
}

} // namespace NOctoshell
