#include "handlers.h"

#include "client/client.h"
#include "client/telegram.h"

#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>

#include <inttypes.h>

using namespace Poco::Net;

namespace NOctoshell {

namespace {

std::string ReadInput(HTTPServerRequest& request) {
    std::istream& is = request.stream();

    char c;
    std::string str;
    while (is.get(c)) {
        str += c;
    }

    return str;
}

template<typename TClient, std::enable_if_t<std::is_base_of<IClient, TClient>::value, int> = 0>
class TClientHandler final : public HTTPRequestHandler {
public:
    TClientHandler(TContext& ctx)
        : Ctx_{ctx}
        , Client_{new TClient(ctx)}
    {
    }

    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        auto& logger = Poco::Logger::get("client_handler");

        logger.information("Client query from %s", Client_->Name());

        // read full stream
        std::string in = ReadInput(request);
        logger.information("Body: %s", in);

        // send default 200 HTTP OK
        response.send();

        try {
            // parse Json from HTTP
            Poco::JSON::Parser parser;
            auto result = parser.parse(in);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            // parse Update from Json
            TUpdate update = Client_->ParseUpdate(*object);
            logger.information("new update: Text \"%s\", UserId \"%" PRIu64 "\"", update.Text, update.UserId);

            // send Reactions from Update
            auto reactions = Ctx_.OnUpdate(std::move(update));
            for (const auto& reaction : reactions) {
                Client_->SendReaction(update, reaction);
            }
        } catch (Poco::JSON::JSONException& e) {
            logger.error(e.displayText());
        }
    }

private:
    TContext& Ctx_;
    std::unique_ptr<IClient> Client_;
};

class TDummyHandler final : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        response.setStatus(HTTPServerResponse::HTTP_IM_A_TEAPOT);
        response.setContentType("text/plain");
        response.send() << "I Am A Teapot";
    }
};

class TPingHandler final : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        response.setStatus(HTTPServerResponse::HTTP_OK);
        response.setContentType("text/plain");
        response.send() << "pong";
    }
};

} // namespace

THandlerFactory::THandlerFactory(TContext& ctx)
    : Ctx_{ctx}
{
}

HTTPRequestHandler* THandlerFactory::createRequestHandler(const HTTPServerRequest& request) {
    auto& logger = Poco::Logger::get("handlers");

    const auto& uri = request.getURI();
    logger.information("Got request at URI %s", uri);

    if (uri == "/ping") {
        return new TPingHandler();
    }

    if (uri == "/api/telegram") {
        return new TClientHandler<TTelegramClient>(Ctx_);
    }

    return new TDummyHandler();
}

} // namespace NOctoshell
