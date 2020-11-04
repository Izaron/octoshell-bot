#pragma once

#include "state_processor.h"

namespace NOctoshell {

class TAuthNewTokenStatesProcessor final : public IStatesProcessor {
public:
    using IStatesProcessor::IStatesProcessor;

    TReactions OnStart(TUpdate update, TUserState& state) override;
    TReactions OnUpdate(TUpdate update, TUserState& state) override;
};

} // namespace NOctoshell
