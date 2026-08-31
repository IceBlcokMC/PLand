#pragma once
#include "ll/api/Expected.h"
#include "pland/internal/import_framework/adapter/GenericSourceAdapter.h"

namespace land::internal::import_framework::adapter {

struct LevelDbAdapter {
    static ll::Expected<nlohmann::json> readAllFromBasicJsonDb(std::string_view dbDir, ValueDecoder_t const& decoder);

    static std::unique_ptr<GenericSourceAdapter> make();
};

} // namespace land::internal::import_framework::adapter