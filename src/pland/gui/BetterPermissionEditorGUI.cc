#include "BetterPermissionEditorGUI.h"

#include "ll/api/i18n/I18n.h"
#include "ll/api/reflection/Reflection.h"
#include "ll/api/ui/form/CustomForm.h"

#include "mc/world/actor/player/Player.h"

#include "fmt/format.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace land::gui {

using namespace ll::i18n_literals;

using GUI = BetterPermissionEditorGUI;

namespace {

constexpr std::size_t kPageSize = 10;

constexpr ll::ui::ObservableOptions kClientWritable{.clientWritable = true};
constexpr ll::ui::ObservableOptions kServerOnly{.clientWritable = false};

std::string toLowerAscii(std::string_view in) {
    std::string out{in};
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

// 字段表：数组长度与 pfr 反射出的成员数一致，新增/删除权限字段时若忘同步本表会编译失败
constexpr std::array<GUI::EnvPermField, ll::reflection::member_count_v<EnvironmentPerms>> kEnvFields{
    {
     GUI::MemberField<&EnvironmentPerms::allowFireSpread>{},
     GUI::MemberField<&EnvironmentPerms::allowMonsterSpawn>{},
     GUI::MemberField<&EnvironmentPerms::allowAnimalSpawn>{},
     GUI::MemberField<&EnvironmentPerms::allowMobGrief>{},
     GUI::MemberField<&EnvironmentPerms::allowExplode>{},
     GUI::MemberField<&EnvironmentPerms::allowFarmDecay>{},
     GUI::MemberField<&EnvironmentPerms::allowPistonPushOnBoundary>{},
     GUI::MemberField<&EnvironmentPerms::allowRedstoneUpdate>{},
     GUI::MemberField<&EnvironmentPerms::allowBlockFall>{},
     GUI::MemberField<&EnvironmentPerms::allowWitherDestroy>{},
     GUI::MemberField<&EnvironmentPerms::allowMossGrowth>{},
     GUI::MemberField<&EnvironmentPerms::allowLiquidFlow>{},
     GUI::MemberField<&EnvironmentPerms::allowDragonEggTeleport>{},
     GUI::MemberField<&EnvironmentPerms::allowSculkBlockGrowth>{},
     GUI::MemberField<&EnvironmentPerms::allowSculkSpread>{},
     GUI::MemberField<&EnvironmentPerms::allowLightningBolt>{},
     GUI::MemberField<&EnvironmentPerms::allowMinecartHopperPullItems>{},
     }
};

constexpr std::array<GUI::RolePermField, ll::reflection::member_count_v<RolePerms>> kRoleFields{
    {
     GUI::MemberField<&RolePerms::allowDestroy>{},
     GUI::MemberField<&RolePerms::allowPlace>{},
     GUI::MemberField<&RolePerms::useBucket>{},
     GUI::MemberField<&RolePerms::useAxe>{},
     GUI::MemberField<&RolePerms::useHoe>{},
     GUI::MemberField<&RolePerms::useShovel>{},
     GUI::MemberField<&RolePerms::placeBoat>{},
     GUI::MemberField<&RolePerms::placeMinecart>{},
     GUI::MemberField<&RolePerms::useButton>{},
     GUI::MemberField<&RolePerms::useDoor>{},
     GUI::MemberField<&RolePerms::useFenceGate>{},
     GUI::MemberField<&RolePerms::allowInteractEntity>{},
     GUI::MemberField<&RolePerms::useTrapdoor>{},
     GUI::MemberField<&RolePerms::editSign>{},
     GUI::MemberField<&RolePerms::useLever>{},
     GUI::MemberField<&RolePerms::useFurnaces>{},
     GUI::MemberField<&RolePerms::allowPlayerPickupItem>{},
     GUI::MemberField<&RolePerms::allowRideTrans>{},
     GUI::MemberField<&RolePerms::allowRideEntity>{},
     GUI::MemberField<&RolePerms::usePressurePlate>{},
     GUI::MemberField<&RolePerms::allowFishingRodAndHook>{},
     GUI::MemberField<&RolePerms::allowUseThrowable>{},
     GUI::MemberField<&RolePerms::useArmorStand>{},
     GUI::MemberField<&RolePerms::allowDropItem>{},
     GUI::MemberField<&RolePerms::useItemFrame>{},
     GUI::MemberField<&RolePerms::useFlintAndSteel>{},
     GUI::MemberField<&RolePerms::useBeacon>{},
     GUI::MemberField<&RolePerms::useBed>{},
     GUI::MemberField<&RolePerms::allowPvP>{},
     GUI::MemberField<&RolePerms::allowHostileDamage>{},
     GUI::MemberField<&RolePerms::allowFriendlyDamage>{},
     GUI::MemberField<&RolePerms::allowSpecialEntityDamage>{},
     GUI::MemberField<&RolePerms::useContainer>{},
     GUI::MemberField<&RolePerms::useWorkstation>{},
     GUI::MemberField<&RolePerms::useBell>{},
     GUI::MemberField<&RolePerms::useCampfire>{},
     GUI::MemberField<&RolePerms::useComposter>{},
     GUI::MemberField<&RolePerms::useDaylightDetector>{},
     GUI::MemberField<&RolePerms::useJukebox>{},
     GUI::MemberField<&RolePerms::useNoteBlock>{},
     GUI::MemberField<&RolePerms::useCake>{},
     GUI::MemberField<&RolePerms::useComparator>{},
     GUI::MemberField<&RolePerms::useRepeater>{},
     GUI::MemberField<&RolePerms::useLectern>{},
     GUI::MemberField<&RolePerms::useCauldron>{},
     GUI::MemberField<&RolePerms::useRespawnAnchor>{},
     GUI::MemberField<&RolePerms::useBoneMeal>{},
     GUI::MemberField<&RolePerms::useBeeNest>{},
     GUI::MemberField<&RolePerms::editFlowerPot>{},
     GUI::MemberField<&RolePerms::allowUseRangedWeapon>{},
     GUI::MemberField<&RolePerms::allowTriggerDripleaf>{},
     }
};

constexpr GUI::RolePermEntryField kMemberField{GUI::MemberField<&RolePerms::Entry::member>{}};
constexpr GUI::RolePermEntryField kActorField{GUI::MemberField<&RolePerms::Entry::actor>{}};

} // namespace

struct BetterPermissionEditorGUI::Impl : std::enable_shared_from_this<Impl> {
    struct EnvUnit {
        std::string_view          permName;
        std::string               displayName;
        GUI::EnvPermField         field;
        ll::ui::ObservableBoolean toggled;
        ll::ui::ObservableBoolean visible;
    };

