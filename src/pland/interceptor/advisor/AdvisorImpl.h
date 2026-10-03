#pragma once
#include "AdvisorBase.h"

#include <memory>
#include <type_traits>
#include <utility>

namespace land::advisor {

template <typename F, typename Ev>
concept AdvisorFor =
    std::derived_from<Ev, ll::event::Event> && std::is_nothrow_move_constructible_v<F> && requires(F& f, Ev& e) {
        { f(e, std::declval<std::shared_ptr<Land> const&>()) } -> std::convertible_to<AdvisorResult>;
    };

template <std::derived_from<ll::event::Event> Ev, AdvisorFor<Ev> F>
class AdvisorImpl final : public AdvisorBase {
    F mHandler;

public:
    explicit AdvisorImpl(F handler, AdvisorPriority p = AdvisorPriority::Normal)
    : AdvisorBase(p),
      mHandler{std::move(handler)} {}

    AdvisorResult handle(ll::event::Event& event, std::shared_ptr<Land> const& land) override {
        return mHandler(static_cast<Ev&>(event), land);
    }
};

// template <std::derived_from<ll::event::Event> Ev, typename F>
//     requires AdvisorFor<std::decay_t<F>, Ev>
// std::unique_ptr<AdvisorBase> makeAdvisor(F&& f, AdvisorPriority p = AdvisorPriority::Normal) {
//     return std::make_unique<AdvisorImpl<Ev, std::decay_t<F>>>(std::forward<F>(f), p);
// }

} // namespace land::advisor