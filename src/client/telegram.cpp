#include "telegram.h"
#include "../context.h"

#include <Poco/Logger.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

using namespace Poco::Net;

namespace NOctoshell {

std::string TTelegramClient::Name() const {
    return "Telegram";
}

TUpdate TTelegramClient::ParseUpdate(const Poco::JSON::Object& data) const {
    TUpdate update;

    if (data.has("message")) {
        auto msg = data.getObject("message");
        if (msg->has("from") && msg->has("text")) {
            update.UserId = msg->getObject("from")->getValue<std::uint64_t>("id");
            update.Text = msg->getValue<std::string>("text");
        }
    }
    return update;
}

void TTelegramClient::SendReaction(const TUpdate& update, const TReaction& reaction) const {
    auto& logger = Poco::Logger::get("telegram");
    logger.information("send telegram reaction");

    std::stringstream ss;
    ss << "https://api.telegram.org/bot" << Ctx_.Config().getString("telegram.token") << "/sendMessage";
    ss << "?chat_id=" << std::to_string(update.UserId);
    ss << "&text=" << reaction.Text;
    Poco::URI uri{ss.str()};

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
    logger.information("sendMessage response: code %d, reason %s, body %s", static_cast<int>(response.getStatus()), response.getReason(), responseStream.str());
}

} // namespace NOctoshell
