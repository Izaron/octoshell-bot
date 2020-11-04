#include "states_holder.h"
#include "state_processor.h"

#include <inttypes.h>
#include <unordered_map>

#include <Poco/Logger.h>

namespace NOctoshell {

namespace {

using TProcessorsMap = std::unordered_map<int, std::unique_ptr<IStatesProcessor>>;

TProcessorsMap ConstructStatesProcessors() {
    return {};
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

std::vector<TReaction> TStatesHolder::ProcessUpdate(TUpdate update, TUserState& state) {
    const static auto processorsMap = ConstructStatesProcessors();

    auto& logger = Poco::Logger::get("states_holder");
    logger.information("processing update from user %" PRIu64, update.UserId);

    std::vector<TReaction> reactions;
    for (const auto func : {&IStatesProcessor::OnStart, &IStatesProcessor::OnUpdate}) {
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
