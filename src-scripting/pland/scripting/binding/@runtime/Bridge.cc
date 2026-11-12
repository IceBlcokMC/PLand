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
    Local<Value> s_exception;
    bool         has_exception = false;

    {
        // ! Local<T> 句柄的析构函数和引擎作用域绑定，请在匹配的引擎上下文中使用/释放 Local<T> 句柄
        // source 为调用发起方, target 为调用目标方
        // 这里进入目标引擎的作用域, 并在作用域内完成参数处理并调用目标函数
        EngineScope enter_target{t_engine};

        Local<Value> t_ret; // 目标函数的返回值
        try {
            std::vector<Local<Value>> t_applyArgs;
            helper::unsafe_StructureCloneValues(s_args, t_engine, t_applyArgs);

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
            t_applyArgs.clear(); // 释放 Local<Value> 句柄
        } catch (Exception const& e) {
            has_exception = true;
            // QuickJs 依赖 JS_EXCEPTION 标志来进行异常处理。但目前，由于没有目标引擎的 JS 栈帧，
            // QJS 不会处理抛出的异常，因此这里直接输出，而不是抛出。
            PLand::getInstance().getSelf().getLogger().error(
                "An exception occurred while the bridge was calling the target: {}\n{}",
                e.what(),
                e.stacktrace()
            );
        }

        // 由于 t_ret 为目标引擎的 Local<T> 句柄，不能跨引擎传递，这里需要回到源引擎作用域并进行结构化克隆
        {
            EngineScope enter_source{s_engine};
            try {
                helper::unsafe_StructuredCloneValue(t_engine, t_ret, s_engine, s_ret);
            } catch (Exception const& e) {
                has_exception = true;
                s_exception   = e.exception(); // 这里的异常是源引擎的异常，可以向上抛出
            }
        }

        // 在目标引擎作用域，释放目标函数返回的 Local<T> 句柄
        t_ret.reset();
    }
    if (has_exception) [[unlikely]] {
        if (s_exception.isNullOrUndefined()) {
            // 目标引擎异常，异常消息已从日志输出，这里直接抛出调用失败中断脚本代码执行
            throw Exception("An exception occurred while the bridge was calling the target");
        } else {
            // 源引擎异常，将暂存的异常重新抛出，由上层处理转为脚本异常
            throw Exception(s_exception);
        }
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
                // ! Local<T> 句柄的析构函数和引擎作用域绑定，请在匹配的引擎上下文中使用/释放 Local<T> 句柄
                EngineScope lock{handle->engine()};
                try {
                    auto t_val      = handle->object();
                    auto t_obj      = t_val.asObject();
                    auto t_property = String::newString(property);
                    exitis          = t_obj.has(t_property);
                    is_func         = t_obj.get(t_property).isFunction();
                } catch (Exception const& e) {
                    has_exception = true;

                    // QuickJs 依赖 JS_EXCEPTION 标志来进行异常处理。但目前，由于没有目标引擎的 JS 栈帧，
                    // QJS 不会处理抛出的异常，因此这里直接输出，而不是抛出。
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