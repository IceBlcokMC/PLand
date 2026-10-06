#pragma once
#include <memory>

#include "ll/api/Expected.h"

namespace ll::mod {
class Mod;
}

namespace land::scripting {

class ScriptTimerSystem;

class Bootstrap {
    class Impl;
    std::unique_ptr<Impl> impl{nullptr};

public:
    explicit Bootstrap(ll::mod::Mod& mod);
    ~Bootstrap();

    ScriptTimerSystem& getScriptTimerSystem();

    [[nodiscard]] ll::Expected<> initialize();
    [[nodiscard]] ll::Expected<> shutdown();
    [[nodiscard]] ll::Expected<> reload();

    [[nodiscard]] ll::Expected<> postOnLoad() const;
    [[nodiscard]] ll::Expected<> postOnEnable() const;
    [[nodiscard]] ll::Expected<> postOnDisable() const;
};

} // namespace land::scripting