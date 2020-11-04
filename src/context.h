#pragma once

#include <memory>

#include <Poco/Logger.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Util/PropertyFileConfiguration.h>

#include "model.h"
#include "mongo.h"
#include "states/states_holder.h"

namespace NOctoshell {

class TContext {
public:
    TContext(const std::string& configPath);

    void StartServer();
    void StopServer();

    const Poco::Util::PropertyFileConfiguration& Config() const;

    [[nodiscard]] TReactions OnUpdate(TUpdate update);

private:
    Poco::Logger& Logger() const;

private:
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> Config_;
    std::unique_ptr<Poco::Net::HTTPServer> HttpServer_;
    TStatesHolder StatesHolder_;
    TMongo Mongo_;
};

} // namespace NOctoshell
