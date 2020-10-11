#pragma once

#include "client.h"

namespace NOctoshell {

class TTelegramClient final : public IClient {
public:
    std::string Name() const override;
    TUpdate ParseUpdate(const Poco::JSON::Object& data) const override;
    void SendReaction(const TReaction& reaction) const override;
};

} // namespace NOctoshell
