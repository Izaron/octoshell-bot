#include "auth_status.h"

#include <Poco/Logger.h>
#include <Poco/JSON/Parser.h>

namespace NOctoshell {

EAuthStatus TryAuth(TOctoshell& octoshell, const TUserState& state) {
    auto& logger = Poco::Logger::get("try_auth");

    std::string response = octoshell.SendQueryWithAuth(state, {{"method", "auth"}});

    Poco::JSON::Parser parser;
    auto result = parser.parse(response);
    auto object = result.extract<Poco::JSON::Object::Ptr>();

    if (object->has("status")) {
        return static_cast<EAuthStatus>(object->getValue<int>("status"));
    }

    logger.error("invalid response from server!");
    return EAuthStatus::SERVICE_UNAVAILABLE;
}

} // namespace NOctoshell
