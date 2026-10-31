#include "InteropHub.h"
#include "pland/Global.h"

#include <ll/api/Expected.h>


namespace land::scripting {


TrackedExportEntry::TrackedExportEntry(jspp::Engine* engine, jspp::Local<jspp::Value> const& obj) : engine_(engine) {
    kind_ = obj.kind();
    object_.reset(obj);
    this->track(engine);
}
jspp::Engine*            TrackedExportEntry::engine() const { return engine_; }
jspp::ValueKind          TrackedExportEntry::kind() const { return kind_; }
jspp::Local<jspp::Value> TrackedExportEntry::object() const { return object_.get(); }
void                     TrackedExportEntry::invalidate() {
    object_.reset();
    engine_ = nullptr;
}
void TrackedExportEntry::onEngineDestroy() { invalidate(); }
std::shared_ptr<TrackedExportEntry>
TrackedExportEntry::create(jspp::Engine* engine, jspp::Local<jspp::Value> const& obj) {
    return std::make_shared<TrackedExportEntry>(engine, obj);
}

InteropHub::InteropHub() = default;
InteropHub::~InteropHub() { clearAll(); }

bool InteropHub::hasExport(Identifier const& identifier) const { return objects.contains(identifier); }

bool InteropHub::hasExport(jspp::Engine* engine, Identifier const& identifier) const {
    auto iter = exports.find(engine);
    if (iter == exports.end()) {
        return false;
    }
    return iter->second.contains(identifier);
}

ll::Expected<> InteropHub::registerExportObject(
    jspp::Engine&                   engine,
    Identifier const&               identifier,
    jspp::Local<jspp::Value> const& object
) {
    if (hasExport(&engine, identifier)) {
        return ll::makeStringError("The identifier '{}' is already in use"_tr(identifier));
    }
    if (hasExport(identifier)) {
        return ll::makeStringError("The identifier '{}' is already used by another engine"_tr(identifier));
    }
    exports[&engine].insert(identifier);
    objects.emplace(identifier, TrackedExportEntry::create(&engine, object));
    return {};
}

bool InteropHub::unregisterExportObject(jspp::Engine& engine, Identifier const& identifier) {
    auto iter = exports.find(&engine);
    if (iter == exports.end()) {
        return false;
    }
    iter->second.erase(identifier);

    if (iter->second.empty()) {
        exports.erase(iter);
    }

    auto iter2 = objects.find(identifier);
    if (iter2 != objects.end()) {
        iter2->second->invalidate(); // explicitly invalidate the object
    }
    objects.erase(iter2);
    return true;
}

TrackedExportEntryPtr InteropHub::getExportObject(Identifier const& identifier) {
    auto iter = objects.find(identifier);
    if (iter == objects.end()) {
        return nullptr;
    }
    return iter->second;
}

bool InteropHub::clearExports(jspp::Engine* engine) {
    if (engine == nullptr) {
        return false;
    }
    auto iter = exports.find(engine);
    if (iter == exports.end()) {
        return false;
    }
    for (auto const& identifier : iter->second) {
        auto it2 = objects.find(identifier);
        if (it2 != objects.end()) {
            it2->second->invalidate(); // explicitly invalidate the object
        }
        objects.erase(it2);
    }
    exports.erase(iter);
    return true;
}

void InteropHub::clearAll() {
    objects.clear();
    exports.clear();
}

} // namespace land::scripting