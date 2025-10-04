#pragma once
#include "pland/Global.h"

namespace land {

struct LandCommand {
    LandCommand() = delete;
    LDAPI static bool setup();
};


} // namespace land