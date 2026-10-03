#pragma once
#include <iostream>
#include <source_location>
#include <sstream>
#include <string_view>

#include <fmt/base.h>
#include <fmt/format.h>

namespace land::internal::interceptor {


struct EventTrace {
    std::ostringstream oss;
    int                depth = 0;

    explicit EventTrace(std::string_view eventName, std::source_location loc = std::source_location::current()) {
        EventTrace::current = this;
        oss << "[EventTrace] " << eventName << "\n";
        oss << "  [File] " << loc.file_name() << ":" << loc.line() << "\n";
    }

    ~EventTrace() {
        EventTrace::current = nullptr;
        std::cout << oss.str() << std::endl;
    }

    void indent() {
        for (int i = 0; i < depth; ++i) oss << "  ";
    }

    template <typename... Args>
    void log(std::string_view fmt, Args&&... args) {
        indent();
        oss << "  [Log] " << fmt::vformat(fmt, fmt::make_format_args(args...)) << "\n";
    }

    inline static thread_local EventTrace* current = nullptr;
};


struct EventTraceScope {
    EventTrace* trace;
    EventTraceScope(std::string_view name) : trace(EventTrace::current) {
        if (trace) {
            ++trace->depth;
            trace->indent();
            trace->oss << "[Scope] " << name << " {\n";
        }
    }
    ~EventTraceScope() {
        if (trace) {
            trace->indent();
            --trace->depth;
            trace->oss << "}\n";
        }
    }
};

#ifdef PLAND_DEBUG

#define TRACE_THIS_EVENT(EVENT) EventTrace __EventTrace_Stack__(#EVENT)
#define TRACE_LOG(...)                                                                                                 \
    if (EventTrace::current) EventTrace::current->log(__VA_ARGS__)

#define TRACE_ADD_SCOPE(...)                                                                                           \
    EventTraceScope __trace_scope_##__LINE__ { __VA_ARGS__ }

#else

#define TRACE_THIS_EVENT(EVENT)
#define TRACE_LOG(...)
#define TRACE_ADD_SCOPE(...)

#endif

} // namespace land::internal::interceptor
