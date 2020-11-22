#pragma once

#include "context.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

namespace NOctoshell {

class THandlerFactory final : public Poco::Net::HTTPRequestHandlerFactory {
public:
    THandlerFactory(TContext& ctx);
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request) override;

private:
    TContext& Ctx_;
};

} // namespace NOctoshell
