#pragma once
#include "ll/api/event/Emitter.h"

#define IMPLEMENT_EVENT_EMITTER(EVENT_CLASS)                                                                           \
    static std::unique_ptr<ll::event::EmitterBase> emitterFactory##EVENT_CLASS();                                      \
    class EVENT_CLASS##Emitter : public ll::event::Emitter<emitterFactory##EVENT_CLASS, EVENT_CLASS> {};               \
    static std::unique_ptr<ll::event::EmitterBase> emitterFactory##EVENT_CLASS() {                                     \
        return std::make_unique<EVENT_CLASS##Emitter>();                                                               \
    }