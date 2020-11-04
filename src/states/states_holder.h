#pragma once

#include <vector>

#include <proto/user_state.pb.h>
#include "../model.h"
#include "../fwd.h"

namespace NOctoshell {

class TStatesHolder {
public:
    TStatesHolder(TContext& ctx);
    std::vector<TReaction> ProcessUpdate(TUpdate update, TUserState& state);

private:
    TContext& Ctx_;
};

} // namespace NOctoshell
