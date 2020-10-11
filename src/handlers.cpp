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
        , Client_{new TClient}
    {
    }

    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        Ctx_.Logger().information("Client query from %s", Client_->Name());

        // read full stream
        std::string in = ReadInput(request);
        Ctx_.Logger().information("Body: %s", in);

        // send default 200 HTTP OK
        response.send();

        try {
            // parse Json from HTTP
            Poco::JSON::Parser parser;
            auto result = parser.parse(in);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            // parse Update from Json
            TUpdate update = Client_->ParseUpdate(*object);
            Ctx_.Logger().information("Text \"%s\", UserId \"%" PRIu64 "\"", update.Text, update.UserId);

            // send Reactions from Update
            auto reactions = Ctx_.OnUpdate(std::move(update));
            for (const auto& reaction : reactions) {
                Client_->SendReaction(reaction);
            }
        } catch (Poco::JSON::JSONException& e) {
            Ctx_.Logger().error(e.displayText());
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

} // namespace

THandlerFactory::THandlerFactory(TContext& ctx)
    : Ctx_{ctx}
{
}

HTTPRequestHandler* THandlerFactory::createRequestHandler(const HTTPServerRequest& request) {
    const auto& uri = request.getURI();
    Ctx_.Logger().information("Got request at URI %s", uri);

    if (uri == "/api/telegram") {
        return new TClientHandler<TTelegramClient>(Ctx_);
    }

    return new TDummyHandler();
}

} // namespace NOctoshell
