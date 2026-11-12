#include "pland/scripting/EngineHelper.h"
#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "ll/api/event/EventBus.h"
#include <ll/api/Expected.h>
#include <ll/api/event/Cancellable.h>
#include <ll/api/mod/Mod.h>

#include <fmt/format.h>

#include "pland/events/Helper.h"
#include "pland/land/Land.h"
#include "pland/reflect/TypeName.h"

#include "pland/events/domain/ConfigReloadEvent.h"
#include "pland/events/domain/LandOwnershipChangedEvent.h"
#include "pland/events/domain/LandRecycleEvent.h"
#include "pland/events/domain/LandResizedEvent.h"
#include "pland/events/domain/LandStateChangedEvent.h"
#include "pland/events/domain/MemberChangedEvent.h"
#include "pland/events/domain/OwnerChangedEvent.h"


namespace land::scripting {

namespace channel {

// A: new ScriptEvent('type', { }) -> eventBus.emit(event) -> EventBus::publish(event) -> EventBus::Impl -|
//                                                                                                       /
// B: eventBus.subscribe('ScriptEvent', callback) -> EventBus::addListener(event, callback) ------------/
//                                          |                                       |
//    /---------- toJs(ScriptEvent&) <------/---------------------------------------/
//    |-> (ev: ScriptEvent)=>void -> call ev.payload -> native:structuredClone -> toJs.
//
// Global<T>::~Global() -> EngineScope{} -> reset
class ScriptEvent final : public ll::event::Cancellable<ll::event::Event>, public jspp::ITrackedHandle {
    std::string   mPubPluginName;             // the name of the plugin that published this event
    std::string   mType;                      // the type of the event
    jspp::Engine& mPubEngine;                 // the engine that published this event
    uint8_t*      mPubPayloadBuffer{nullptr}; // the payload of the event
    size_t        mPubPayloadSize{0};         // the size of the payload

    void cleanup() {
        helper::unsafe_StructuredCloneFreeBuffer(mPubEngine, mPubPayloadBuffer); // free payload
        mPubPayloadBuffer = nullptr;
        mPubPayloadSize   = 0;
    }

    void onEngineDestroy() override { cleanup(); }

    struct BufferTransfer {
        uint8_t*      buffer{nullptr};
        size_t        size{0};
        jspp::Engine& engine;

        BufferTransfer(BufferTransfer const&)            = delete;
        BufferTransfer& operator=(BufferTransfer const&) = delete;

        inline void release() {
            buffer = nullptr;
            size   = 0;
        }

        BufferTransfer(BufferTransfer&& mv) noexcept : buffer(mv.buffer), size(mv.size), engine(mv.engine) {
            mv.release();
        }
        BufferTransfer& operator=(BufferTransfer&&) = delete;

        explicit BufferTransfer(uint8_t* buffer, size_t size, jspp::Engine& engine)
        : buffer(buffer),
          size(size),
          engine(engine) {}
        ~BufferTransfer() {
            if (buffer) {
                helper::unsafe_StructuredCloneFreeBuffer(engine, buffer);
            }
        }
    };

public:
    ScriptEvent(ScriptEvent const&)            = delete;
    ScriptEvent(ScriptEvent&&)                 = delete;
    ScriptEvent& operator=(ScriptEvent const&) = delete;
    ScriptEvent& operator=(ScriptEvent&&)      = delete;

    explicit ScriptEvent(std::string type, BufferTransfer&& trans)
    : mType(std::move(type)),
      mPubEngine(trans.engine),
      mPubPayloadBuffer(trans.buffer),
      mPubPayloadSize(trans.size) {
        trans.release(); // release buffer ownership

        auto data      = mPubEngine.getData<EngineOwnData>();
        mPubPluginName = data->pluginInst->getName();

        this->track(&mPubEngine); // track engine to free payload when engine is destroyed
    }
    ~ScriptEvent() override { cleanup(); }

    std::string const& pluginName() const { return mPubPluginName; }

    std::string const& type() const { return mType; }

    static jspp::Local<jspp::Value> data(jspp::InstancePayload& payload, jspp::Arguments const& args) {
        auto self   = payload.unwrap<ScriptEvent>();
        auto engine = args.engine(); // receiver engine

        if (!self->mPubPayloadBuffer) {
            return jspp::Null::newNull(); // 事件创建方引擎已销毁，返回空
        }

        jspp::Local<jspp::Value> val;
        helper::unsafe_StructuredCloneBufferToValue(
            self->mPubEngine,
            self->mPubPayloadBuffer,
            self->mPubPayloadSize,
            *engine,
            val,
            false
        );
        return val;
    }

