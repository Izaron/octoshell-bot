#include "mongo.h"
#include "context.h"

#include <iostream>
#include <inttypes.h>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <Poco/JSON/Parser.h>

#include "../contrib/json2pb.h"

using namespace bsoncxx::builder::stream;

namespace NOctoshell {

TMongo::TMongo(TContext& ctx)
    : Ctx_{ctx}
    , Instance_{}
    , Client_{mongocxx::uri{ctx.Config().getString("mongodb.url")}}
    , Database_{Client_["default"]}
    , StatesCollection_{Database_.collection("states")}
{
}

TUserState TMongo::Load(std::uint64_t userId) {
    auto& logger = Poco::Logger::get("mongo");

    TUserState state;

    logger.information("getting state for userId %" PRIu64, userId);

    auto stateDoc = StatesCollection_.find_one(document{} << "UserId" << static_cast<std::int64_t>(userId) << finalize);
    if (stateDoc) {
        std::string json = bsoncxx::to_json(*stateDoc);
        logger.information("got state for userId %" PRIu64 ": %s", userId, json);
        json2pb(state, json.c_str(), json.length());
    } else {
        logger.information("got no state for userId %" PRIu64, userId);
        state.set_userid(userId);
    }

    return state;
}

void TMongo::Store(const NOctoshell::TUserState& state) {
    std::string json = pb2json(state);
    auto stateDoc = bsoncxx::from_json(std::move(json));
    StatesCollection_.insert_one(std::move(stateDoc));
}

} // namespace NOctoshell