    struct RoleUnit {
        std::string_view          permName;
        std::string               displayName;
        GUI::RolePermField        field;
        ll::ui::ObservableBoolean member;
        ll::ui::ObservableBoolean actor;
        ll::ui::ObservableBoolean visible;
    };

    // 环境块 + 角色块的综合顺序槽位
    struct Slot {
        bool        isEnv;
        std::size_t index;
    };

    LandPermTable         ptable_;
    GUI::EnvFieldChanged  envFieldChanged_;
    GUI::RoleFieldChanged roleFieldChanged_;

    std::optional<ll::ui::CustomForm> form_;

    ll::ui::ObservableNumber categoryIdx{0.0, kClientWritable};
    ll::ui::ObservableString serachText_{std::string{}, kClientWritable};
    ll::ui::ObservableNumber pageIdx{0.0, kClientWritable};
    ll::ui::ObservableNumber pageMax{0.0, kServerOnly};
    ll::ui::ObservableString pageDesc{std::string{}, kServerOnly};

    std::vector<EnvUnit>  envUnits_;
    std::vector<RoleUnit> roleUnits_;
    std::vector<Slot>     slots_;

    Impl(LandPermTable const& perms, GUI::EnvFieldChanged envFieldChanged, GUI::RoleFieldChanged roleFieldChanged)
    : ptable_(perms),
      envFieldChanged_(std::move(envFieldChanged)),
      roleFieldChanged_(std::move(roleFieldChanged)) {
        envUnits_.reserve(kEnvFields.size());
        roleUnits_.reserve(kRoleFields.size());
        slots_.reserve(kEnvFields.size() + kRoleFields.size());
    }

