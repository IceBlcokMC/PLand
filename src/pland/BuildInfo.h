#pragma once
#include <string_view>

#include "pland/_version.h" // auto generated

namespace land {

struct BuildInfo {
    BuildInfo() = delete;

    static constexpr std::string_view kBuildMode = PLAND_BUILD_MODE;

    static constexpr std::string_view kBuildCommit = PLAND_BUILD_GIT_COMMIT;
    static constexpr std::string_view kBuildBranch = PLAND_BUILD_GIT_BRANCH;
    static constexpr std::string_view kBuildTag    = PLAND_BUILD_GIT_TAG;

    static constexpr std::string_view kBuildVariant = PLAND_BUILD_VARIANT;
    static constexpr bool             kHeadless     = PLAND_BUILD_HEADLESS;

    static constexpr int kVersionMajor = PLAND_VERSION_MAJOR;
    static constexpr int kVersionMinor = PLAND_VERSION_MINOR;
    static constexpr int kVersionPatch = PLAND_VERSION_PATCH;

    // commit hash (e.g. "a1b2c3d4")
    [[deprecated("Use kBuildCommit instead")]] static constexpr std::string_view Commit = kBuildCommit;

    // build branch (e.g. "main")
    [[deprecated("Use kBuildBranch instead")]] static constexpr std::string_view Branch = kBuildBranch;

    // build tag (e.g. "v1.2.3")
    // v0.1.0-alpha.1-14-g61162a0 -> v0.1.0-alpha.1 is the tag
    // 14 is the tag distance
    // g61162a0 is the tag commit hash
    [[deprecated("Use kBuildTag instead")]] static constexpr std::string_view Tag = kBuildTag;
};


} // namespace land