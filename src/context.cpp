#include "context.h"

#include "handlers.h"

#include <map>
#include <sstream>
#include <inttypes.h>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <Poco/Logger.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>

#include <Poco/URI.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPResponse.h>

using namespace Poco;
using namespace Poco::Net;

namespace NOctoshell {

namespace {

std::unique_ptr<Net::HTTPServer> ConstructHttpServer(TContext& ctx) {
    std::uint16_t port = ctx.Config().getInt("port");
    return std::make_unique<Net::HTTPServer>(new THandlerFactory(ctx), port);
}

void InitLogs(const Util::PropertyFileConfiguration& config) {
    AutoPtr<ConsoleChannel> channel;

    if (config.has("logpath")) {
        AutoPtr<SimpleFileChannel> channel(new SimpleFileChannel);
        channel->setProperty("path", config.getString("logpath"));
    } else {
        channel = new ConsoleChannel;
    }

    AutoPtr<PatternFormatter> patternFormatter(new PatternFormatter);
    patternFormatter->setProperty("pattern", "[%Y/%b/%d %H:%M:%S] [thread %I] [%p] [%s] %t");

    AutoPtr<FormattingChannel> formattingChannel(new FormattingChannel(patternFormatter, channel));
    Logger::root().setChannel(formattingChannel.get());
}

void SetTelegramWebhook(const Util::PropertyFileConfiguration& config) {
    auto& logger = Poco::Logger::get("telegram");
    logger.information("setting telegram webhook to %s", config.getString("url"));

    std::stringstream ss;
    ss << "https://api.telegram.org/bot" << config.getString("telegram.token");
    ss << "/setWebhook?url=" << config.getString("url");
    if (config.getString("url").back() != '/') {
        ss << "/";
    }
    ss << "api/telegram";

    URI uri{ss.str()};

    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }

    HTTPSClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    session.sendRequest(request);

    HTTPResponse response;
    std::istream& rs = session.receiveResponse(response);

    std::stringstream responseStream;
    responseStream << rs.rdbuf();
    logger.information("https response: code %d, reason %s, body %s", static_cast<int>(response.getStatus()), response.getReason(), responseStream.str());
}

} // namespace


TContext::TContext(const std::string& configPath)
    : Config_{new Util::PropertyFileConfiguration(configPath)}
    , HttpServer_{ConstructHttpServer(*this)}
    , StatesHolder_{*this}
    , Mongo_{*this}
{
    InitLogs(*Config_);
    SetTelegramWebhook(*Config_);
}

void TContext::StartServer() {
    HttpServer_->start();
    Logger().information("Server started");
}

void TContext::StopServer() {
    HttpServer_->stopAll();
    Logger().information("Server stopped");
}

const Util::PropertyFileConfiguration& TContext::Config() const {
    return *Config_;
}

Logger& TContext::Logger() const {
    return Logger::get("context");
}

std::vector<TReaction> TContext::OnUpdate(TUpdate update) {
    // TODO: realize statemachine
    Logger().information("working with update from %" PRIu64, update.UserId);

    TUserState state = Mongo_.Load(update.UserId);
    std::vector<TReaction> reactions = StatesHolder_.ProcessUpdate(update, state);

    Mongo_.Store(state);
    return reactions;
}

} // namespace NOctoshell
