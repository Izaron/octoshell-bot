#pragma once

#include <string>

#include "octoshell.h"
#include <proto/user_state.pb.h>

namespace NOctoshell {

enum class EAuthStatus {
    SUCCESS = 0,
    INACTIVE_TOKEN = 1,
    WRONG_TOKEN = 2,
    WRONG_EMAIL = 3,
    SERVICE_UNAVAILABLE = 4,
};

EAuthStatus TryAuth(TOctoshell& octoshell, const TUserState& state);

static std::unordered_map<EAuthStatus, std::string> AuthToTemplateMap = {
    {EAuthStatus::SUCCESS, "auth.status.success"},
    {EAuthStatus::INACTIVE_TOKEN, "auth.status.inactive-token"},
    {EAuthStatus::WRONG_TOKEN, "auth.status.wrong-token"},
    {EAuthStatus::WRONG_EMAIL, "auth.status.wrong-email"},
    {EAuthStatus::SERVICE_UNAVAILABLE, "auth.status.service-unavailable"},
};

} // namespace NOctoshell
