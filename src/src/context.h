#pragma once

#include <memory>

#include <Poco/Logger.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Util/PropertyFileConfiguration.h>

#include "model.h"
#include "mongo.h"
#include "octoshell.h"
#include "states/states_holder.h"
#include "translate.h"

namespace NOctoshell {

class TContext {
public:
    TContext(const std::string& configPath);

    void StartServer();
    void StopServer();

    const Poco::Util::PropertyFileConfiguration& Config() const;
    const TTranslate& Translate() const;
    TMongo& Mongo();
    TOctoshell& Octoshell();

    [[nodiscard]] TReactions OnUpdate(TUpdate update, const TUserState_ESource source);

private:
    Poco::Logger& Logger() const;

private:
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> Config_;
    std::unique_ptr<Poco::Net::HTTPServer> HttpServer_;
    TTranslate Translate_;
    TStatesHolder StatesHolder_;
    TMongo Mongo_;
    TOctoshell Octoshell_;
};

} // namespace NOctoshell
