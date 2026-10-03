#pragma once
#include "pland/Global.h"
#include "pland/enums/LandOwnershipKind.h"
#include "pland/events/LandEventMixin.h"

#include <memory>
#include <utility>


namespace land::event {


/**
 * 领地归属类型变更事件
 *
 * NOTE: 领地主 UUID 未变化但归属类型变化（如 PendingMigration 转为 Ownerless）时只会触发本事件，
 *       OwnerChangedEvent 在这种情况下不会触发。
 */
class LandOwnershipChangedEvent final : public LandEventMixin<ll::event::Event> {
    LandOwnershipKind mOldKind;
    LandOwnershipKind mNewKind;

public:
    explicit LandOwnershipChangedEvent(std::shared_ptr<Land> land, LandOwnershipKind oldKind, LandOwnershipKind newKind)
    : LandEventMixin(std::move(land)),
      mOldKind(oldKind),
      mNewKind(newKind) {}

    LDNDAPI LandOwnershipKind oldKind() const;

    LDNDAPI LandOwnershipKind newKind() const;
};


} // namespace land::event