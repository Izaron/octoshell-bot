#pragma once

#include <string>

#include <Poco/JSON/Object.h>

#include "../model.h"

namespace NOctoshell {

class IClient {
public:
    virtual ~IClient() = default;

    virtual std::string Name() const = 0;
    virtual TUpdate ParseUpdate(const Poco::JSON::Object& data) const = 0;
    virtual void SendReaction(const TReaction& reaction) const = 0;
};

} // namespace NOctoshell
