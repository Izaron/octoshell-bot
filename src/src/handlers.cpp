#include "handlers.h"

#include "client/client.h"
#include "client/telegram.h"
#include "client/vkontakte.h"
#include <proto/user_state.pb.h>

#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Parser.h>

#include <exception>
#include <inttypes.h>
#include <sstream>

using namespace Poco::Net;

namespace NOctoshell {

namespace {

std::unique_ptr<IClient> ConstructClient(const TUserState_ESource source, TContext& ctx) {
    if (source == TUserState_ESource_TELEGRAM) {
        return std::make_unique<TTelegramClient>(ctx);
    } else if (source == TUserState_ESource_VKONTAKTE) {
        return std::make_unique<TVkontakteClient>(ctx);
    } else {
        throw std::runtime_error("unknown client source");
    }
}

std::string ReadInput(HTTPServerRequest& request) {
    std::istream& is = request.stream();

    char c;
    std::string str;
    while (is.get(c)) {
        str += c;
    }

    return str;
}

void ConfirmVkontakte(const Poco::JSON::Object& data, const TContext& ctx, HTTPServerRequest& request, HTTPServerResponse& response) {
    auto& logger = Poco::Logger::get("vkontakte_confirm");

    if (!data.has("secret") || data.getValue<std::string>("secret") != ctx.Config().getString("vk.secret_code")) {
        logger.warning("wrong or absent \"secret\" in VK confirm request");
        return;
    }

    if (!data.has("group_id") || data.getValue<std::string>("group_id") != ctx.Config().getString("vk.group_id")) {
        logger.warning("wrong or absent \"group_id\" in VK confirm request");
        return;
    }
    logger.information("successfully confirmed VK server");

    response.setStatus(HTTPServerResponse::HTTP_OK);
    response.setContentType("text/plain");
    response.send() << ctx.Config().getString("vk.confirmation_code");
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

        logger.information("Client query from %s", request.getURI());

        // read full stream
        std::string in = ReadInput(request);
        logger.information("Body: %s", in);

        try {
            // parse Json from HTTP
            Poco::JSON::Parser parser;
            auto result = parser.parse(in);
            auto object = result.extract<Poco::JSON::Object::Ptr>();

            // corner case - VKontakte identification
            if (object->has("type") && object->getValue<std::string>("type") == "confirmation") {
                ConfirmVkontakte(*object, Ctx_, request, response);
                return;
            }

            // send default 200 HTTP OK
            response.send() << "ok";

            // parse Update from Json
            TUpdate update = Client_->ParseUpdate(*object);
            logger.information("new update: Text \"%s\", UserId \"%" PRIu64 "\"", update.Text, update.UserId);

            // send Reactions from Update
            auto reactions = Ctx_.OnUpdate(std::move(update), Client_->Source());
            for (const auto& reaction : reactions) {
                Client_->SendReaction(update, reaction);
            }
        } catch (const Poco::JSON::JSONException& e) {
            logger.error(e.displayText());
        } catch (const std::exception& e) {
            logger.error(e.what());
        }
    }

private:
    TContext& Ctx_;
    std::unique_ptr<IClient> Client_;
};

class TDummyHandler final : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        auto& logger = Poco::Logger::get("dummy_handler");
        logger.information("Request input: %s", ReadInput(request));

        response.setStatus(HTTPServerResponse::HTTP_IM_A_TEAPOT);
        response.setContentType("text/plain");
        response.send() << "I Am A Teapot";
    }
};

class TNotifyHandler final : public HTTPRequestHandler {
public:
    TNotifyHandler(TContext& ctx)
        : Ctx_{ctx}
    {}

    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        auto& logger = Poco::Logger::get("notify_handler");

        const std::string in = ReadInput(request);
        logger.information("Notify body: %s", in);

        // immediate answer
        response.setStatus(HTTPServerResponse::HTTP_OK);
        response.setContentType("text/plain");
        response.send() << "ok";

        // working with input
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(in);

