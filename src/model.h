#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace NOctoshell {

/*
 * Update from a messager
 */
struct TUpdate {
    std::uint64_t UserId;
    std::string Text;
};

/*
 * Reaction from the bot
 */
struct TReaction {
    using TKeyboardRow = std::vector<std::string>;
    using TKeyboard = std::vector<TKeyboardRow>;
    
    std::string Text;
    TKeyboard Keyboard;
    bool ForceReply = false;
};

using TReactions = std::vector<TReaction>;

} // namespace NOctoshell
