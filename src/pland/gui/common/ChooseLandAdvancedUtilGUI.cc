#include "ChooseLandAdvancedUtilGUI.h"
#include "ll/api/form/CustomForm.h"
#include "ll/api/form/SimpleForm.h"
#include "pland/gui/form/BackPaginatedSimpleForm.h"
#include "pland/gui/form/BackSimpleForm.h"
#include "pland/gui/form/PaginatedSimpleForm.h"
#include <cassert>


namespace land {

using ll::form::CustomForm;
using ll::form::CustomFormResult;


class ChooseLandAdvancedUtilGUI::Impl final {
    std::vector<SharedLand>                 mLands{};                    // 领地数据
    ChooseCallback                          mCallback{};                 // 回调
    BackSimpleForm<>::ButtonCallback        mBackCallback{};             // 返回按钮回调
    std::optional<std::string>              mFuzzyKeyword{std::nullopt}; // 模糊搜索关键字
    View                                    mCurrentView{View::All};     // 当前视图
    std::map<View, BackPaginatedSimpleForm> mViews{};                    // 视图

public:
    explicit Impl(std::vector<SharedLand> lands, ChooseCallback callback, BackSimpleForm<>::ButtonCallback back = {})
    : mLands(std::move(lands)),
      mCallback(std::move(callback)),
      mBackCallback(std::move(back)) {
#ifdef DEBUG
        std::cout << "ChooseLandAdvancedUtilGUI::Impl::Impl()" << std::endl;
#endif
    }

#ifdef DEBUG
    ~Impl() { std::cout << "ChooseLandAdvancedUtilGUI::Impl::~Impl()" << std::endl; }
#else
    ~Impl() = default;
#endif

    [[nodiscard]] Impl* getThis() { return this; }

    BackSimpleForm<>::ButtonCallback makeBackCallback() {
        return [thiz = getThis()](Player& self) {
            if (thiz) {
                thiz->mBackCallback(self);
                delete thiz; // 已返回上一个表单，当前表单不再需要
            }
        };
    }

    SimpleForm::ButtonCallback makeCallback(WeakLand wLand) {
        return [wLand, thiz = getThis()](Player& self) {
            auto land = wLand.lock();
            if (thiz && land) {
                thiz->mCallback(self, land);
                delete thiz; // 进入下一个表单，当前表单不再需要
            }
        };
    }

    void nextView(Player& player) {
        if (mCurrentView == View::OnlySub) {
            sendView(player, View::All); // 回到初始视图
            return;
        }
        sendView(player, static_cast<View>(static_cast<int>(mCurrentView) + 1));
    }

    SimpleForm::ButtonCallback makeNextViewCallback() {
        return [thiz = getThis()](Player& self) {
            if (thiz) {
                thiz->nextView(self);
            }
        };
    }

