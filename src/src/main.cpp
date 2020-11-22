#include <iostream>
#include <memory>

#include <Poco/Util/ServerApplication.h>

#include "context.h"

using namespace Poco;

class TApp final : public Util::ServerApplication {
private:
    int main(const std::vector<std::string>& args) override {
        if (args.size() < 1) {
            std::cerr << "You should provide path to .properties file" << std::endl;
            return 1;
        }

        NOctoshell::TContext ctx(args[0]);

        ctx.StartServer();
        waitForTerminationRequest();
        ctx.StopServer();

        return 0;
    }
};

POCO_SERVER_MAIN(TApp)
