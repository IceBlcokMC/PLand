#pragma once
#include "ll/api/Expected.h"

#include "nlohmann/json.hpp"
#include "nlohmann/json_fwd.hpp"

#include <memory>
#include <string>

namespace del {
class TemplateEngine; // fwd
}

namespace land::internal::import_framework {

struct TemplateDef {
    nlohmann::json sources;
    nlohmann::json landTmp;
};

class ImportProcesser {
    struct Impl;

public:
    // for devtools
    static std::unique_ptr<del::TemplateEngine> newTemplateEngine();
    static ll::Expected<TemplateDef> parseTemplate(nlohmann::ordered_json const& tmp, std::string const& sourcesDir);

    static ll::Expected<>
    process(std::string const& templatePath, std::string const& sourcesDir, bool clearDbBeforeImport);
};

} // namespace land::internal::import_framework