        const std::string& uri = request.getURI();
        if (uri == "/notify/ticket") {
            OnNotifyTicket(std::move(result));
        } else if (uri == "/notify/announcement") {
            OnNotifyAnnouncement(std::move(result));
        } else {
            logger.warning("Unknown uri!");
        }
    }

private:
    void OnNotifyTicket(Poco::Dynamic::Var result) {
        auto objectsArr = result.extract<Poco::JSON::Array::Ptr>();
        for (size_t i = 0; i < objectsArr->size(); ++i) {
            const auto object = objectsArr->getObject(i);
            if (!object->has("token")) {
                return;
            }

            const std::string email = object->getValue<std::string>("email");
            const std::string token = object->getValue<std::string>("token");
            const std::string event = object->getValue<std::string>("event");
            const std::string subject = object->getValue<std::string>("subject");

            auto& mongo = Ctx_.Mongo();
            std::vector<TUserState> states = mongo.LoadByAuth(email, token);

            for (const TUserState& state : states) {
                auto client = ConstructClient(state.source(), Ctx_);

                TUpdate update;
                update.UserId = state.userid();

                TReaction reaction;
                std::stringstream ss;
                ss << "notify.ticket.header" << "\n"
                    << "notify.ticket.subject" << ": \"" << subject << "\"\n"
                    << "notify.ticket.status." << event << "\n";
                reaction.Text = ss.str();
                TranslateReaction(reaction, state.language(), Ctx_.Translate());

                client->SendReaction(update, reaction);
            }
        }
    }

    void OnNotifyAnnouncement(Poco::Dynamic::Var result) {
        auto& logger = Poco::Logger::get("notify_handler");

        auto object = result.extract<Poco::JSON::Object::Ptr>();
        if (!object->has("users") || !object->has("announcement")) {
            return;
        }

        const auto announce = object->getObject("announcement");
        const auto usersArr = object->getArray("users");
        auto& mongo = Ctx_.Mongo();

        for (size_t i = 0; i < usersArr->size(); ++i) {
            const auto& user = usersArr->getObject(i);
            if (!user->has("email") || !user->has("token")) {
                continue;
            }
            const std::string email = user->getValue<std::string>("email");
            const std::string token = user->getValue<std::string>("token");
            logger.information("send notify to email %s, token %s", email, token);

            std::vector<TUserState> states = mongo.LoadByAuth(email, token);
            for (const TUserState& state : states) {
                const std::string locale = state.language() == TUserState_ELanguage_EN ? "en" : "ru";
                auto client = ConstructClient(state.source(), Ctx_);

                TUpdate update;
                update.UserId = state.userid();

                TReaction reaction;
                std::stringstream ss;
                ss << "notify.announcement.header" << "\n\n"
                    << "notify.announcement.type.title" << ": " << (announce->getValue<bool>("is_special") ? "notify.announcement.type.service" : "notify.announcement.type.info") << "\n\n"
                    << "notify.announcement.title" << ": \"" << announce->getValue<std::string>("title_" + locale) << "\"\n\n"
                    << "notify.announcement.body" << ": \"" << announce->getValue<std::string>("body_" + locale) << "\"\n";

                if (announce->has("attachment") && !announce->isNull("attachment")) {
                    ss << "\n" << "notify.announcement.has-attachment";
                }

                reaction.Text = ss.str();
                TranslateReaction(reaction, state.language(), Ctx_.Translate());

                client->SendReaction(update, reaction);
            }
        }
    }

private:
    TContext& Ctx_;
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

    const std::string& uri = request.getURI();
    logger.information("Got request at URI %s", uri);

    if (uri == "/ping") {
        return new TPingHandler();
    }

    if (uri == "/api/telegram") {
        return new TClientHandler<TTelegramClient>(Ctx_);
    }

    if (uri == "/api/vkontakte") {
        return new TClientHandler<TVkontakteClient>(Ctx_);
    }

    // `uri` starts with "/notify"
    if (uri.rfind("/notify", 0) == 0) {
        return new TNotifyHandler(Ctx_);
    }

    return new TDummyHandler();
}

} // namespace NOctoshell
