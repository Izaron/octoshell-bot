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

    try {
        auto stateDoc = StatesCollection_.find_one(document{} << "_id" << static_cast<std::int64_t>(userId) << finalize);
        if (stateDoc) {
            std::string json = bsoncxx::to_json(*stateDoc);
            logger.information("got state for userId %" PRIu64 ": %s", userId, json);
            json2pb(state, json.c_str(), json.length());
        } else {
            logger.information("got no state for userId %" PRIu64, userId);
            state.set_userid(userId);
        }
    } catch (const std::exception& e) {
        logger.error(e.what());
    }

    return state;
}

void TMongo::Store(const TUserState& state) {
    auto& logger = Poco::Logger::get("mongo");

    std::string json = pb2json(state);
    logger.information("storing state %s", json);

    try {
        auto filter = document{} << "_id" << static_cast<std::int64_t>(state.userid()) << finalize;
        auto stateDoc = bsoncxx::from_json(std::move(json));

        StatesCollection_.replace_one(std::move(filter), std::move(stateDoc), mongocxx::options::replace().upsert(true));
    } catch (const std::exception& e) {
        logger.error(e.what());
    }
}

} // namespace NOctoshell
