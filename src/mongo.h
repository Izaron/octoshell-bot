#pragma once

#include <proto/user_state.pb.h>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include "fwd.h"

namespace NOctoshell {

class TMongo {
public:
    TMongo(TContext& ctx);

    NOctoshell::TUserState Load(std::uint64_t userId);
    void Store(const NOctoshell::TUserState& state);

private:
    TContext& Ctx_;

    mongocxx::instance Instance_;
    mongocxx::client Client_;
    mongocxx::database Database_;
    mongocxx::collection StatesCollection_;
};

} // namespace NOctoshell
