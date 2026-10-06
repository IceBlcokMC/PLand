#pragma once
#include "pch.h"

namespace land::scripting {

struct EngineOwnData {
    jspp::Global<jspp::Object> scriptMod{};
};

} // namespace land::scripting