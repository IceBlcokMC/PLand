#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include <ll/api/io/FileUtils.h>

#include <optional>

namespace land::scripting {

namespace {

namespace fs = std::filesystem;

inline bool a_exists(std::filesystem::path const& path) {
    std::error_code ec;
    return fs::exists(path, ec);
}

inline std::optional<std::string> readFile(std::filesystem::path const& path) {
    if (!a_exists(path)) {
        return std::nullopt;
    }
    return ll::file_utils::readFile(path);
}

inline bool writeFile(std::filesystem::path const& path, std::string const& content) {
    return ll::file_utils::writeFile(path, content);
}

inline bool copy(std::filesystem::path const& src, std::filesystem::path const& dst) {
    std::error_code ec;
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    return !ec;
}

inline bool move(std::filesystem::path const& src, std::filesystem::path const& dst) {
    std::error_code ec;
    fs::rename(src, dst, ec);
    if (!ec) {
        return true;
    }
    ec.clear();
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        return false;
    }
    fs::remove(src, ec);
    return !ec;
}

inline bool mkdir(std::filesystem::path const& path) {
    std::error_code ec;
    fs::create_directory(path, ec);
    return !ec;
}
inline bool mkdir(std::filesystem::path const& path, bool recursive) {
    std::error_code ec;
    if (recursive) {
        fs::create_directories(path, ec);
    } else {
        fs::create_directory(path, ec);
    }
    return !ec;
}

inline bool appendFile(std::filesystem::path const& path, std::string const& content, bool createIfNotExists) {
    std::error_code ec;
    if (!createIfNotExists && !a_exists(path)) {
        return false;
    }
    std::ofstream out(path, std::ios::binary | std::ios::app);
    if (!out) {
        return false;
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return out.good();
}
inline bool appendFile(std::filesystem::path const& path, std::string const& content) {
    return appendFile(path, content, true);
}

inline bool remove(std::filesystem::path const& path) {
    std::error_code ec;
    fs::remove(path, ec);
    return !ec;
}
inline bool remove(std::filesystem::path const& path, bool recursive) {
    std::error_code ec;
    if (recursive) {
        fs::remove_all(path, ec);
    } else {
        fs::remove(path, ec);
    }
    return !ec;
}

} // namespace

decltype(Modules::kScriptFileSystem) Modules::kScriptFileSystem = //
    jspp::binding::defClass<void>("fs")
        .func("exists", a_exists)
        .func("readFile", readFile)
        .func("writeFile", writeFile)
        .func("copy", copy)
        .func("move", move)
        .func(
            "mkdir",
            static_cast<bool (*)(std::filesystem::path const&)>(&mkdir),
            static_cast<bool (*)(std::filesystem::path const&, bool)>(&mkdir)
        )
        .func(
            "appendFile",
            static_cast<bool (*)(std::filesystem::path const&, std::string const&)>(&appendFile),
            static_cast<bool (*)(std::filesystem::path const&, std::string const&, bool)>(&appendFile)
        )
        .func(
            "remove",
            static_cast<bool (*)(std::filesystem::path const&)>(&remove),
            static_cast<bool (*)(std::filesystem::path const&, bool)>(&remove)
        )
        .build();

} // namespace land::scripting