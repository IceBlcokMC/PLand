#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "ll/api/event/EventBus.h"
#include <ll/api/mod/Mod.h>

#include "pland/PLand.h"
#include "pland/land/Land.h"
#include "pland/reflect/TypeName.h"
#include "pland/scripting/community/CommunityScriptPlugin.h"

#include "pland/events/domain/ConfigReloadEvent.h"
#include "pland/events/domain/LandOwnershipChangedEvent.h"
#include "pland/events/domain/LandRecycleEvent.h"
#include "pland/events/domain/LandResizedEvent.h"
#include "pland/events/domain/LandStateChangedEvent.h"
#include "pland/events/domain/MemberChangedEvent.h"
#include "pland/events/domain/OwnerChangedEvent.h"


namespace land::scripting {

namespace {

using namespace jspp;

/// 脚本可监听的事件
using ListenableEvent = std::tuple<
    event::ConfigReloadEvent,
    event::LandRecycleEvent,
    event::LandResizedEvent,
    event::LandStateChangedEvent,
    event::MemberChangedEvent,
    event::MembersClearedEvent,
    event::OwnerChangedEvent,
    event::LandOwnershipChangedEvent>;

struct ScriptEventBusImpl {
    ScriptEventBusImpl() = delete;

private:
    /// 将脚本事件回调封装为底层事件监听器
    template <std::derived_from<ll::event::Event> T>
    static std::shared_ptr<ll::event::Listener<T>>
    wrapScriptListener(Local<Function> const& cb, ll::event::EventPriority p, std::shared_ptr<ll::mod::Mod>& mod) {
        auto handler = binding::adapter::wrapScriptCallback</* no-return */ void, /* event */ T&>(cb);
        return ll::event::Listener<T>::create(handler, p, mod);
    }

    /// 添加一个事件监听器
    template <std::derived_from<ll::event::Event> T>
    static std::optional<ll::event::ListenerId>
    addListenerImpl(Engine* engine, Local<Function> const& callback, ll::event::EventPriority priority) {
        auto mod = [&]() -> std::shared_ptr<ll::mod::Mod> {
            auto data = engine->getData<EngineOwnData>();
            if (data->isPrivateEngine) {
                return PLand::getInstance().getSelfShared();
            }
            return data->communityScriptPlugin;
        }();

        auto listener = wrapScriptListener<T>(callback, priority, mod);
        if (!ll::event::EventBus::getInstance().addListener<T>(listener)) {
            return std::nullopt;
        }

        auto data = engine->getData<EngineOwnData>();
        assert(data);
        data->eventListeners.push_back(listener->getId());
        return listener->getId();
    }

    template <std::derived_from<ll::event::Event>... Ts>
        requires(sizeof...(Ts) > 0)
    static std::optional<ll::event::ListenerId> dispatchEventImpl(
        std::string_view         eventName,
        Local<Function> const&   callback,
        ll::event::EventPriority priority,
        Engine*                  engine
    ) {
        using land::reflect::getTypeTemplateInnerLeafName;
        using ll::hash_utils::doHash;

        std::optional<ll::event::ListenerId> id = std::nullopt;

        auto const targetHash = doHash(eventName);

        bool matched =
            ((targetHash == doHash(getTypeTemplateInnerLeafName<Ts>())
                  ? (id = addListenerImpl<Ts>(engine, callback, priority), true)
                  : false)
             || ...);

        if (!matched) [[unlikely]] {
            throw Exception{fmt::format("EventBus: Unknown event type {}", eventName), ExceptionType::TypeError};
        }

        return id;
    }

