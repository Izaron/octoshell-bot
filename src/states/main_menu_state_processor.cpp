#include "main_menu_state_processor.h"

#include <Poco/Logger.h>

#include <inttypes.h>

namespace NOctoshell {

TReactions TMainMenuStatesProcessor::OnStart(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("main_menu_processor");
    logger.information("main menu on_start from user %" PRIu64, update.UserId);

    TReaction r1;
    r1.Text = "Sample text";

    TReaction r2;
    r2.Text = "You wrote: " + update.Text;

    return {std::move(r1), std::move(r2)};
}

TReactions TMainMenuStatesProcessor::OnUpdate(TUpdate update, TUserState& state) {
    auto& logger = Poco::Logger::get("main_menu_processor");
    logger.information("main menu on_update from user %" PRIu64, update.UserId);

    return {};
}

} // namespace NOctoshell
