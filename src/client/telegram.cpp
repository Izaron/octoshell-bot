#include "telegram.h"

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

void TTelegramClient::SendReaction(const TReaction& reaction) const {
    // TODO: realize better
    auto& logger = Poco::Logger::get("telegram");
    Poco::URI uri{"https://api.telegram.org/botXXXX/sendMessage?chat_id=202048837&text=" + reaction.Text};

    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }

    HTTPSClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    //request.setContentType("application/x-www-form-urlencoded");
    //request.setKeepAlive(true);
    
    //Poco::JSON::Object msg;
    //msg.set("message", "hello!");

    //std::stringstream msgStream;
    //msg.stringify(msgStream);
    //std::string msgJson = msgStream.str();
    //logger.information("sendMessage json: %s", msgJson);

    //request.setContentLength(msgJson.length());

    //std::ostream& os = session.sendRequest(request);
    //os << msgJson;
    session.sendRequest(request);

    HTTPResponse response;
    std::istream& rs = session.receiveResponse(response);

    std::stringstream ss;
    ss << rs.rdbuf();
    logger.information("sendMessage response: code %d, reason %s, body %s", static_cast<int>(response.getStatus()), response.getReason(), ss.str());
}

} // namespace NOctoshell