    void buildForms(Player& player) {
        if (mViews.empty()) {
            mViews.emplace(View::All, BackPaginatedSimpleForm::make(makeBackCallback()));
            mViews.emplace(View::OnlyOrdinary, BackPaginatedSimpleForm::make(makeBackCallback()));
            mViews.emplace(View::OnlyParent, BackPaginatedSimpleForm::make(makeBackCallback()));
            mViews.emplace(View::OnlyMix, BackPaginatedSimpleForm::make(makeBackCallback()));
            mViews.emplace(View::OnlySub, BackPaginatedSimpleForm::make(makeBackCallback()));

            for (auto& [view, form] : mViews) {
                form.setTitle("选择领地"_trf(player));
                form.setContent("请选择一个领地:"_trf(player));

                // 重置过滤器（仅在非主视图或存在模糊搜索关键字时）
                if (view != View::All || mFuzzyKeyword.has_value()) {
                    form.appendButton(
                        "重置过滤器"_trf(player),
                        "textures/ui/refresh_light",
                        "path",
                        [thiz = getThis()](Player& self) {
                            if (!thiz) return;
                            if (!thiz->mFuzzyKeyword.has_value()) {
                                thiz->sendView(self, View::All);
                                return; // 没有模糊搜索关键字，仅重置视图
                            }
                            thiz->mViews.clear();
                            thiz->mFuzzyKeyword = std::nullopt;
                            thiz->mCurrentView  = View::All;
                            thiz->sendTo(self); // 重新发送表单
                        }
                    );
                }

                form.appendButton(
                    "模糊搜索"_trf(player),
                    "textures/ui/magnifyingGlass",
                    "path",
                    [thiz = getThis()](Player& self) {
                        if (thiz) {
                            thiz->sendFuzzySearch(self);
                        }
                    }
                );
                form.onFormCanceled([thiz = getThis()](Player&) { delete thiz; });

                switch (view) {
                case View::All:
                    form.appendButton(
                        "过滤: >全部领地<"_trf(player),
                        "textures/ui/store_sort_icon",
                        "path",
                        makeNextViewCallback()
                    );
                    break;
                case View::OnlyOrdinary:
                    form.appendButton(
                        "过滤: >普通领地<"_trf(player),
                        "textures/ui/store_sort_icon",
                        "path",
                        makeNextViewCallback()
                    );
                    break;
                case View::OnlyParent:
                    form.appendButton(
                        "过滤: >父领地<"_trf(player),
                        "textures/ui/store_sort_icon",
                        "path",
                        makeNextViewCallback()
                    );
                    break;
                case View::OnlyMix:
                    form.appendButton(
                        "过滤: >混合领地<"_trf(player),
                        "textures/ui/store_sort_icon",
                        "path",
                        makeNextViewCallback()
                    );
                    break;
                case View::OnlySub:
                    form.appendButton(
                        "过滤: >子领地<"_trf(player),
                        "textures/ui/store_sort_icon",
                        "path",
                        makeNextViewCallback()
                    );
                    break;
                }
            }
        }

        for (auto const& land : mLands) {
            if (mFuzzyKeyword && land->getName().find(*mFuzzyKeyword) == std::string::npos) {
                continue; // 模糊搜索
            }

            auto wLand = std::weak_ptr(land);

            std::string text =
                "{}\n维度: {} | ID: {}"_trf(player, land->getName(), land->getDimensionId(), land->getId());

            mViews.at(View::All).appendButton(text, "textures/ui/icon_recipe_nature", "path", makeCallback(wLand));

            switch (land->getType()) {
            case Land::Type::Ordinary:
                mViews.at(View::OnlyOrdinary)
                    .appendButton(text, "textures/ui/icon_recipe_nature", "path", makeCallback(wLand));
                break;
            case Land::Type::Parent:
                mViews.at(View::OnlyParent)
                    .appendButton(text, "textures/ui/icon_recipe_nature", "path", makeCallback(wLand));
                break;
            case Land::Type::Mix:
                mViews.at(View::OnlyMix)
                    .appendButton(text, "textures/ui/icon_recipe_nature", "path", makeCallback(wLand));
                break;
            case Land::Type::Sub:
                mViews.at(View::OnlySub)
                    .appendButton(text, "textures/ui/icon_recipe_nature", "path", makeCallback(wLand));
                break;
            }
        }
    }

    void sendFuzzySearch(Player& player) {
        CustomForm fm;
        fm.setTitle(PLUGIN_NAME + " | 模糊搜索领地"_trf(player));
        fm.appendInput("name", "请输入领地名称"_trf(player), "string", mFuzzyKeyword.value_or(""));
        fm.sendTo(player, [thiz = getThis()](Player& self, CustomFormResult const& res, auto) {
            if (!thiz) return;
            if (!res) {
                delete thiz;
                return;
            }
            auto name = std::get<std::string>(res->at("name"));
            if (name.empty()) {
                return thiz->sendFuzzySearch(self); // 重新发送
            }
            thiz->mFuzzyKeyword = name;
            thiz->mViews.clear();
            thiz->buildForms(self);
            thiz->sendView(self, thiz->mCurrentView);
        });
    }

    void sendView(Player& player, View view) {
        mCurrentView = view;
        mViews.at(view).sendTo(player);
    }

    void sendTo(Player& player) {
        buildForms(player);
        sendView(player, View::All);
    }
};


// ChooseLandAdvancedUtilGUI
ChooseLandAdvancedUtilGUI::ChooseLandAdvancedUtilGUI(
    std::vector<SharedLand>          lands,
    ChooseCallback                   callback,
    BackSimpleForm<>::ButtonCallback back
) {
    impl_ = new Impl(std::move(lands), std::move(callback), std::move(back));
}

void ChooseLandAdvancedUtilGUI::sendTo(Player& player) { impl_->sendTo(player); }


} // namespace land