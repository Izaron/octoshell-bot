#pragma once

#include "client.h"

namespace NOctoshell {

class TVkontakteClient final : public IClient {
public:
    using IClient::IClient;

    TUpdate ParseUpdate(const Poco::JSON::Object& data) const override;
    void SendReaction(const TUpdate& update, const TReaction& reaction) const override;
};

} // namespace NOctoshell