    static jspp::Local<jspp::Value> updateData(jspp::InstancePayload& payload, jspp::Arguments const& args) {
        auto self   = payload.unwrap<ScriptEvent>();
        auto engine = args.engine();

        if (engine != &self->mPubEngine) [[unlikely]] {
            throw jspp::Exception("Only the publisher can update event data");
        }

        if (args.length() != 1) [[unlikely]] {
            throw jspp::Exception("updateData requires 1 argument");
        }

        auto   newPayload = args[0];
        size_t len{0};
        auto   buf = helper::unsafe_StructuredCloneToBuffer(*engine, newPayload, len);
        if (!buf) [[unlikely]] {
            throw jspp::Exception{"Failed to clone payload"};
        }

        if (self->mPubPayloadBuffer) {
            helper::unsafe_StructuredCloneFreeBuffer(self->mPubEngine, self->mPubPayloadBuffer);
        }

        self->mPubPayloadBuffer = buf;
        self->mPubPayloadSize   = len;
        return {};
    }

    // new ScriptEvent('type', { })
    static std::unique_ptr<jspp::NativeInstance> ctor(jspp::Arguments const& args) {
        auto argc = args.length();
        if (argc != 2) {
            throw jspp::Exception("ScriptEvent constructor requires 2 arguments");
        }
        auto type    = jspp::binding::toCpp<std::string>(args[0]);
        auto payload = args[1];
        auto engine  = args.engine();

        size_t len{0};
        auto   buf = helper::unsafe_StructuredCloneToBuffer(*engine, payload, len);
        if (!buf) [[unlikely]] {
            throw jspp::Exception{"Failed to clone payload"};
        }

        auto trans = BufferTransfer(buf, len, *engine);
        return jspp::binding::factory::newNativeInstanceFor<ScriptEvent>(type, std::move(trans));
    }
};

static_assert(jspp::binding::traits::isConstructorCallback_v<decltype(&ScriptEvent::ctor)>);
static_assert(jspp::binding::traits::isInstanceGetterCallback_v<decltype(&ScriptEvent::data)>);
static_assert(jspp::binding::traits::isInstanceGetterCallback_v<decltype(&ScriptEvent::updateData)>);

IMPLEMENT_EVENT_EMITTER(ScriptEvent);

} // namespace channel

namespace {

using namespace jspp;

/// 脚本可监听的事件
using ListenableEvent = std::tuple<
    channel::ScriptEvent,
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
        auto data = engine->getData<EngineOwnData>();
        auto mod  = data->pluginInst;

        auto listener = wrapScriptListener<T>(callback, priority, mod);
        if (!ll::event::EventBus::getInstance().addListener<T>(listener)) {
            return std::nullopt;
        }
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

    static void emit(ll::event::Event& ev) {
        ll::event::EventBus::getInstance().publish(ev, ev.getId()); //
    }
    static void emitTo(ll::event::Event& ev, std::string_view modName) {
        ll::event::EventBus::getInstance().publish(modName, ev, ev.getId());
    }
};

} // namespace

// impl @levilamina -------------------------------------------------------------------

decltype(Modules::kScriptEventBus) Modules::kScriptEventBus = //
    jspp::binding::defClass<void>("eventBus")
        .func("subscribe", &ScriptEventBusImpl::subscribe)
        .func("unsubscribe", &ScriptEventBusImpl::unsubscribe)
        .func("emit", &ScriptEventBusImpl::emit)
        .func("emitTo", &ScriptEventBusImpl::emitTo)
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
        .method("getId", [](ll::event::Event& self) -> std::string { return self.getId().name; })
        .build();


// impl @pland -------------------------------------------------------------------

decltype(Modules::kScriptToScriptEvent) Modules::kScriptToScriptEvent = //
    jspp::binding::defClass<channel::ScriptEvent>("ScriptEvent")
        .inherit<ll::event::Event>(kScriptEvent)
        .ctor(channel::ScriptEvent::ctor)
        .prop_readonly("pluginName", &channel::ScriptEvent::pluginName)
        .prop_readonly("type", &channel::ScriptEvent::type)
        .prop_readonly("data", &channel::ScriptEvent::data)
        .method("updateData", &channel::ScriptEvent::updateData)

        .prop_readonly("isCancelled", &channel::ScriptEvent::isCancelled)
        .method("setCancelled", &channel::ScriptEvent::setCancelled)
        .method("cancel", &channel::ScriptEvent::cancel)
        .build();

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