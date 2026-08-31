#pragma once
#include "nlohmann/json.hpp"

#include "ll/api/Expected.h"

#include <memory>
#include <string_view>
#include <utility>

namespace land::internal::import_framework::adapter {

using ValueDecoder_t  = std::function<ll::Expected<nlohmann::json>(std::string_view rawData)>;
using StorageReader_t = std::function<ll::Expected<nlohmann::json>(
    std::string_view      inputSource, //
    ValueDecoder_t const& decoder
)>;

class GenericSourceAdapter {
private:
    StorageReader_t mReader{nullptr};
    ValueDecoder_t  mDecoder{nullptr};

public:
    GenericSourceAdapter(StorageReader_t reader, ValueDecoder_t decoder)
    : mReader(std::move(reader)),
      mDecoder(std::move(decoder)) {}

    [[nodiscard]] inline ll::Expected<nlohmann::json> load(std::string_view inputSource) {
        return mReader(inputSource, mDecoder);
    }


    /// factroy

    [[nodiscard]] static inline std::unique_ptr<GenericSourceAdapter>
    make(StorageReader_t reader, ValueDecoder_t decoder) {
        return std::make_unique<GenericSourceAdapter>(std::move(reader), std::move(decoder));
    }
};

} // namespace land::internal::import_framework::adapter