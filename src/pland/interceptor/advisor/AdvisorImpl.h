#pragma once
#include "AdvisorBase.h"

#include <memory>
#include <type_traits>
#include <utility>

namespace land::interceptor::advisor {

template <typename F, typename Ev>
concept AdvisorFor =
    std::derived_from<Ev, ll::event::Event> && std::is_nothrow_move_constructible_v<F> && requires(F& f, Ev const& e) {
        { f(e, std::declval<std::shared_ptr<Land> const&>()) } -> std::convertible_to<AdvisorResult>;
    };

template <std::derived_from<ll::event::Event> Ev, AdvisorFor<Ev> F>
class AdvisorImpl final : public AdvisorBase {
public:
    using event_type = Ev;

    explicit AdvisorImpl(F handler, AdvisorPriority p = AdvisorPriority::Normal)
    : AdvisorBase(p),
      mHandler{std::move(handler)} {}

    ~AdvisorImpl() override = default;

    AdvisorResult handle(ll::event::Event const& event, std::shared_ptr<Land> const& land) override {
        return mHandler(static_cast<event_type const&>(event), land);
    }

    [[nodiscard]] static std::shared_ptr<AdvisorImpl> make(F handler, AdvisorPriority p = AdvisorPriority::Normal) {
        return std::make_shared<AdvisorImpl>(std::move(handler), p);
    }

private:
    F mHandler;
};

} // namespace land::interceptor::advisor