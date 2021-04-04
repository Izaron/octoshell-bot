#pragma once

#include <string>
#include <unordered_map>

#include <proto/user_state.pb.h>
#include "fwd.h"

namespace NOctoshell {

class TOctoshell {
public:
    TOctoshell(TContext& ctx) : Ctx_{ctx} {};

    std::string SendQueryWithAuth(const TUserState& state, const std::unordered_map<std::string, std::string>& params);
    std::string SendQuery(const std::unordered_map<std::string, std::string>& params);

private:
    TContext& Ctx_;
};

} // namespace NOctoshell
