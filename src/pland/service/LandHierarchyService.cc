#include "LandHierarchyService.h"

#include "pland/land/Land.h"
#include "pland/land/repo/LandRegistry.h"
#include "pland/land/repo/TransactionContext.h"
#include "pland/land/validator/LandCreateValidator.h"

namespace land {
namespace service {


struct LandHierarchyService::Impl {
    LandRegistry& mLandRegistry;
};

LandHierarchyService::LandHierarchyService(LandRegistry& registry) : impl(std::make_unique<Impl>(registry)) {}
LandHierarchyService::~LandHierarchyService() {}

#define UNEXPECTED_UNSUPPORTED_LEASED_ERR()                                                                            \
    ll::makeStringError(                                                                                               \
        "Leased land do not support adding any territorial hierarchy information; leasing and sub-land "               \
        "are mutually exclusive."                                                                                      \
    )

ll::Expected<> LandHierarchyService::ensureBoughtLand(std::shared_ptr<Land> const& land) {
    if (!land->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    return {};
}

ll::Expected<>
LandHierarchyService::attachSubLand(std::shared_ptr<Land> const& parent, std::shared_ptr<Land> const& sub) {
    if (!parent->isBought() || !sub->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    if (!sub->isOrdinaryLand()) {
        // 禁止两个领地家族整合
        return ll::makeStringError("The land type does not match the required type");
    }
    if (auto expected = ensureSubLandLegal(parent, sub); !expected) {
        return expected;
    }
    auto result = impl->mLandRegistry.executeTransaction({parent, sub}, [&](TransactionContext& ctx) -> bool {
        if (sub->getId() == INVALID_LAND_ID) {
            ctx.allocateId(sub); // 分配ID
        }
        parent->_getContext().mSubLandIDs.push_back(sub->getId());
        sub->_getContext().mParentLandID = parent->getId();

        parent->markDirty();
        sub->markDirty();
        return true;
    });
    if (result) {
        // parent 可能位于任意层级，但parent下新建的 sub 领地，这个新领地没有子节点，直接进行 +1
        sub->_setCachedNestedLevel(parent->getNestedLevel() + 1);
    }
    return result;
}

ll::Expected<> LandHierarchyService::deleteSubLand(std::shared_ptr<Land> const& sub) {
    if (!sub->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    if (!sub->isSubLand()) {
        return ll::makeStringError("The land type does not match the required type");
    }

    auto parent = getParent(sub);
    if (!parent) {
        return ll::makeStringError("The parent land is null");
    }

    // isSubLand 约束 sub 必须为最底层，那么 sub 后面没有任何节点，这里不需要更新层级缓存
    return impl->mLandRegistry.executeTransaction({parent, sub}, [&](TransactionContext& context) -> bool {
        std::erase_if(parent->_getContext().mSubLandIDs, [&](LandID id) { return id == sub->getId(); });
        parent->markDirty();
        context.markForRemoval(sub);
        return true;
    });
}

ll::Expected<> LandHierarchyService::deleteLandRecursive(std::shared_ptr<Land> const& land) {
    if (!land->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    if (!land->isParentLand() && !land->isMixLand()) {
        return ll::makeStringError("The land type does not match the required type");
    }
    std::unordered_set<std::shared_ptr<Land>> txnParticipants; // 事务参与者

    std::shared_ptr<Land> parent{nullptr}; // 当前领地的父节点
    if (land->hasParentLand()) {
        parent = getParent(land);
        txnParticipants.insert(parent);
    }

    auto removalTargets = getSelfAndDescendants(land);
    txnParticipants.insert(removalTargets.begin(), removalTargets.end());

    return impl->mLandRegistry.executeTransaction(txnParticipants, [&](TransactionContext& context) -> bool {
        // 擦除父领地中的记录
        if (parent) {
            std::erase_if(parent->_getContext().mSubLandIDs, [&](LandID const& id) { return id == land->getId(); });
            parent->markDirty();
        }
        // 联级移除子领地
        for (auto tar : removalTargets) {
            assert(tar != parent); // 父领地不应该被移除
            context.markForRemoval(tar);
        }
        return true;
    });
}

ll::Expected<> LandHierarchyService::deleteParentLandAndPromoteChildren(std::shared_ptr<Land> const& parent) {
    if (!parent->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    if (!parent->isParentLand()) {
        return ll::makeStringError("The land type does not match the required type");
    }
    std::unordered_set<std::shared_ptr<Land>> txnParticipants; // 事务参与者

    auto subs = getSubLands(parent);

    txnParticipants.insert(parent);
    txnParticipants.insert(subs.begin(), subs.end());

    auto result = impl->mLandRegistry.executeTransaction(txnParticipants, [&](TransactionContext& context) -> bool {
        for (auto& sub : subs) {
            sub->_getContext().mParentLandID = INVALID_LAND_ID;
            sub->markDirty();
        }
        context.markForRemoval(parent);
        return true;
    });
    if (result) {
        // 因 parent 是根节点，其 getNestedLevel 为 0
        // 而 parent 名下的子领地的层级为1，现在进行了提权，整个树 subs[i] 需要进行 -1 操作
        auto top = parent->getNestedLevel();
        for (auto& sub : subs) {
            updateLevels(sub, top);
        }
    }
    return result;
}

ll::Expected<> LandHierarchyService::deleteMixLandAndTransferChildren(std::shared_ptr<Land> const& mixLand) {
    if (!mixLand->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    if (!mixLand->isMixLand()) {
        return ll::makeStringError("The land type does not match the required type");
    }
    std::unordered_set<std::shared_ptr<Land>> txnParticipants; // 事务参与者

    auto parent = getParent(mixLand);
    auto subs   = getSubLands(mixLand);

    assert(parent != nullptr); // mixLand 必须有父节点

    txnParticipants.insert(parent);
    txnParticipants.insert(mixLand);
    txnParticipants.insert(subs.begin(), subs.end());

    auto result = impl->mLandRegistry.executeTransaction(txnParticipants, [&](TransactionContext& context) -> bool {
        std::erase_if(parent->_getContext().mSubLandIDs, [&](LandID const& id) { return id == mixLand->getId(); });
        context.markForRemoval(mixLand); // 移除 mixLand
        parent->markDirty();

        auto const parentId = parent->getId();
        for (auto& sub : subs) {
            sub->_getContext().mParentLandID = parentId;
            parent->_getContext().mSubLandIDs.push_back(sub->getId()); // 添加子领地到父领地记录中
            sub->markDirty();
            parent->markDirty();
        }
        return true;
    });
    if (result) {
        // mixLand 是混合领地，在树中间，层级为 parent + 1
        // 现在 mixLand 被移除，其子领地层级为 minLand + 1，需要 -1 操作
        auto mix = mixLand->getNestedLevel();
        for (auto& sub : subs) {
            updateLevels(sub, mix);
        }
    }
    return result;
}

std::shared_ptr<Land> LandHierarchyService::getParent(std::shared_ptr<Land> const& land) const {
    if (land->isParentLand() || !land->hasParentLand()) {
        return nullptr;
    }
    return impl->mLandRegistry.getLand(land->getParentLandID());
}
std::vector<std::shared_ptr<Land>> LandHierarchyService::getSubLands(std::shared_ptr<Land> const& land) const {
    if (!land->hasSubLand()) {
        return {};
    }
    return impl->mLandRegistry.getLands(land->getSubLandIDs());
}
int LandHierarchyService::getNestedLevel(std::shared_ptr<Land> const& land) const {
    if (!land->hasParentLand()) {
        return 0;
    }

    std::stack<std::shared_ptr<Land>> stack;
    stack.push(getParent(land));
    int level = 0;
    while (!stack.empty()) {
        auto curr = stack.top();
        stack.pop();
        level++;
        if (curr->hasParentLand()) {
            stack.push(getParent(curr));
        }
    }
    return level;
}
void LandHierarchyService::updateLevels(std::shared_ptr<Land> const& root, int rootLevel) {
    std::stack<std::pair<std::shared_ptr<Land>, int>> stack;
    stack.push({root, rootLevel});

    while (!stack.empty()) {
        auto [curr, level] = stack.top();
        stack.pop();

        curr->_setCachedNestedLevel(level);

        if (curr->hasSubLand()) {
            for (auto& child : getSubLands(curr)) {
                stack.push({child, level + 1});
            }
        }
    }
}


std::shared_ptr<Land> LandHierarchyService::getRoot(std::shared_ptr<Land> const& land) const {
    if (!land->hasParentLand()) {
        return land;
    }
    std::shared_ptr<Land> cur = land;
    while (auto parent = getParent(cur)) {
        cur = parent;
    }
    return cur;
}

std::unordered_set<std::shared_ptr<Land>> LandHierarchyService::getFamilyTree(std::shared_ptr<Land> const& land) const {
    auto root = getRoot(land);
    return getSelfAndDescendants(root);
}


ll::coro::Generator<std::shared_ptr<Land>>
LandHierarchyService::getDescendants(std::shared_ptr<Land> const& land) const {
    std::stack<std::shared_ptr<Land>> stack;
    stack.push(land);
    while (!stack.empty()) {
        auto current = stack.top();
        stack.pop();
        co_yield current;
        if (current->hasSubLand()) {
            for (auto& ptr : getSubLands(current)) {
                stack.push(ptr);
            }
        }
    }
}
ll::coro::Generator<std::shared_ptr<Land>> LandHierarchyService::getAncestors(std::shared_ptr<Land> const& land) const {
    std::stack<std::shared_ptr<Land>> stack;
    stack.push(land);
    while (!stack.empty()) {
        auto cur = stack.top();
        stack.pop();
        co_yield cur;
        if (cur->hasParentLand()) {
            stack.push(getParent(cur));
        }
    }
}

std::unordered_set<std::shared_ptr<Land>>
LandHierarchyService::getSelfAndAncestors(std::shared_ptr<Land> const& land) const {
    std::unordered_set<std::shared_ptr<Land>> parentLands;
    for (auto ptr : getAncestors(land)) {
        parentLands.insert(ptr);
    }
    return parentLands;
}

std::unordered_set<std::shared_ptr<Land>>
LandHierarchyService::getSelfAndDescendants(std::shared_ptr<Land> const& land) const {
    std::unordered_set<std::shared_ptr<Land>> descendants;
    for (auto ptr : getDescendants(land)) {
        descendants.insert(ptr);
    }
    return descendants;
}

ll::Expected<>
LandHierarchyService::ensureSubLandLegal(std::shared_ptr<Land> const& parent, std::shared_ptr<Land> const& sub) {
    if (!parent->isBought() || !sub->isBought()) {
        return UNEXPECTED_UNSUPPORTED_LEASED_ERR();
    }
    if (auto expected = LandCreateValidator::ensureLandRangeIsLegal(sub->getAABB(), sub->getDimensionId(), true); !expected) {
        return expected;
    }
    if (auto expected = LandCreateValidator::ensureSubLandPositionIsLegal(*this, parent, sub->getAABB()); !expected) {
        return expected;
    }
    if (parent->getDimensionId() != sub->getDimensionId()) {
        return ll::makeStringError("Sub land must be in the same dimension as parent land"_tr());
    }
    return {};
}

#undef UNEXPECTED_UNSUPPORTED_LEASED_ERR

} // namespace service
} // namespace land