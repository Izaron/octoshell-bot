#include "context.h"

#include "handlers.h"

#include <inttypes.h>
#include <map>
#include <queue>
#include <sstream>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <Poco/Logger.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>

#include <Poco/Environment.h>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/URI.h>

using namespace Poco;
using namespace Poco::Net;

namespace NOctoshell {

namespace {

std::unique_ptr<Net::HTTPServer> ConstructHttpServer(TContext& ctx) {
    std::uint16_t port = ctx.Config().getInt("port");
    return std::make_unique<Net::HTTPServer>(new THandlerFactory(ctx), port);
}

Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> ConstructConfig(const std::string& configPath) {
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> config = new Util::PropertyFileConfiguration(configPath);

    std::vector<std::string> keys;
    std::queue<std::string> q;
    q.push("");
    while (!q.empty()) {
        std::string prefix = q.front();
        q.pop();

        std::vector<std::string> child;
        config->keys(prefix, child);

        if (child.empty()) {
            keys.push_back(std::move(prefix));
        } else {
            for (auto&& v : child) {
                if (!prefix.empty()) {
                    q.push(prefix + "." + std::move(v));
                } else {
                    q.push(std::move(v));
                }
            }
        }
    }

    for (const auto& key : keys) {
        std::string envVar = "OCTOBOT_" + translate(toUpper(key), ".", "_");
        if (Poco::Environment::has(envVar)) {
            config->setString(key, Poco::Environment::get(envVar));
        }
    }
    return config;
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
    patternFormatter->setProperty("pattern", "[%Y/%b/%d %H:%M:%S.%i] [thread %I] [%p] [%s] %t");

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
    : Config_{ConstructConfig(configPath)}
    , HttpServer_{ConstructHttpServer(*this)}
    , StatesHolder_{*this}
    , Mongo_{*this}
    , Octoshell_{*this}
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

const TTranslate& TContext::Translate() const {
    return Translate_;
}

TMongo& TContext::Mongo() {
    return Mongo_;
}

TOctoshell& TContext::Octoshell() {
    return Octoshell_;
}

Logger& TContext::Logger() const {
    return Logger::get("context");
}

TReactions TContext::OnUpdate(TUpdate update, const TUserState_ESource source) {
    auto lang = TUserState_ELanguage_EN;
    try {
        Logger().information("working with update from %" PRIu64, update.UserId);

        TUserState state = Mongo_.Load(update.UserId, source);
        TReactions reactions = StatesHolder_.ProcessUpdate(update, state);
        lang = state.language();
        for (auto& r : reactions) {
            TranslateReaction(r, lang, Translate_);
        }

        state.set_source(source);
        Mongo_.Store(state);
        return reactions;
    } catch (const std::exception& e) {
        Logger().error(e.what());

        TReaction reaction;
        reaction.Text = "unavailable";
        TranslateReaction(reaction, lang, Translate_);
        return {std::move(reaction)};
    }
}

} // namespace NOctoshell
