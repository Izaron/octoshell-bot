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

TUserState TMongo::Load(std::uint64_t userId, const TUserState_ESource source) {
    auto& logger = Poco::Logger::get("mongo");
    logger.information("getting state for userId %" PRIu64, userId);

    try {
        mongocxx::cursor cursor = StatesCollection_.find(document{} << "_id" << static_cast<std::int64_t>(userId) << finalize);
        for (auto doc : cursor) {
            std::string json = bsoncxx::to_json(doc);

            TUserState state;
            json2pb(state, json.c_str(), json.length());

            if (state.source() == source) {
                logger.information("got state for userId %" PRIu64 ": %s", userId, json);
                return state;
            }
        }
    } catch (const std::exception& e) {
        logger.error(e.what());
    }

    logger.information("got no state for userId %" PRIu64, userId);

    TUserState state;
    state.set_userid(userId);
    return state;
}

std::vector<TUserState> TMongo::LoadByAuth(const std::string& email, const std::string& token) {
    auto& logger = Poco::Logger::get("mongo");
    logger.information("getting all states for email %s, token %s", email, token);

    std::vector<TUserState> states;
    try {
        mongocxx::cursor cursor = StatesCollection_.find(document{} << "Email" << email << "Token" << token << finalize);
        for (auto doc : cursor) {
            std::string json = bsoncxx::to_json(doc);

            TUserState state;
            json2pb(state, json.c_str(), json.length());
            states.push_back(std::move(state));
        }
    } catch (const std::exception& e) {
        logger.error(e.what());
    }

    logger.information("got %d states for email %s, token %s", static_cast<int>(states.size()), email, token);
    return states;
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
