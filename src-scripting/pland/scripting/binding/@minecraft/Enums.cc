#include "pland/scripting/binding/Helper.h"
#include "pland/scripting/binding/Modules.h"
#include "pland/scripting/pch.h"

#include "mc/server/commands/CommandPermissionLevel.h"

#include <mc/server/commands/CommandParameterOption.h>

namespace land::scripting {

decltype(Modules::kScriptCommandPermissionLevel) Modules::kScriptCommandPermissionLevel =
    helper::auto_gen_enum_def<CommandPermissionLevel>("CommandPermissionLevel");

decltype(Modules::kScriptCommandParameterOption) Modules::kScriptCommandParameterOption =
    helper::auto_gen_enum_def<CommandParameterOption>("CommandParameterOption");

} // namespace land::scripting