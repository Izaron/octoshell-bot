#pragma once

#include "client.h"

namespace NOctoshell {

class TTelegramClient final : public IClient {
public:
    using IClient::IClient;

    std::string Name() const override;
    TUpdate ParseUpdate(const Poco::JSON::Object& data) const override;
    void SendReaction(const TUpdate& update, const TReaction& reaction) const override;
};

} // namespace NOctoshell
