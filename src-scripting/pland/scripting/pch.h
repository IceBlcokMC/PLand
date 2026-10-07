#pragma once

// std
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// jspp
#include "jspp/Jspp.h"
#include "jspp/Macro.h"
#include "jspp/binding/BindingUtils.h"
#include "jspp/binding/MetaBuilder.h"
#include "jspp/binding/NativeInstanceImpl.h"
#include "jspp/binding/ReturnValuePolicy.h"
#include "jspp/binding/TypeConverter.h"
#include "jspp/binding/traits/FunctionTraits.h"
#include "jspp/binding/traits/Polymorphic.h"
#include "jspp/binding/traits/TypeTraits.h"
#include "jspp/core/Concepts.h"
#include "jspp/core/Engine.h"
#include "jspp/core/EngineScope.h"
#include "jspp/core/Exception.h"
#include "jspp/core/Fwd.h"
#include "jspp/core/InstancePayload.h"
#include "jspp/core/MetaInfo.h"
#include "jspp/core/NativeInstance.h"
#include "jspp/core/Reference.h"
#include "jspp/core/Reference.inl"
#include "jspp/core/TrackedHandle.h"
#include "jspp/core/Trampoline.h"
#include "jspp/core/Utils.h"
#include "jspp/core/Value.h"
#include "jspp/core/ValueHelper.h"

// jspp 扩展点的特化 (RTTI / 承载实例 / 类型转换)
#include "pland/scripting/binding/Customizations.h"

// absl
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
