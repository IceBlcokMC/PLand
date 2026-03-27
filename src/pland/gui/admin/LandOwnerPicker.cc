#include "LandOwnerPicker.h"

#include "pland/gui/common/PaginatedForm.h"
#include "pland/gui/utils/BackUtils.h"
#include "pland/land/Land.h"

#include "ll/api/service/PlayerInfo.h"
#include "ll/api/utils/StringUtils.h"
#include "pland/PLand.h"
#include "pland/gui/common/SimpleInputForm.h"
#include "pland/land/repo/LandRegistry.h"

namespace land {
namespace gui {

struct LandOwnerPicker::Impl : std::enable_shared_from_this<Impl> {
    struct Entry {
        mce::UUID const   mOwner;
        std::string const mLowerName;
        std::string const mDisplayName;
        size_t const      mLandCount;
        explicit Entry(mce::UUID const& owner, std::string const& displayName, size_t landCount)
        : mOwner(owner),
          mLowerName(ll::string_utils::toLowerCase(displayName)),
          mDisplayName(displayName),
          mLandCount(landCount) {}
    };
    std::vector<Entry>                   mEntries;
    std::optional<std::string>           mFuzzyKeyword;
    Callback                             mCallback;
    ll::form::SimpleForm::ButtonCallback mBackTo;

    void _collectEntries() {
        auto& info  = ll::service::PlayerInfo::getInstance();
        auto  lands = PLand::getInstance().getLandRegistry().getLandsByOwner();
        mEntries.reserve(lands.size());
        for (auto const& [owner, landSet] : lands) {
            if (owner == SYSTEM_ACCOUNT_UUID) {
                mEntries.emplace_back(owner, "PLandSystem", landSet.size());
            } else {
                auto entry = info.fromUuid(owner);
                mEntries.emplace_back(owner, entry ? entry->name : owner.asString(), landSet.size());
            }
        }
    }

    void sendSearchForm(Player& player) {
        auto localeCode = player.getLocaleCode();
        SimpleInputForm::sendTo(
            player,
            "[PLand] | 模糊搜索"_trl(localeCode),
            "请输入玩家名"_trl(localeCode),
            mFuzzyKeyword.value_or(""),
            [data = shared_from_this()](Player& player, std::string keyword) {
                if (keyword.empty()) {
                    data->mFuzzyKeyword = std::nullopt;
                } else {
                    data->mFuzzyKeyword = ll::string_utils::toLowerCase(keyword);
                }
                data->sendTo(player);
            }
        );
    }

    void buildForm(PaginatedForm& form, std::string const& localeCode) {
        form.setTitle("[PLand] | 领地主人选择器"_trl(localeCode));
        form.setContent("请选择您要管理的玩家"_trl(localeCode));
        if (mBackTo) {
            back_utils::injectBackButton(form, mBackTo);
        }
        form.appendButton(
            "模糊搜索"_trl(localeCode),
            "textures/ui/magnifyingGlass",
            "path",
            [data = shared_from_this()](Player& player) { data->sendSearchForm(player); }
        );
        for (auto const& entry : mEntries) {
            if (mFuzzyKeyword && entry.mLowerName.find(*mFuzzyKeyword) == std::string::npos) {
                continue;
            }
            form.appendButton(
                "{} ({} 个领地)"_trl(localeCode, entry.mDisplayName, entry.mLandCount),
                [data = shared_from_this(), uuid = entry.mOwner](Player& player) { data->mCallback(player, uuid); }
            );
        }
    }

    void sendTo(Player& player) {
        auto localeCode = player.getLocaleCode();
        auto fm         = PaginatedForm{};
        buildForm(fm, localeCode);
        fm.sendTo(player);
    }

    explicit Impl(Callback callback, ll::form::SimpleForm::ButtonCallback backTo)
    : mCallback(std::move(callback)),
      mBackTo(std::move(backTo)) {
        _collectEntries();
    }
};

void LandOwnerPicker::sendTo(Player& player, Callback callback, ll::form::SimpleForm::ButtonCallback backTo) {
    auto impl = std::make_shared<Impl>(std::move(callback), std::move(backTo));
    impl->sendTo(player);
}


} // namespace gui
} // namespace land