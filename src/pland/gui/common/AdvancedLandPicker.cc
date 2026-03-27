#include "AdvancedLandPicker.h"
#include "PaginatedForm.h"
#include "SimpleInputForm.h"
#include "pland/Global.h"
#include "pland/gui/utils/BackUtils.h"
#include "pland/land/Land.h"
#include "pland/utils/TimeUtils.h"

namespace land {
namespace gui {

using ll::form::SimpleForm;

struct AdvancedLandPicker::Impl : std::enable_shared_from_this<Impl> {
    enum class View {
        All = 0,      // 所有领地视图
        OnlyOrdinary, // 普通领地视图
        OnlyParent,   // 父领地视图
        OnlyMix,      // 混合领地视图
        OnlySub,      // 子领地视图
    };

    std::vector<std::shared_ptr<Land>> mData;
    Callback                           mCallback;
    SimpleForm::ButtonCallback         mBackCallback;
    View                               mCurrentView{View::All};
    std::optional<std::string>         mFuzzyKeyword{std::nullopt};

    explicit Impl(std::vector<std::shared_ptr<Land>> data, Callback callback, SimpleForm::ButtonCallback back = {})
    : mData(std::move(data)),
      mCallback(std::move(callback)),
      mBackCallback(std::move(back)) {}

    std::string getViewName(View view, std::string const& localeCode) {
        switch (view) {
        case View::All:
            return "过滤: >全部领地<"_trl(localeCode);
        case View::OnlyOrdinary:
            return "过滤: >普通领地<"_trl(localeCode);
        case View::OnlyParent:
            return "过滤: >父领地<"_trl(localeCode);
        case View::OnlyMix:
            return "过滤: >混合领地<"_trl(localeCode);
        case View::OnlySub:
            return "过滤: >子领地<"_trl(localeCode);
        default:
            return "未知视图"_trl(localeCode);
        }
    }

    void _buildActionButton(PaginatedForm& form, View view, std::string const& localeCode) {
        form.appendButton(
            getViewName(view, localeCode),
            "textures/ui/store_sort_icon",
            "path",
            [data = shared_from_this()](Player& player) { data->nextView(player); }
        );
        form.appendButton(
            "模糊搜索"_trl(localeCode),
            "textures/ui/magnifyingGlass",
            "path",
            [data = shared_from_this()](Player& player) { data->sendFuzzySearch(player); }
        );
    }

    void sendFuzzySearch(Player& player) {
        auto localeCode = player.getLocaleCode();
        SimpleInputForm::sendTo(
            player,
            "[PLand] | 领地选择器-模糊搜索"_trl(localeCode),
            "请输入搜索关键字"_trl(localeCode),
            mFuzzyKeyword.value_or(""),
            [data = shared_from_this()](Player& player, std::string keyword) {
                if (keyword.empty()) {
                    data->mFuzzyKeyword = std::nullopt;
                } else {
                    data->mFuzzyKeyword = std::move(keyword);
                }
                data->sendView(player, data->mCurrentView);
            }
        );
    }

    bool canRender(View view, LandType type) {
        switch (view) {
        case View::OnlyOrdinary:
            return type == LandType::Ordinary;
        case View::OnlyParent:
            return type == LandType::Parent;
        case View::OnlyMix:
            return type == LandType::Mix;
        case View::OnlySub:
            return type == LandType::Sub;
        case View::All:
        default:
            return true;
        }
    }

    void buildView(PaginatedForm& form, View view, std::string const& localeCode) {
        form.setTitle("[PLand] | 领地选择器"_trl(localeCode));
        form.setContent("请选择一个领地"_trl(localeCode));
        if (mBackCallback) {
            back_utils::injectBackButton(form, mBackCallback);
        }
        _buildActionButton(form, view, localeCode);

        for (auto& land : mData) {
            if (mFuzzyKeyword.has_value() && land->getName().find(mFuzzyKeyword.value()) == std::string::npos) {
                continue;
            }
            if (!canRender(view, land->getType())) {
                continue;
            }
            std::string leaseContent = "";
            if (land->isLeased()) {
                auto state = land->getLeaseState();
                if (state == LeaseState::Active) {
                    leaseContent = time_utils::formatRemaining(land->getLeaseEndAt());
                } else if (state == LeaseState::Frozen) {
                    leaseContent = " | §e已冻结§r"_trl(localeCode);
                } else {
                    leaseContent = " | §c租赁过期§r"_trl(localeCode);
                }
            }
            form.appendButton(
                "[{}] {}§r\n维度: {}{}§r"_trl(
                    localeCode,
                    land->getId(),
                    land->getName(),
                    land->getDimensionId(),
                    leaseContent
                ),
                "textures/ui/icon_recipe_nature",
                "path",
                [data = shared_from_this(), weak = std::weak_ptr(land)](Player& player) {
                    if (auto land = weak.lock()) {
                        data->mCallback(player, land);
                    }
                }
            );
        }
    }

    void sendView(Player& player, View view) {
        PaginatedForm form{};
        buildView(form, view, player.getLocaleCode());
        form.sendTo(player);
        mCurrentView = view;
    }

    void nextView(Player& player) {
        if (mCurrentView == View::OnlySub) {
            sendView(player, View::All); // 回到初始视图
            return;
        }
        sendView(player, static_cast<View>(static_cast<int>(mCurrentView) + 1));
    }

    void sendTo(Player& player) { sendView(player, View::All); }
};


void AdvancedLandPicker::sendTo(
    Player&                            player,
    std::vector<std::shared_ptr<Land>> data,
    Callback                           callback,
    SimpleForm::ButtonCallback         backTo
) {
    auto impl = std::make_shared<Impl>(std::move(data), std::move(callback), std::move(backTo));
    impl->sendTo(player);
}

} // namespace gui
} // namespace land