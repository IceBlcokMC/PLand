#pragma once
#include <functional>
#include <string_view>

#include "pland/land/repo/LandContext.h"
#include "pland/reflect/Traits.h"
#include "pland/reflect/TypeName.h"

class Player;

namespace land::gui {

class BetterPermissionEditorGUI {
    struct Impl;

public:
    BetterPermissionEditorGUI() = delete;

    template <typename CT, typename FT>
    struct MemberFieldBase {
        using class_type = CT;
        using field_type = FT;

        std::string_view name;
        field_type class_type::* offset{};

        constexpr MemberFieldBase() = default;
        constexpr MemberFieldBase(std::string_view name, field_type class_type::* offset)
        : name(name),
          offset(offset) {}

        [[nodiscard]] field_type&       get(class_type& obj) const { return obj.*offset; }
        [[nodiscard]] field_type const& get(class_type const& obj) const { return obj.*offset; }
        void                            set(class_type& obj, field_type value) const { obj.*offset = value; }
    };

    template <auto MemberPtr>
    struct MemberField : MemberFieldBase<
                             reflect::member_pointer_class_t<decltype(MemberPtr)>,
                             reflect::member_pointer_field_t<decltype(MemberPtr)>> {
        using Base = MemberFieldBase<
            reflect::member_pointer_class_t<decltype(MemberPtr)>,
            reflect::member_pointer_field_t<decltype(MemberPtr)>>;

        constexpr MemberField() : Base(reflect::getTemplateInnerLeafName<MemberPtr>(), MemberPtr) {}
    };


    using EnvPermField       = MemberFieldBase<EnvironmentPerms, bool>;
    using RolePermField      = MemberFieldBase<RolePerms, RolePerms::Entry>;
    using RolePermEntryField = MemberFieldBase<RolePerms::Entry, bool>;

    using EnvFieldChanged  = std::function<void(EnvPermField const& field, EnvPermField::field_type val)>;
    using RoleFieldChanged = std::function<
        void(RolePermField const& permf, RolePermEntryField const& entryf, RolePermEntryField::field_type val)>;

    static bool sendTo(
        Player&                 player,
        LandPermTable const&    perms,
        EnvFieldChanged const&  envFieldChanged,
        RoleFieldChanged const& roleFieldChanged
    );
};


} // namespace land::gui
