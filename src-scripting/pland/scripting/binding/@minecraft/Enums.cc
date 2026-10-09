#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "mc/server/commands/CommandPermissionLevel.h"

#include <mc/server/commands/CommandParameterOption.h>

#include "mc/network/packet/ModalFormCancelReason.h"

namespace land::scripting {

decltype(Modules::kScriptCommandPermissionLevel) Modules::kScriptCommandPermissionLevel =
    helper::auto_gen_enum_def<CommandPermissionLevel>("CommandPermissionLevel");

decltype(Modules::kScriptCommandParameterOption) Modules::kScriptCommandParameterOption =
    helper::auto_gen_enum_def<CommandParameterOption>("CommandParameterOption");

decltype(Modules::kScriptModalFormCancelReason) Modules::kScriptModalFormCancelReason =
    helper::auto_gen_enum_def<ModalFormCancelReason>("ModalFormCancelReason");

} // namespace land::scripting