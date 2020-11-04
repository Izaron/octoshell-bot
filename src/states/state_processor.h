#pragma once

#include <unordered_map>
#include <vector>

#include <proto/user_state.pb.h>
#include "../model.h"
#include "../fwd.h"

namespace NOctoshell {

class IStatesProcessor {
public:
    IStatesProcessor(TContext& ctx)
        : Ctx_{ctx}
    {};

    virtual std::vector<TReaction> OnStart(TUpdate update, TUserState& state) = 0;
    virtual std::vector<TReaction> OnUpdate(TUpdate update, TUserState& state) = 0;

protected:
    TContext& Ctx_;
};

} // namespace NOctoshell
