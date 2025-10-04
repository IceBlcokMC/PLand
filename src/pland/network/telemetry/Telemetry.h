#pragma once
#include "pland/Global.h"

#include <memory>

namespace land {

class PLand;

namespace network {

class Telemetry final {
    struct Impl;
    std::unique_ptr<Impl> mImpl;

public:
    LD_DISABLE_COPY_AND_MOVE(Telemetry);

    explicit Telemetry(PLand* pland);
    ~Telemetry();
};

} // namespace network

} // namespace land
