#include "ImportProcesser.h"
#include "mc/platform/UUID.h"
#include "nlohmann/detail/json_pointer.hpp"
#include "pland/PLand.h"
#include "pland/internal/import_framework/adapter/GenericSourceAdapter.h"
#include "pland/internal/import_framework/adapter/JsonAdapter.h"
#include "pland/internal/import_framework/adapter/LevelDbAdapter.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/utils/JsonUtil.h"


#include "ll/api/Expected.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/utils/HashUtils.h"

#include "nlohmann/json_fwd.hpp"

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "del/exception.h"
#include "del/template_engine.h"


namespace land::internal::import_framework {

namespace fs = std::filesystem;


class TemplateParser {
public:
    static constexpr int              kVersion                        = 1;
    static constexpr std::string_view kVersionKey                     = "$version";
    static constexpr std::string_view kSourceLoadersKey               = "$source-loaders";
    static constexpr std::string_view kSourceLoadersElementAdapterKey = "adapter";
    static constexpr std::string_view kSourceLoadersElementPathKey    = "path";
    static constexpr std::string_view kLandTemplateKey                = "land";

    static constexpr std::string_view kSourceLoaderJsonFileAdapter = "json-file";
    static constexpr std::string_view kSourceLoaderLevelDbAdapter  = "level-db";

    static constexpr std::array<std::string_view, 3> kRootRequiredKeys = {
        kVersionKey, //
        kSourceLoadersKey,
        kLandTemplateKey
    };
    static constexpr std::array<std::string_view, 2> kSourceLoaderRequiredKeys = {
        kSourceLoadersElementAdapterKey,
        kSourceLoadersElementPathKey
    };
    static constexpr std::array<std::string_view, 2> kSourceLoaderAdapterAcceptValues = {
        kSourceLoaderJsonFileAdapter,
        kSourceLoaderLevelDbAdapter
    };

public:
    static ll::Expected<TemplateDef> parse(nlohmann::ordered_json const& tmp, std::string const& sourcesDir) {
        if (!fs::exists(sourcesDir) || !fs::is_directory(sourcesDir)) {
            return ll::makeStringError("Sources directory does not exist, or is not a directory"_tr());
        }
        if (auto e = validateTemplate(tmp); !e) {
            return ll::forwardError(e.error());
        }

        auto ex_sou = loadAndMountSources(tmp, sourcesDir);
        if (!ex_sou) {
            return ll::forwardError(ex_sou.error());
        }

        TemplateDef def;
        def.sources = std::move(ex_sou.value());
        def.landTmp = tmp[kLandTemplateKey];

        return def;
    }

    static ll::Expected<TemplateDef> parse(std::string const& templatePath, std::string const& sourcesDir) {
        if (!fs::exists(templatePath) || !fs::is_regular_file(templatePath)) {
            return ll::makeStringError("Template file does not exist, or is not a regular file"_tr());
        }

        auto ex_tmp = loadTemplate(templatePath);
        if (!ex_tmp) {
            return ll::forwardError(ex_tmp.error());
        }
        auto tmp = std::move(ex_tmp.value());
        return parse(tmp, sourcesDir);
    }

private:
    static ll::Expected<nlohmann::ordered_json> loadTemplate(std::string const& templatePath) {
        try {
            auto opt = ll::file_utils::readFile(templatePath);
            if (!opt) {
                return ll::makeStringError("Failed to read template file: {}"_tr(templatePath));
            }
            return nlohmann::ordered_json::parse(opt.value());
        } catch (std::exception const& e) {
            return ll::makeStringError(e.what());
        }
    }

    static ll::Expected<>
    _ensureRequiredFields(nlohmann::ordered_json const& tmp, std::span<std::string_view const> keys) {
        for (auto const& key : keys) {
            if (!tmp.contains(key)) {
                return ll::makeStringError("Template does not contain required key: {}"_tr(key));
            }
        }
        return {};
    }

    static ll::Expected<> validateTemplate(nlohmann::ordered_json const& tmp) {
        // ensure required fields (root)
        if (auto e = _ensureRequiredFields(tmp, kRootRequiredKeys); !e) {
            return e;
        }
        {
            auto& version = tmp[kVersionKey];
            if (!version.is_number_integer()) {
                return ll::makeStringError("Template version is not an integer"_tr());
            }
            auto v = version.get<int>();
            if (v != kVersion) {
                return ll::makeStringError("Template version is not supported, expected {} but got {}"_tr(kVersion, v));
            }
        }
        {
            auto& sourceLoaders = tmp[kSourceLoadersKey];
            if (!sourceLoaders.is_object()) {
                return ll::makeStringError("Template source loaders is not an object"_tr());
            }
            for (auto const& [key, value] : sourceLoaders.items()) {
                if (auto e = _ensureRequiredFields(value, kSourceLoaderRequiredKeys); !e) {
                    return e;
                }
                {
                    auto& adapter = value[kSourceLoadersElementAdapterKey];
                    if (!adapter.is_string()) {
                        return ll::makeStringError("Template source loader adapter is not a string"_tr());
                    }
                    auto adapter_v = adapter.get<std::string>();
                    if (std::find(
                            kSourceLoaderAdapterAcceptValues.begin(),
                            kSourceLoaderAdapterAcceptValues.end(),
                            adapter_v
                        )
                        == kSourceLoaderAdapterAcceptValues.end()) {
                        return ll::makeStringError("Template source loader adapter is not supported: {}"_tr(adapter_v));
                    }
                }
                {
                    auto& path = value[kSourceLoadersElementPathKey];
                    if (!path.is_string()) {
                        return ll::makeStringError("Template source loader path is not a string"_tr());
                    }
                }
            }
        }
        return {};
    }

