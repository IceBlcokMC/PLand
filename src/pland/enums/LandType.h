#pragma once
#include <cstdint>

namespace land {

enum class LandType : uint8_t {
    Ordinary = 0, // 普通领地(无父、无子)
    Parent   = 1, // 父领地(无父、有子)
    Mix      = 2, // 混合领地(有父、有子)
    Sub      = 3, // 子领地(有父、无子)
};

}