#pragma once

#include <vector>

#include <proto/user_state.pb.h>
#include "../model.h"
#include "../fwd.h"

namespace NOctoshell {

class IStatesProcessor {
public:
    virtual ~IStatesProcessor() = default;
    IStatesProcessor(TContext& ctx) : Ctx_{ctx} {};

    virtual TReactions OnStart(TUpdate update, TUserState& state) = 0;
    virtual TReactions OnUpdate(TUpdate update, TUserState& state) = 0;

protected:
    TContext& Ctx_;
};

} // namespace NOctoshell
