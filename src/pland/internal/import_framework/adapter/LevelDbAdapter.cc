#include "LevelDbAdapter.h"
#include "GenericSourceAdapter.h"
#include "JsonAdapter.h"
#include "pland/Global.h"


#include "ll/api/Expected.h"
#include "ll/api/data/KeyValueDB.h"

namespace land::internal::import_framework::adapter {

ll::Expected<nlohmann::json>
LevelDbAdapter::readAllFromBasicJsonDb(std::string_view dbDir, ValueDecoder_t const& decoder) {
    namespace fs = std::filesystem;

    auto path = fs::path(dbDir);
    if (!fs::exists(path)) {
        return ll::makeStringError("Database not found, path: {}"_tr(dbDir));
    }

    if (!fs::is_directory(path)) {
        return ll::makeStringError("Database is not a directory, path: {}"_tr(dbDir));
    }

    auto db = std::make_unique<ll::data::KeyValueDB>(path);
    if (db->empty()) {
        return ll::makeStringError("Database is empty");
    }

    nlohmann::json out;

    auto iter = db->iter();
    for (auto [key, value] : iter) {
        auto ex = decoder(value);
        if (!ex) {
            return ll::forwardError(ex.error());
        }
        out[key] = std::move(ex.value());
    }
    return std::move(out);
}

std::unique_ptr<GenericSourceAdapter> LevelDbAdapter::make() {
    return GenericSourceAdapter::make(readAllFromBasicJsonDb, JsonAdapter::decoder);
}


} // namespace land::internal::import_framework::adapter