    ll::ui::ObservableBoolean& visibleOf(Slot slot) {
        return slot.isEnv ? envUnits_[slot.index].visible : roleUnits_[slot.index].visible;
    }

    std::string_view nameOf(Slot slot) const {
        return slot.isEnv ? envUnits_[slot.index].permName : roleUnits_[slot.index].permName;
    }

    std::string const& displayNameOf(Slot slot) const {
        return slot.isEnv ? envUnits_[slot.index].displayName : roleUnits_[slot.index].displayName;
    }

    void applyEnv(GUI::EnvPermField const& field, bool value) {
        field.set(ptable_.environment, value);
        if (envFieldChanged_) {
            envFieldChanged_(field, value);
        }
    }

    void applyRole(GUI::RolePermField const& permField, GUI::RolePermEntryField const& entryField, bool value) {
        auto entry = permField.get(ptable_.role);
        entryField.set(entry, value);
        permField.set(ptable_.role, entry);
        if (roleFieldChanged_) {
            roleFieldChanged_(permField, entryField, value);
        }
    }

    void build(std::string_view localeCode) {
        auto& i18n = ll::i18n::getInstance();
        auto  weak = std::weak_ptr{shared_from_this()};

        for (auto const& field : kEnvFields) {
            envUnits_.push_back(
                EnvUnit{
                    .permName    = field.name,
                    .displayName = std::string{           i18n.get(field.name,     localeCode)},
                    .field       = field,
                    .toggled     = ll::ui::ObservableBoolean{field.get(ptable_.environment), kClientWritable},
                    .visible     = ll::ui::ObservableBoolean{                          true,     kServerOnly},
            }
            );
            auto& unit = envUnits_.back();
            unit.toggled.subscribe([weak, f = unit.field](bool const& value) {
                if (auto self = weak.lock()) {
                    self->applyEnv(f, value);
                }
            });
            slots_.push_back({.isEnv = true, .index = envUnits_.size() - 1});
        }

        for (auto const& field : kRoleFields) {
            auto const entry = field.get(ptable_.role);
            roleUnits_.push_back(
                RoleUnit{
                    .permName    = field.name,
                    .displayName = std::string{i18n.get(field.name,     localeCode)},
                    .field       = field,
                    .member      = ll::ui::ObservableBoolean{       entry.member, kClientWritable},
                    .actor       = ll::ui::ObservableBoolean{        entry.actor, kClientWritable},
                    .visible     = ll::ui::ObservableBoolean{               true,     kServerOnly},
            }
            );
            auto& unit = roleUnits_.back();
            unit.member.subscribe([weak, f = unit.field, ef = kMemberField](bool const& value) {
                if (auto self = weak.lock()) {
                    self->applyRole(f, ef, value);
                }
            });
            unit.actor.subscribe([weak, f = unit.field, ef = kActorField](bool const& value) {
                if (auto self = weak.lock()) {
                    self->applyRole(f, ef, value);
                }
            });
            slots_.push_back({.isEnv = false, .index = roleUnits_.size() - 1});
        }

        categoryIdx.subscribe([weak](double const&) {
            if (auto self = weak.lock()) self->updateVisibility();
        });
        serachText_.subscribe([weak](std::string const&) {
            if (auto self = weak.lock()) self->updateVisibility();
        });
        pageIdx.subscribe([weak](double const&) {
            if (auto self = weak.lock()) self->updateVisibility();
        });
    }

    void updateVisibility() {
        auto const category = static_cast<int>(categoryIdx.getData());
        auto const needle   = toLowerAscii(serachText_.getData());

        std::vector<std::size_t> matched;
        matched.reserve(slots_.size());
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            auto const& slot = slots_[i];
            if (category != 0 && (category == 1) != slot.isEnv) {
                continue;
            }
            if (!needle.empty() && toLowerAscii(nameOf(slot)).find(needle) == std::string::npos
                && toLowerAscii(displayNameOf(slot)).find(needle) == std::string::npos) {
                continue;
            }
            matched.push_back(i);
        }

