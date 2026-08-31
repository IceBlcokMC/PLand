#pragma once
#include "GenericSourceAdapter.h"

#include <memory>

namespace land::internal::import_framework::adapter {

struct JsonAdapter {

    static ll::Expected<nlohmann::json> decoder(std::string_view raw);

    static ll::Expected<nlohmann::json> readAllFromUniqueFile(std::string_view filePath, ValueDecoder_t const& decoder);

    static std::unique_ptr<GenericSourceAdapter> make();
};


} // namespace land::internal::import_framework::adapter