    static std::optional<ll::event::ListenerId> dispatchEvent(
        std::string_view         eventName,
        Local<Function> const&   callback,
        ll::event::EventPriority priority,
        Engine*                  engine
    ) {
        return [&]<typename... Ts>(std::tuple<Ts...>*) {
            return dispatchEventImpl<Ts...>(eventName, callback, priority, engine);
        }(static_cast<ListenableEvent*>(nullptr));
    }

public:
    static Local<Value> subscribe(Arguments const& args) {
        // subscribe(event, callback, priority)
        auto argc = args.length();
        if (argc < 2) {
            throw Exception{"eventBus.subscribe requires at least 2 arguments!", ExceptionType::TypeError};
        }

        auto event     = binding::toCpp<std::string>(args[0]);
        auto scallback = args[1].asFunction();

        auto priority = ll::event::EventPriority::Normal;
        if (argc == 3) {
            priority = binding::toCpp<ll::event::EventPriority>(args[2]);
        }

        auto id = dispatchEvent(event, scallback, priority, args.engine());
        return binding::toJs(id);
    }
    static Local<Value> unsubscribe(Arguments const& args) {
        // unsubscribe(id)
        if (args.length() < 1) [[unlikely]] {
            throw Exception{"eventBus.unsubscribe requires at least 1 argument!", ExceptionType::TypeError};
        }

        auto id = binding::toCpp<ll::event::ListenerId>(args[0]);

        auto data = args.engine()->getData<EngineOwnData>();
        assert(data);
        std::erase(data->eventListeners, id);
        return binding::toJs(ll::event::EventBus::getInstance().removeListener(id));
    }
};

} // namespace

// impl @levilamina -------------------------------------------------------------------

decltype(Modules::kScriptEventBus) Modules::kScriptEventBus = //
    jspp::binding::defClass<void>("eventBus")
        .func("subscribe", &ScriptEventBusImpl::subscribe)
        .func("unsubscribe", &ScriptEventBusImpl::unsubscribe)
        .build();

decltype(Modules::kScriptEventPriority) Modules::kScriptEventPriority = //
    jspp::binding::defEnum<ll::event::EventPriority>("EventPriority")
        .value("Highest", ll::event::EventPriority::Highest)
        .value("High", ll::event::EventPriority::High)
        .value("Normal", ll::event::EventPriority::Normal)
        .value("Low", ll::event::EventPriority::Low)
        .value("Lowest", ll::event::EventPriority::Lowest)
        .build();

decltype(Modules::kScriptEvent) Modules::kScriptEvent = //
    jspp::binding::defClass<ll::event::Event>("Event")
        .ctor(nullptr)
        .func("getId", [](ll::event::Event& self) -> std::string { return self.getId().name; })
        .build();


// impl @pland -------------------------------------------------------------------

decltype(Modules::kScriptConfigReloadEvent) Modules::kScriptConfigReloadEvent = //
    jspp::binding::defClass<event::ConfigReloadEvent>("ConfigReloadEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .build();

decltype(Modules::kScriptLandRecycleEvent) Modules::kScriptLandRecycleEvent = //
    jspp::binding::defClass<event::LandRecycleEvent>("LandRecycleEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::LandRecycleEvent::land)
        .prop_readonly("reason", &event::LandRecycleEvent::reason)
        .build();

decltype(Modules::kScriptLandResizedEvent) Modules::kScriptLandResizedEvent = //
    jspp::binding::defClass<event::LandResizedEvent>("LandResizedEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::LandResizedEvent::land)
        .prop_readonly("newRange", &event::LandResizedEvent::newRange)
        .build();

decltype(Modules::kScriptLandStateChangedEvent) Modules::kScriptLandStateChangedEvent = //
    jspp::binding::defClass<event::LandStateChangedEvent>("LandStateChangedEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::LandStateChangedEvent::land)
        .prop_readonly("oldState", &event::LandStateChangedEvent::oldState)
        .prop_readonly("newState", &event::LandStateChangedEvent::newState)
        .build();

decltype(Modules::kScriptMemberChangedEvent) Modules::kScriptMemberChangedEvent = //
    jspp::binding::defClass<event::MemberChangedEvent>("MemberChangedEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::MemberChangedEvent::land)
        .prop_readonly("target", &event::MemberChangedEvent::target)
        .prop_readonly("isAdd", &event::MemberChangedEvent::isAdd)
        .build();

decltype(Modules::kScriptMembersClearedEvent) Modules::kScriptMembersClearedEvent = //
    jspp::binding::defClass<event::MembersClearedEvent>("MembersClearedEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::MembersClearedEvent::land)
        .build();

decltype(Modules::kScriptOwnerChangedEvent) Modules::kScriptOwnerChangedEvent = //
    jspp::binding::defClass<event::OwnerChangedEvent>("OwnerChangedEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::OwnerChangedEvent::land)
        .prop_readonly("oldOwner", &event::OwnerChangedEvent::oldOwner)
        .prop_readonly("newOwner", &event::OwnerChangedEvent::newOwner)
        .build();

decltype(Modules::kScriptLandOwnershipChangedEvent) Modules::kScriptLandOwnershipChangedEvent = //
    jspp::binding::defClass<event::LandOwnershipChangedEvent>("LandOwnershipChangedEvent")
        .ctor(nullptr)
        .inherit<ll::event::Event>(kScriptEvent)
        .prop_readonly("land", &event::LandOwnershipChangedEvent::land)
        .prop_readonly("oldKind", &event::LandOwnershipChangedEvent::oldKind)
        .prop_readonly("newKind", &event::LandOwnershipChangedEvent::newKind)
        .build();


} // namespace land::scripting