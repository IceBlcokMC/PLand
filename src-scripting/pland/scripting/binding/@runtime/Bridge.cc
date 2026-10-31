#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "pland/PLand.h"
#include "pland/scripting/ScriptingBootstrap.h"
#include "pland/scripting/system/InteropHub.h"


namespace land::scripting {

namespace {

using namespace jspp;
using namespace jspp::binding;

InteropHub& getInteropHub() { return PLand::getInstance().getScriptingBootstrap().getInteropHub(); }

bool has(std::string const& identifier) { return getInteropHub().hasExport(identifier); }

Local<Value> exports(Arguments const& args) {
    if (args.length() != 2) {
        throw Exception("exports expects 2 arguments");
    }

    auto exportObj = args[1];
    if (exportObj.isProxy() || (!exportObj.isFunction() && !exportObj.isObject())) [[unlikely]] {
        throw Exception("Unsupported type, the bridge only supports exporting class instances or functions");
    }

    auto identifier = toCpp<std::string>(args[0]);

    auto expected = getInteropHub().registerExportObject(*args.engine(), identifier, exportObj);
    if (!expected) {
        throw Exception{expected.error().message()};
    }
    return {};
}

Local<Value> unexport(Arguments const& args) {
    if (args.length() != 1) {
        throw Exception("unexport expects 1 argument");
    }
    auto identifier = toCpp<std::string>(args[0]);
    auto res        = getInteropHub().unregisterExportObject(*args.engine(), identifier);
    return toJs(res);
}

namespace unsafe {

void structuredClone(Engine& source, Local<Value> const& s_val, Engine& target, Local<Value>& t_val) {
    constexpr auto flags = JS_WRITE_OBJ_REFERENCE;

    size_t len   = 0;
    auto   s_buf = JS_WriteObject(source.context(), &len, qjs_backend::QjsHelper::peekValue(s_val), flags);
    if (!s_buf) [[unlikely]] {
        throw Exception("Failed to serialize value");
    }

    auto t_res = JS_ReadObject(target.context(), s_buf, len, flags);
    js_free(source.context(), s_buf);

    {
        EngineScope lock{target}; // cross-engine handle
        qjs_backend::QjsHelper::rethrowException(t_res);
        t_val = ValueHelper::wrap<Value>(t_res);
    }
}
void structureClones(Arguments const& s_args, Engine& target, std::vector<Local<Value>>& t_out) {
    auto s_len = s_args.length();
    if (s_len == 0) {
        return;
    }

    t_out.resize(s_len);

    auto& f_engine = *s_args.engine();
    for (size_t i = 0; i < s_len; ++i) {
        auto f_value = s_args[i];
        structuredClone(f_engine, f_value, target, t_out[i]);
    }
}

} // namespace unsafe

void ensureRemoteHandle(TrackedExportEntryPtr& handle) {
    if (handle->engine() == nullptr) {
        throw Exception(
            "Bridge call failed, the target class instance or function has been unexported",
            ExceptionType::ReferenceError
        );
    }
}

Local<Value> /* The return value belongs to the current engine context */
invokeRemoteFunc(Arguments const& s_args, TrackedExportEntryPtr& handle, std::string_view property = "") {
    auto& s_engine = *s_args.engine();
    auto& t_engine = *handle->engine();

    Local<Value> s_ret;
    bool         has_exception = false;

    {
        // !WARN: Local<T> handle destructor binds to the target engine context!!!
        // Pure value types are destructor-safe, and jspp won't release or access the scope internally.
        std::vector<Local<Value>> t_applyArgs; // target engine args
        unsafe::structureClones(s_args, t_engine, t_applyArgs);

        EngineScope lock{t_engine};
        try {
            Local<Value> t_ret;

            auto spanArgs   = std::span{t_applyArgs};
            auto t_handle_v = handle->object();
            if (property.empty()) {
                auto t_func = t_handle_v.asFunction();
                t_ret       = t_func.call({}, spanArgs);
            } else {
                auto t_obj      = t_handle_v.asObject();
                auto t_property = String::newString(property);
                auto t_fv       = t_obj.get(t_property);
                auto t_func     = t_fv.asFunction();
                t_ret           = t_func.call(t_obj, spanArgs);
            }
            unsafe::structuredClone(t_engine, t_ret, s_engine, s_ret);
        } catch (Exception const& e) {
            has_exception = true;
            // QuickJs relies on the JS_EXCEPTION flag for exception handling.
            // But currently, with no target engine JS stack frame, QJS won't
            //  handle thrown exceptions, so it just outputs them directly.
            PLand::getInstance().getSelf().getLogger().error(
                "An exception occurred while the bridge was calling the target: {}\n{}",
                e.what(),
                e.stacktrace()
            );
        }
        t_applyArgs.clear(); // free resources in target engine scope
    }
    if (has_exception) [[unlikely]] {
        throw Exception("An exception occurred while the bridge was calling the target");
    }
    return s_ret;
}
Local<Function> newFunctionWrapper(TrackedExportEntryPtr ptr) {
    return Function::newFunction([handle = std::move(ptr)](Arguments const& args) mutable -> Local<Value> {
        // bridge.imports("service") -> remote_func -> .call(args) -> invokeRemoteFunc
        ensureRemoteHandle(handle);
        return invokeRemoteFunc(args, handle);
    });
}
Local<Function> newRemoteMethodWrapper(std::string property, TrackedExportEntryPtr ptr) {
    return Function::newFunction(
        [property = std::move(property), handle = std::move(ptr)](Arguments const& args) mutable -> Local<Value> {
            // bridge.imports("service") -> Proxy Object -> .method -> get_trap -> remote_func -> .call(args) ->
            // invokeRemoteFunc
            ensureRemoteHandle(handle);
            return invokeRemoteFunc(args, handle, property);
        }
    );
}
Local<Proxy> newProxyObject(TrackedExportEntryPtr ptr) {
    auto get_remote_func_trap =
        Function::newFunction([handle = std::move(ptr)](Arguments const& args) mutable -> Local<Value> {
            // args[0]: target
            // args[1]: property
            // args[2]: receiver
            if (args.length() != 3) [[unlikely]] {
                throw Exception("get trap expects 3 arguments");
            }
            ensureRemoteHandle(handle);

            auto property = toCpp<std::string>(args[1]);

            bool exitis        = false;
            bool is_func       = false;
            bool has_exception = false;
            {
                EngineScope lock{handle->engine()};
                // !WARN: Local<T> handle destructor binds to the target engine context!!!
                try {
                    auto t_val      = handle->object();
                    auto t_obj      = t_val.asObject();
                    auto t_property = String::newString(property);
                    exitis          = t_obj.has(t_property);
                    is_func         = t_obj.get(t_property).isFunction();
                } catch (Exception const& e) {
                    has_exception = true;

                    // QuickJs relies on the JS_EXCEPTION flag for exception handling.
                    // But currently, with no target engine JS stack frame, QJS won't
                    //  handle thrown exceptions, so it just outputs them directly.
                    PLand::getInstance().getSelf().getLogger().error(
                        "An exception occurred while the bridge proxy object was checking the target object's property "
                        "{}: {}\n{}",
                        property,
                        e.what(),
                        e.stacktrace()
                    );
                }
            }
            if (has_exception) {
                throw Exception(
                    fmt::format(
                        "bridge call failed, an uncaught exception occurred while checking property {}",
                        property
                    )
                );
            }
            if (!exitis) {
                throw Exception(
                    fmt::format("bridge call failed, property '{}' does not exist on exported object", property)
                );
            }
            if (!is_func) {
                throw Exception(fmt::format("bridge call failed, property '{}' is not a function", property));
            }

            return newRemoteMethodWrapper(std::move(property), handle);
        });

    auto handlers = Object::newObject();
    handlers.set(String::newString("get"), get_remote_func_trap);

    auto target = Object::newObject(); // empty object
    auto p      = Proxy::newProxy(target, handlers);
    return p;
}

Local<Value> imports(Arguments const& args) {
    if (args.length() != 1) [[unlikely]] {
        throw Exception("imports expects 1 argument");
    }

    auto identifier = toCpp<std::string>(args[0]);

    auto& hub = getInteropHub();
    auto  ptr = hub.getExportObject(identifier);
    if (!ptr) {
        return Null::newNull();
    }

    switch (ptr->kind()) {
    case ValueKind::kObject:
        return newProxyObject(std::move(ptr));
    case ValueKind::kFunction:
        return newFunctionWrapper(std::move(ptr));
    default:
        [[unlikely]] throw Exception("Unsupported type");
    };
}


static_assert(traits::isFunctionCallback_v<decltype(exports)>);
static_assert(traits::isFunctionCallback_v<decltype(unexport)>);
static_assert(traits::isFunctionCallback_v<decltype(imports)>);

} // namespace

decltype(Modules::kScriptBridge) Modules::kScriptBridge = //
    jspp::binding::defClass<void>("bridge")
        .func("has", has)
        .func("exports", exports)
        .func("unexport", unexport)
        .func("imports", imports)
        .build();

} // namespace land::scripting