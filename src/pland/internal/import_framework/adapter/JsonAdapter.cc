#include "JsonAdapter.h"
#include "pland/Global.h"

#include "ll/api/Expected.h"
#include "ll/api/io/FileUtils.h"

#include <filesystem>

namespace land::internal::import_framework::adapter {


ll::Expected<nlohmann::json> JsonAdapter::decoder(std::string_view raw) {
    try {
        return nlohmann::json::parse(raw);
    } catch (nlohmann::json::parse_error const& e) {
        return ll::makeStringError(e.what());
    }
}

ll::Expected<nlohmann::json>
JsonAdapter::readAllFromUniqueFile(std::string_view filePath, ValueDecoder_t const& decoder) {
    namespace fs = std::filesystem;

    auto path = fs::path(filePath);
    if (!fs::exists(path)) {
        return ll::makeStringError("File not found: {}"_tr(filePath));
    }

    if (fs::is_directory(path)) {
        return ll::makeStringError("File is a directory: {}"_tr(filePath));
    }

    auto raw = ll::file_utils::readFile(path);
    if (!raw.has_value()) {
        return ll::makeStringError("Failed to read file, the file is empty: {}"_tr(filePath));
    }

    return decoder(*raw);
}

std::unique_ptr<GenericSourceAdapter> JsonAdapter::make() {
    return GenericSourceAdapter::make(readAllFromUniqueFile, decoder);
}

} // namespace land::internal::import_framework::adapter