        auto const pageCount = matched.empty() ? 0 : (matched.size() - 1) / kPageSize; // 最后一页下标
        pageMax.setData(static_cast<double>(pageCount));

        auto const current =
            static_cast<std::size_t>(std::clamp(pageIdx.getData(), 0.0, static_cast<double>(pageCount)));
        pageIdx.setData(static_cast<double>(current)); // 越界回跳（值相等时 setData 自动短路）

        auto const begin = current * kPageSize;
        auto const end   = begin + kPageSize;

        std::vector<char> flags(slots_.size(), 0);
        for (std::size_t pos = begin; pos < std::min(end, matched.size()); ++pos) {
            flags[matched[pos]] = 1;
        }
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            visibleOf(slots_[i]).setData(flags[i] != 0);
        }

        pageDesc.setData(fmt::format("{}/{}", current + 1, pageCount + 1));
    }

    void buildForm(ll::ui::CustomForm& f, std::string_view localeCode) {
        f.dropdown(
            "权限类别"_trl(localeCode),
            categoryIdx,
            std::vector<ll::ui::DropdownItemData>{
                {               std::string{"所有权限"_trl(localeCode)}, 0.0, {}},
                {               std::string{"环境权限"_trl(localeCode)}, 1.0, {}},
                {std::string{"角色权限(领地成员/实体)"_trl(localeCode)}, 2.0, {}},
        }
        );
        f.textField("搜索"_trl(localeCode), serachText_);
        f.slider(
            "页码"_trl(localeCode),
            pageIdx,
            0.0,
            ll::ui::NumberValue{pageMax},
            ll::ui::SliderOptions{.description = ll::ui::TextValue{pageDesc}, .step = 1.0}
        );

        // 每个权限单元前插一个 spacer 拉开间距；spacer 与单元共享同一 visible，被筛选掉的单元不留空隙
        for (auto& unit : envUnits_) {
            auto const vis = ll::ui::BooleanValue{unit.visible};
            f.spacer(ll::ui::SpacingOptions{.visible = vis});
            f.toggle(unit.displayName, unit.toggled, ll::ui::ToggleOptions{.visible = vis});
        }
        for (auto& unit : roleUnits_) {
            auto const vis = ll::ui::BooleanValue{unit.visible};
            f.spacer(ll::ui::SpacingOptions{.visible = vis});
            f.label(unit.displayName, ll::ui::TextOptions{.visible = vis});
            f.toggle("成员"_trl(localeCode), unit.member, ll::ui::ToggleOptions{.visible = vis});
            f.toggle("实体"_trl(localeCode), unit.actor, ll::ui::ToggleOptions{.visible = vis});
        }
    }

    bool sendTo(Player& player) {
        auto const localeCode = player.getLocaleCode();

        auto f = ll::ui::CustomForm{player, std::string{"权限编辑器"_trl(localeCode)}};
        buildForm(f, localeCode);
        updateVisibility(); // 在 show() 锁结构前完成初筛，首帧即呈现正确的可见性与页码

        form_.emplace(std::move(f));

        // 自托管生命周期：shared_ptr 由 show 完成回调持有，
        // LL 保证回调在屏幕关闭/玩家退出/服务器停止时恰好触发一次，随后回调对象析构即释放本对象
        return static_cast<bool>(form_->show([self = shared_from_this()](ll::ui::CustomForm::Result) mutable {
            self->onClosed();
        }));
    }

    void onClosed() {
        // 屏幕已结束，释放表单句柄；self 自身随完成回调对象析构一并释放
        form_.reset();
    }
};


bool GUI::sendTo(
    Player&                 player,
    LandPermTable const&    perms,
    EnvFieldChanged const&  envFieldChanged,
    RoleFieldChanged const& roleFieldChanged
) {
    auto impl = std::make_shared<Impl>(perms, envFieldChanged, roleFieldChanged);
    impl->build(player.getLocaleCode());
    return impl->sendTo(player);
}

} // namespace land::gui