    static std::unique_ptr<adapter::GenericSourceAdapter> loadAdapter(std::string_view type) {
        if (type == kSourceLoaderJsonFileAdapter) {
            return adapter::JsonAdapter::make();
        } else if (type == kSourceLoaderLevelDbAdapter) {
            return adapter::LevelDbAdapter::make();
        }
        return nullptr;
    }

    static ll::Expected<nlohmann::json>
    loadAndMountSources(nlohmann::ordered_json const& tmp, std::string const& sourcesDir) {
        nlohmann::json sources;

        for (auto const& [key, value] : tmp[kSourceLoadersKey].items()) {
            auto& adapter = value[kSourceLoadersElementAdapterKey];
            auto& path    = value[kSourceLoadersElementPathKey];

            auto adapter_v = adapter.get<std::string>();
            auto path_v    = path.get<std::string>();

            auto finalPath = fs::path(sourcesDir) / path_v;
            finalPath      = fs::weakly_canonical(finalPath);
            if (!fs::exists(finalPath)) {
                return ll::makeStringError("Source path does not exist: {}"_tr(finalPath));
            }

            auto adapter_obj = loadAdapter(adapter_v);
            if (!adapter_obj) {
                return ll::makeStringError("Failed to load adapter: {}"_tr(adapter_v));
            }

            auto fp  = finalPath.string();
            auto opt = adapter_obj->load(fp);
            if (!opt) {
                return ll::forwardError(opt.error());
            }

            auto jp     = nlohmann::json_pointer<std::string>(key);
            sources[jp] = std::move(*opt);
        }

        return sources;
    }
};


struct ImportProcesser::Impl {
    static nlohmann::json
    isUuid(del::Arguments const& args, del::EvaluationContext& ctx, del::ExprEvaluator const& eval) {
        if (args.size() != 1) {
            throw del::RuntimeError("is_uuid expected 1 string argument");
        }
        auto arg = eval(*args[0], ctx);
        if (!arg.is_string()) {
            throw del::RuntimeError("is_uuid expected 1 string argument");
        }
        return mce::UUID::canParse(arg.get<std::string>());
    }

    static std::unique_ptr<del::TemplateEngine> newTemplateEngine() {
        auto engine = std::make_unique<del::TemplateEngine>();
        engine->RegisterCustomFunction("is_uuid", &isUuid);
        return engine;
    }

    static ll::Expected<>
    process(const std::string& templatePath, const std::string& sourcesDir, bool clearDbBeforeImport) {
        auto ex_def = TemplateParser::parse(templatePath, sourcesDir);
        if (!ex_def) {
            return ll::forwardError(ex_def.error());
        }
        auto def = std::move(*ex_def);

        auto& mod = PLand::getInstance();

        auto& registry = mod.getLandRegistry();
        registry.createSnapshot("import_before_backup");

        if (clearDbBeforeImport) {
            // TODO: 清理领地、缓存、服务类
        }

        auto engine = newTemplateEngine();
        assert(engine);

        try {
            if (def.landTmp.contains("$records")) {
                auto cp = engine->CompileRecordTemplate(def.landTmp);
                engine->ExecuteRecords(cp, def.sources, [](std::string const& key, nlohmann::json record) -> bool {
                    return true; // TODO: implement
                });
            } else {
                auto cp    = engine->Compile(def.landTmp);
                auto datas = engine->Execute(cp, def.sources);
                // TODO: implement
            }
            return {};
        } catch (std::exception const& ex) {
            return ll::makeStringError("Failed to execute template: {}"_tr(ex.what()));
        }
    }
};

std::unique_ptr<del::TemplateEngine> ImportProcesser::newTemplateEngine() { return Impl::newTemplateEngine(); }

ll::Expected<TemplateDef>
ImportProcesser::parseTemplate(nlohmann::ordered_json const& tmp, std::string const& sourcesDir) {
    return TemplateParser::parse(tmp, sourcesDir);
}

ll::Expected<>
ImportProcesser::process(const std::string& templatePath, const std::string& sourcesDir, bool clearDbBeforeImport) {
    return Impl::process(templatePath, sourcesDir, clearDbBeforeImport);
}

} // namespace land::internal::import_framework