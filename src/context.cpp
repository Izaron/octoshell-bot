#include "context.h"

#include "handlers.h"

#include <map>
#include <inttypes.h>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

namespace NOctoshell {

namespace {

std::unique_ptr<Poco::Net::HTTPServer> ConstructHttpServer(TContext& ctx) {
    std::uint16_t port = 13000;
    return std::make_unique<Poco::Net::HTTPServer>(new THandlerFactory(ctx), port);
}

} // namespace

TContext::TContext()
    : HttpServer_{ConstructHttpServer(*this)}
    , Mongo_{*this}
{
}

void TContext::StartServer() {
    HttpServer_->start();
}

void TContext::StopServer() {
    HttpServer_->stopAll();
}

Poco::Logger& TContext::Logger() const {
    return Poco::Logger::get("default");
}

std::vector<TReaction> TContext::OnUpdate(TUpdate update) {
    // TODO: realize statemachine

    //TUserState state = Mongo_.Load(update.UserId);
    //Mongo_.Store(state);
    
    TReaction reaction;
    reaction.Text = "You wrote: " + update.Text;
    return {reaction};
}

} // namespace NOctoshell
