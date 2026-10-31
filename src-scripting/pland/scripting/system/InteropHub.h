#pragma once
#include "pland/scripting/pch.h"

#include <ll/api/Expected.h>


namespace land::scripting {

class TrackedExportEntry final : public jspp::ITrackedHandle {
public:
    explicit TrackedExportEntry(jspp::Engine* engine, jspp::Local<jspp::Value> const& obj);

    jspp::Engine* engine() const;

    jspp::ValueKind kind() const;

    jspp::Local<jspp::Value> object() const;

    void invalidate();

    static std::shared_ptr<TrackedExportEntry> create(jspp::Engine* engine, jspp::Local<jspp::Value> const& obj);

private:
    void onEngineDestroy() override;

    jspp::Engine*             engine_{nullptr};
    jspp::ValueKind           kind_{jspp::ValueKind::kUndefined};
    jspp::Global<jspp::Value> object_{};
};
using TrackedExportEntryPtr = std::shared_ptr<TrackedExportEntry>;

class InteropHub {
public:
    using Identifier = std::string;

    ~InteropHub();
    InteropHub(InteropHub const&)            = delete;
    InteropHub& operator=(InteropHub const&) = delete;
    InteropHub(InteropHub&&)                 = delete;
    InteropHub& operator=(InteropHub&&)      = delete;

    [[nodiscard]] bool hasExport(Identifier const& identifier) const;                       // global
    [[nodiscard]] bool hasExport(jspp::Engine* engine, Identifier const& identifier) const; // engine-specific

    [[nodiscard]] ll::Expected<>
    registerExportObject(jspp::Engine& engine, Identifier const& identifier, jspp::Local<jspp::Value> const& object);

    bool unregisterExportObject(jspp::Engine& engine, Identifier const& identifier);

    [[nodiscard]] TrackedExportEntryPtr getExportObject(Identifier const& identifier);

    bool clearExports(jspp::Engine* engine);

    void clearAll();

private:
    InteropHub();

    friend class ScriptingBootstrap;

    absl::flat_hash_map<jspp::Engine*, absl::flat_hash_set<std::string>> exports;
    absl::flat_hash_map<Identifier, TrackedExportEntryPtr>               objects;
};


} // namespace land::scripting