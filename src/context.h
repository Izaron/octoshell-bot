#pragma once

#include <memory>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Logger.h>

#include "model.h"
#include "mongo.h"

namespace NOctoshell {

class TContext {
public:
    TContext();

    Poco::Logger& Logger() const;

    void StartServer();
    void StopServer();

    [[nodiscard]] std::vector<TReaction> OnUpdate(TUpdate update);

private:
    std::unique_ptr<Poco::Net::HTTPServer> HttpServer_;
    TMongo Mongo_;
};

} // namespace NOctoshell
