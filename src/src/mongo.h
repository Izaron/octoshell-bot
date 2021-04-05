#pragma once

#include <proto/user_state.pb.h>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "fwd.h"

namespace NOctoshell {

class TMongo {
public:
    TMongo(TContext& ctx);

    TUserState Load(std::uint64_t userId, const TUserState_ESource source);
    std::vector<TUserState> LoadByAuth(const std::string& email, const std::string& token);
    void Store(const TUserState& state);

private:
    TContext& Ctx_;

    mongocxx::instance Instance_;
    mongocxx::client Client_;
    mongocxx::database Database_;
    mongocxx::collection StatesCollection_;
};

} // namespace NOctoshell
