#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include <ll/api/io/FileUtils.h>

namespace land::scripting {

namespace {

namespace fs = std::filesystem;

// 脚本侧路径契约：POSIX 风格为主，'/' 为分隔符；兼容 Windows 风格输入（'\' 同样视作分隔符）。
// 相对路径不做基目录解析，由操作系统按进程 cwd 解析（与 Node.js fs 一致）。
inline std::string normalizeSeparators(std::string s) {
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

inline fs::path nativePath(std::string_view scriptPath) {
    return ll::file_utils::u8path(normalizeSeparators(std::string(scriptPath)));
}

inline std::string toPOSIXPath(fs::path const& p) {
    auto        u8 = p.generic_u8string();
    std::string s(reinterpret_cast<char const*>(u8.data()), u8.size());
    assert(s.find('\\') == std::string::npos); // 防呆：反斜杠不允许外泄到脚本
    return s;
}

// > fs module impl
// ================================

inline bool exists(std::string const& path) { return fs::exists(nativePath(path)); }

// 文本模式读取。非普通文件（不存在、目录等）统一返回 nullopt，保证平台一致。
inline std::optional<std::string> readFile(std::string const& path) {
    fs::path p = nativePath(path);
    if (!fs::is_regular_file(p)) {
        return std::nullopt;
    }
    std::ifstream in(p); // 文本模式
    if (!in) {
        return std::nullopt;
    }
    std::string data((std::istreambuf_iterator<char>(in)), {});
    if (in.bad()) {
        return std::nullopt;
    }
    return data;
}

// 文本模式写入，自动创建父目录。返回值 = 数据是否完整落盘。
inline void createParentDirs(fs::path const& p) {
    auto parent = p.parent_path();
    if (!parent.empty()) {
        fs::create_directories(parent);
    }
}

inline bool writeFile(std::string const& path, std::string const& content) {
    fs::path p = nativePath(path);
    createParentDirs(p);
    std::ofstream out(p); // 文本模式
    if (!out) {
        return false;
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.flush();
    return out.good();
}

// 文件或目录树拷贝：src 为目录时其内容被拷贝到 dst 下（dst 不存在则创建）。
inline bool copy(std::string const& src, std::string const& dst) {
    fs::path srcP = nativePath(src);
    fs::path dstP = nativePath(dst);
    createParentDirs(dstP);
    if (fs::is_directory(srcP)) {
        fs::copy(srcP, dstP, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
        return true;
    }
    return fs::copy_file(srcP, dstP, fs::copy_options::overwrite_existing);
}

// 真实 move：同卷直接 rename；跨卷（POSIX 挂载点 EXDEV）回退为递归拷贝后删除。
inline bool move(std::string const& src, std::string const& dst) {
    fs::path srcP = nativePath(src);
    fs::path dstP = nativePath(dst);
    createParentDirs(dstP);
    try {
        fs::rename(srcP, dstP);
        return true;
    } catch (fs::filesystem_error const& e) {
        if (e.code() != std::errc::cross_device_link) {
            throw; // 操作系统拒绝（权限等），原样抛给脚本
        }
    }
    fs::copy(srcP, dstP, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    fs::remove_all(srcP);
    return true;
}

inline bool mkdir(std::string const& path) { return fs::create_directory(nativePath(path)); }
inline bool mkdir(std::string const& path, bool recursive) {
    fs::path p = nativePath(path);
    if (recursive) {
        return fs::create_directories(p);
    }
    return fs::create_directory(p);
}

// 追加写：内容原样写入（不做换行转换），换行符由脚本自行处理。
inline bool appendFile(std::string const& path, std::string const& content, bool createIfNotExists) {
    fs::path p = nativePath(path);
    if (!createIfNotExists && !fs::exists(p)) {
        return false;
    }
    if (createIfNotExists) {
        createParentDirs(p);
    }
    std::ofstream out(p, std::ios::binary | std::ios::app);
    if (!out) {
        return false;
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.flush();
    return out.good();
}
inline bool appendFile(std::string const& path, std::string const& content) {
    return appendFile(path, content, false);
}

// 删除：始终递归，返回是否有条目被删除。
inline bool remove(std::string const& path) { return fs::remove_all(nativePath(path)) > 0; }

inline bool isDirectory(std::string const& path) { return fs::is_directory(nativePath(path)); }

inline bool isFile(std::string const& path) { return fs::is_regular_file(nativePath(path)); }


// > path module impl（纯字符串 POSIX，对齐 Node.js path.posix）
// ================================

// 移植自 Node.js v22 lib/path.js normalizeString()。
std::string normalizeString(std::string_view path, bool allowAboveRoot) {
    std::string res;
    size_t      lastSegmentLength = 0;
    ptrdiff_t   lastSlash         = -1;
    int         dots              = 0;
    char        code              = 0;

    for (size_t i = 0; i <= path.size(); ++i) {
        if (i < path.size()) {
            code = path[i];
        } else if (code == '/') {
            break;
        } else {
            code = '/';
        }

        if (code == '/') {
            if (lastSlash == static_cast<ptrdiff_t>(i) - 1 || dots == 1) {
                // 空段或 "." 段：跳过
            } else if (dots == 2) {
                if (res.size() < 2 || lastSegmentLength != 2 || res[res.size() - 1] != '.'
                    || res[res.size() - 2] != '.') {
                    if (res.size() > 2) {
                        auto lastSlashIndex =
                            static_cast<ptrdiff_t>(res.size()) - static_cast<ptrdiff_t>(lastSegmentLength) - 1;
                        if (lastSlashIndex == -1) {
                            res.clear();
                            lastSegmentLength = 0;
                        } else {
                            res.resize(static_cast<size_t>(lastSlashIndex));
                            auto lastSep      = res.find_last_of('/');
                            lastSegmentLength = (lastSep == std::string::npos) ? res.size()
                                                                               : res.size() - 1 - lastSep;
                        }
                        lastSlash = static_cast<ptrdiff_t>(i);
                        dots      = 0;
                        continue;
                    } else if (!res.empty()) {
                        res.clear();
                        lastSegmentLength = 0;
                        lastSlash         = static_cast<ptrdiff_t>(i);
                        dots              = 0;
                        continue;
                    }
                }
                if (allowAboveRoot) {
                    res += res.empty() ? ".." : "/..";
                    lastSegmentLength = 2;
                }
            } else {
                if (!res.empty()) {
                    res += '/';
                }
                res.append(path.substr(static_cast<size_t>(lastSlash) + 1, i - static_cast<size_t>(lastSlash) - 1));
                lastSegmentLength = i - static_cast<size_t>(lastSlash) - 1;
            }
            lastSlash = static_cast<ptrdiff_t>(i);
            dots      = 0;
        } else if (code == '.' && dots != -1) {
            ++dots;
        } else {
            dots = -1;
        }
    }
    return res;
}

std::string normalize(std::string const& path) {
    std::string const normalized = normalizeSeparators(path);
    if (normalized.empty()) {
        return ".";
    }
    bool isAbsolute        = normalized.front() == '/';
    bool trailingSeparator = normalized.back() == '/';

    std::string res = normalizeString(normalized, !isAbsolute);
    if (res.empty()) {
        if (isAbsolute) return "/";
        return trailingSeparator ? "./" : ".";
    }
    if (trailingSeparator) {
        res += '/';
    }
    return isAbsolute ? "/" + res : res;
}

// 对齐 Node.js path.posix.join()：非空段以 '/' 连接后 normalize，全空返回 "."。
jspp::Local<jspp::Value> join(jspp::Arguments const& args) {
    std::vector<std::string> parts;
    for (size_t i = 0; i < args.length(); ++i) {
        auto raw = args[i].asString().getValue();
        if (!raw.empty()) {
            parts.push_back(normalizeSeparators(std::move(raw)));
        }
    }
    if (parts.empty()) {
        return jspp::String::newString(".");
    }
    std::string joined = parts.front();
    for (size_t i = 1; i < parts.size(); ++i) {
        joined += '/';
        joined += parts[i];
    }
    return jspp::String::newString(normalize(joined));
}
static_assert(jspp::binding::traits::isFunctionCallback_v<decltype(join)>);

// 对齐 Node.js path.posix.basename()
std::string basename(std::string const& path) {
    std::string const normalized = normalizeSeparators(path);
    std::string_view v{normalized};
    size_t           end = v.size();
    while (end > 0 && v[end - 1] == '/') {
        --end;
    }
    if (end == 0) {
        return {};
    }
    size_t start = v.rfind('/', end - 1);
    if (start == std::string_view::npos) {
        return std::string(v.substr(0, end));
    }
    return std::string(v.substr(start + 1, end - start - 1));
}

// 对齐 Node.js path.posix.dirname()
std::string dirname(std::string const& path) {
    std::string const normalized = normalizeSeparators(path);
    std::string_view v{normalized};
    if (v.empty()) {
        return ".";
    }
    bool hasRoot = v.front() == '/';
    if (v.size() == 1) {
        return hasRoot ? std::string("/") : std::string(".");
    }

    size_t end          = std::string_view::npos;
    bool   matchedSlash = true;
    for (size_t i = v.size() - 1; i >= 1; --i) {
        if (v[i] == '/') {
            if (!matchedSlash) {
                end = i;
                break;
            }
        } else {
            matchedSlash = false;
        }
    }
    if (end == std::string_view::npos) {
        return hasRoot ? std::string("/") : std::string(".");
    }
    if (hasRoot && end == 1) {
        return "//";
    }
    return std::string(v.substr(0, end));
}

// 对齐 Node.js path.posix.extname()。纯字符串，不经过 fs::path。
std::string extname(std::string const& path) {
    std::string const normalized = normalizeSeparators(path);
    std::string_view v{normalized};
    std::ptrdiff_t   startPart    = 0;
    std::ptrdiff_t   startDot     = -1;
    std::ptrdiff_t   end          = -1;
    bool             matchedSlash = true;
    int              preDotState  = 0;

    for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(v.size()) - 1; i >= 0; --i) {
        char c = v[static_cast<size_t>(i)];
        if (c == '/') {
            if (!matchedSlash) {
                startPart = i + 1;
                break;
            }
            continue;
        }
        if (end == -1) {
            matchedSlash = false;
            end          = i + 1;
        }
        if (c == '.') {
            if (startDot == -1) {
                startDot = i;
            } else if (preDotState != 1) {
                preDotState = 1;
            }
        } else if (startDot != -1) {
            preDotState = -1;
        }
    }

    if (startDot == -1 || end == -1 || preDotState == 0 ||
        // basename 恰好是 ".."
        (preDotState == 1 && startDot == end - 1 && startDot == startPart + 1)) {
        return {};
    }
    return std::string(v.substr(static_cast<size_t>(startDot), static_cast<size_t>(end - startDot)));
}

// 基于 cwd 的绝对化（对齐 Node path.resolve）：空输入回退 cwd，不保留尾分隔符（根形式除外）。
inline fs::path absoluteNormalized(std::string const& path) {
    fs::path p = fs::absolute(nativePath(path)).lexically_normal();
    if (p.empty()) {
        p = fs::current_path();
    }
    std::string s = toPOSIXPath(p);
    bool isRootForm = (s == "/") || (s == "//") || (s.size() == 3 && s[1] == ':' && s[2] == '/');
    if (!isRootForm && !s.empty() && s.back() == '/') {
        s.pop_back();
    }
    return ll::file_utils::u8path(s);
}

std::string relative(std::string const& from, std::string const& to) {
    auto absFrom = absoluteNormalized(from);
    auto absTo   = absoluteNormalized(to);
    return toPOSIXPath(absTo.lexically_relative(absFrom));
}

std::string absolute(std::string const& path) { return toPOSIXPath(absoluteNormalized(path)); }

} // namespace

decltype(Modules::kScriptFileSystem) Modules::kScriptFileSystem = //
    jspp::binding::defClass<void>("fs")
        .func("exists", exists)
        .func("readFile", readFile)
        .func("writeFile", writeFile)
        .func("copy", copy)
        .func("move", move)
        .func(
            "mkdir",
            static_cast<bool (*)(std::string const&)>(&mkdir),
            static_cast<bool (*)(std::string const&, bool)>(&mkdir)
        )
        .func(
            "appendFile",
            static_cast<bool (*)(std::string const&, std::string const&)>(&appendFile),
            static_cast<bool (*)(std::string const&, std::string const&, bool)>(&appendFile)
        )
        .func("remove", remove)
        .func("isDirectory", isDirectory)
        .func("isFile", isFile)
        .build();

decltype(Modules::kScriptPath) Modules::kScriptPath = //
    jspp::binding::defClass<void>("path")
        .func("join", join)
        .func("basename", basename)
        .func("dirname", dirname)
        .func("extname", extname)
        .func("relative", relative)
        .func("absolute", absolute)
        .func("normalize", normalize)
        .build();

} // namespace land::scripting
