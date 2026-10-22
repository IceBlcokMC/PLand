#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "ll/api/i18n/I18n.h"
#include <jspp/core/Exception.h>

namespace land::scripting {

using ll::i18n::I18n;
decltype(Modules::kScriptI18n) Modules::kScriptI18n = //
    jspp::binding::defClass<I18n>("I18n")
        .ctor()
        .method(
            "load",
            [](I18n& self, std::filesystem::path const& path) {
                if (auto expected = self.load(path); !expected) {
                    throw jspp::Exception{expected.error().message()};
                }
            }
        )
        .method("clear", &I18n::clear)
        .method("get", &I18n::get)
        .method("set", &I18n::set)
        .var_readonly("defaultLocaleCode", ll::i18n::getDefaultLocaleCode)
        .build();

} // namespace land::scripting