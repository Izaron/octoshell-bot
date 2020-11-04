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
    
    std::string Text;
    std::vector<TKeyboardRow> Keyboard;
    bool ForceReply;
};

using TReactions = std::vector<TReaction>;

} // namespace NOctoshell
