#include "telegram.h"
#include "../context.h"

#include <regex>

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
    markup.set("resize_keyboard", true);

    Poco::JSON::Array keyboardArr;
    for (const auto& row : keyboard) {
        Poco::JSON::Array keyboardRow;
        for (const auto& text : row) {
            Poco::JSON::Object button;
            button.set("text", text);
            keyboardRow.add(button);
        }
        keyboardArr.add(keyboardRow);
    }

    markup.set("keyboard", keyboardArr);
    return markup;
}

std::string ConstructReplyMarkupJson(const TReaction::TKeyboard& keyboard) {
    auto obj = ConstructReplyMarkup(keyboard);
    std::stringstream ss;
    obj.stringify(ss);
    return ss.str();
}

std::string UrlQuote(std::string s, bool escapeUnderscore = false) {
    s = std::regex_replace(s, std::regex(" "), "%20");
    s = std::regex_replace(s, std::regex("\n"), "%0A");
    s = std::regex_replace(s, std::regex("\r"), "%0D");

    std::string res;
    if (escapeUnderscore) {
        bool inCode = false;
        for (const auto c : s) {
            if (c == '`') {
                inCode = !inCode;
            }
            if (c == '_' && !inCode) {
                res += "\\";
            }
            res += c;
        }
    } else {
        res = s;
    }

    return res;
}

} // namespace

TUpdate TTelegramClient::ParseUpdate(const Poco::JSON::Object& data) const {
    Poco::Logger::get("telegram").information("parsing telegram update");

    TUpdate update;

    if (data.has("message")) {
        auto msg = data.getObject("message");
        if (msg->has("from")) {
            update.UserId = msg->getObject("from")->getValue<std::uint64_t>("id");
        }
        if (msg->has("text")) {
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
    ss << "&text=" << UrlQuote(reaction.Text, /* escapeUnderscore = */ true);
    if (!reaction.Keyboard.empty()) {
        ss << "&reply_markup=" << UrlQuote(ConstructReplyMarkupJson(reaction.Keyboard), /* escapeUnderscore = */ true);
    }
    if (reaction.ForceReply) {
        ss << "&reply_markup=" << R"({"force_reply":true})";
    }
    ss << "&parse_mode=markdown";

    Poco::URI uri{UrlQuote(ss.str())};

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

TUserState_ESource TTelegramClient::Source() const {
    return TUserState_ESource_TELEGRAM;
}

} // namespace NOctoshell
