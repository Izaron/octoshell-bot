#include <iostream>
#include <memory>

#include <Poco/Util/ServerApplication.h>
#include <Poco/Logger.h>
#include <Poco/SimpleFileChannel.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/AutoPtr.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>

#include "context.h"

using namespace Poco;

void InitLogs() {
    //AutoPtr<SimpleFileChannel> channel(new SimpleFileChannel);
    //channel->setProperty("path", "octoshell.log");
    AutoPtr<Poco::ConsoleChannel> channel(new ConsoleChannel);

    AutoPtr<PatternFormatter> patternFormatter(new PatternFormatter);
    patternFormatter->setProperty("pattern", "[%Y/%b/%d %H:%M:%S] [thread %I] [%p] [%s] %t");

    AutoPtr<FormattingChannel> formattingChannel(new FormattingChannel(patternFormatter, channel));
    Logger::root().setChannel(formattingChannel.get());
}

class TApp final : public Util::ServerApplication {
private:
	int main(const std::vector<std::string>& args) override {
        InitLogs();

        NOctoshell::TContext ctx;

        ctx.StartServer();
        waitForTerminationRequest();
        ctx.StopServer();

        return 0;
    }
};

POCO_SERVER_MAIN(TApp)
