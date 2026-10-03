#pragma once

namespace land::infra {

template <typename... Ts>
struct overload_t : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
overload_t(Ts...) -> overload_t<Ts...>;

} // namespace land::infra