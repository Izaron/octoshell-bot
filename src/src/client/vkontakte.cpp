#include "vkontakte.h"
#include "../context.h"

#include <regex>
#include <sstream>

#include <Poco/Logger.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>

using namespace Poco::Net;

namespace NOctoshell {

namespace {

Poco::JSON::Object ConstructReplyMarkup(const TReaction::TKeyboard& keyboard) {
    Poco::JSON::Object markup;
    markup.set("one_time", false);
    markup.set("inline", false);

    Poco::JSON::Array keyboardArr;
    for (const auto& row : keyboard) {
        Poco::JSON::Array keyboardRow;
        for (const auto& text : row) {
            Poco::JSON::Object action;
            action.set("type", "text");
            action.set("label", text);

            Poco::JSON::Object button;
            button.set("action", action);
            keyboardRow.add(button);
        }
        keyboardArr.add(keyboardRow);
    }

    markup.set("buttons", keyboardArr);
    return markup;
}

std::string ConstructReplyMarkupJson(const TReaction::TKeyboard& keyboard) {
    auto obj = ConstructReplyMarkup(keyboard);
    std::stringstream ss;
    obj.stringify(ss);
    return ss.str();
}

std::string UrlQuote(std::string s) {
    s = std::regex_replace(s, std::regex(" "), "%20");
    s = std::regex_replace(s, std::regex("\n"), "%0A");
    return s;
}

} // namespace

TUpdate TVkontakteClient::ParseUpdate(const Poco::JSON::Object& data) const {
    Poco::Logger::get("vkontakte").information("parsing vkontakte update");

    TUpdate update;
    if (data.has("object")) {
        auto obj = data.getObject("object");
        if (obj->has("message")) {
            auto msg = obj->getObject("message");
            update.UserId = msg->getValue<std::uint64_t>("peer_id");
            update.Text = msg->getValue<std::string>("text");
        }
    }
    return update;
}

void TVkontakteClient::SendReaction(const TUpdate& update, const TReaction& reaction) const {
    auto& logger = Poco::Logger::get("vkontakte");
    logger.information("send vkontakte reaction");

    std::stringstream ss;
    ss << "https://api.vk.com/method/messages.send";
    ss << "?message=" << UrlQuote(reaction.Text);
    ss << "&peer_id=" << update.UserId;
    ss << "&access_token=" << Ctx_.Config().getString("vk.access-token");
    ss << "&v=" << "5.124";
    ss << "&random_id=" << "0";
    ss << "&keyboard=" << UrlQuote(ConstructReplyMarkupJson(reaction.Keyboard));

    Poco::URI uri{UrlQuote(ss.str())};

    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }

    HTTPSClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_POST, path, HTTPMessage::HTTP_1_1);
    request.setContentLength(0);
    session.sendRequest(request);

    HTTPResponse response;
    std::istream& rs = session.receiveResponse(response);

    std::stringstream responseStream;
    responseStream << rs.rdbuf();
    logger.information("sendMessage response: code %d, reason %s, body %s", static_cast<int>(response.getStatus()), response.getReason(), responseStream.str());
}

} // namespace NOctoshell
