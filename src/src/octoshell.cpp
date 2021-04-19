#include "octoshell.h"
#include "context.h"

#include <regex>
#include <sstream>

#include <Poco/Logger.h>
#include <Poco/URI.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPSClientSession.h>

using namespace Poco::Net;

namespace NOctoshell {

namespace {

std::string UrlQuote(std::string s) {
    s = std::regex_replace(s, std::regex(" "), "%20");
    s = std::regex_replace(s, std::regex("\n"), "%0A");
    s = std::regex_replace(s, std::regex("\r"), "%0D");
    return s;
}

} // namespace

std::string TOctoshell::SendQueryWithAuth(const TUserState& state, const std::unordered_map<std::string, std::string>& params) {
    auto& logger = Poco::Logger::get("octoshell");
    logger.information("add user's mail and token to the query");

    auto paramsCopy = params;
    paramsCopy["email"] = state.email();
    paramsCopy["token"] = state.token();

    return SendQuery(paramsCopy);
}

std::string TOctoshell::SendQuery(const std::unordered_map<std::string, std::string>& params) {
    auto& logger = Poco::Logger::get("octoshell");
    logger.information("send query to octoshell server");

    std::stringstream ss;
    ss << Ctx_.Config().getString("octoshell.url");
    if (!params.empty()) {
        ss << "?";
    }
    bool needAmpersand = false;
    for (const auto& [key, value] : params) {
        if (needAmpersand) {
            needAmpersand = false;
            ss << "&";
        }
        ss << key << "=" << value;
        needAmpersand = true;
    }

    Poco::URI uri{UrlQuote(ss.str())};

    std::string path(uri.getPathAndQuery());
    if (path.empty()) {
        path = "/";
    }

    std::unique_ptr<HTTPClientSession> session;
    if (uri.getScheme() == "http") {
        session = std::make_unique<HTTPClientSession>(uri.getHost(), uri.getPort());
    } else {
        session = std::make_unique<HTTPSClientSession>(uri.getHost(), uri.getPort());
    }

    const Poco::Timespan ts(/* seconds = */ 5L, /* microseconds = */ 0L);
    session->setTimeout(ts);

    HTTPRequest request(HTTPRequest::HTTP_GET, path, HTTPMessage::HTTP_1_1);
    session->sendRequest(request);

    HTTPResponse response;
    std::istream& rs = session->receiveResponse(response);

    std::stringstream responseStream;
    responseStream << rs.rdbuf();
    logger.information("sendMessage response: code %d, reason %s, body %s", static_cast<int>(response.getStatus()), response.getReason(), responseStream.str());

    return responseStream.str();
}

} // namespace NOctoshell
