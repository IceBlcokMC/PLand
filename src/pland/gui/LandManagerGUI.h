#pragma once
#include "pland/Global.h"

class Player;
namespace mce {
class UUID;
}

namespace land {

class Land;

namespace gui {

class LandManagerGUI {
public:
    LandManagerGUI() = delete;

    LDAPI static void sendMainMenu(Player& player, std::shared_ptr<Land> land);

    LDAPI static void sendLeaseRenewGUI(Player& player, std::shared_ptr<Land> const& land);

    LDAPI static void confirmRenewDuration(Player& player, std::shared_ptr<Land> const& land, int days);

    LDAPI static void sendEditLandPermGUI(Player& player, std::shared_ptr<Land> const& ptr); // 编辑领地权限

    // 删除领地
    LDAPI static void showRemoveConfirm(Player& player, std::shared_ptr<Land> const& ptr);   // 删除领地确认
    LDAPI static void confirmSimpleDelete(Player& player, std::shared_ptr<Land> const& ptr); // 删除普通或子领地
    LDAPI static void confirmParentDelete(Player& player, std::shared_ptr<Land> const& ptr); // 删除父领地
    LDAPI static void confirmMixDelete(Player& player, std::shared_ptr<Land> const& ptr);    // 删除混合领地

    LDAPI static void sendEditLandNameGUI(Player& player, std::shared_ptr<Land> const& ptr); // 编辑领地名称
    LDAPI static void sendTransferLandGUI(Player& player, std::shared_ptr<Land> const& ptr); // 转让领地
    LDAPI static void
    _sendTransferLandToOnlinePlayer(Player& player, std::shared_ptr<Land> const& ptr); // 转让领地给在线玩家
    LDAPI static void
    _sendTransferLandToOfflinePlayer(Player& player, std::shared_ptr<Land> const& ptr); // 转让领地给离线玩家
    LDAPI static void _confirmTransferLand( // 确认转让领地
        Player&           player,
        std::shared_ptr<Land> const& ptr,
        mce::UUID         target,
        std::string       displayName
    );
    LDAPI static void sendCreateSubLandConfirm(Player& player, std::shared_ptr<Land> const& ptr); // 创建子领地确认
    LDAPI static void sendChangeRangeConfirm(Player& player, std::shared_ptr<Land> const& ptr);   // 更改领地范围

    LDAPI static void sendChangeMember(Player& player, std::shared_ptr<Land> ptr);      // 更改成员
    LDAPI static void _sendAddOnlineMember(Player& player, std::shared_ptr<Land> ptr);  // 添加在线成员
    LDAPI static void _sendAddOfflineMember(Player& player, std::shared_ptr<Land> ptr); // 添加离线成员
    LDAPI static void
    _confirmAddMember(Player& player, std::shared_ptr<Land> ptr, mce::UUID member, std::string displayName); // 添加成员
    LDAPI static void _confirmRemoveMember(Player& player, std::shared_ptr<Land> ptr, mce::UUID members);    // 移除成员
};

} // namespace gui
} // namespace land
