# 配置文件

> 配置文件为`json`格式，位于`plugins/PLand/`目录下，文件名为`config.json`。

!> 配置文件为`json`格式，请勿使用记事本等不支持`json`格式的文本编辑器进行编辑，否则会导致配置文件损坏。

## 配置文件结构

```json
{
  "version": 23, // 配置文件版本，请勿修改
  "logLevel": "Info", // 日志等级 Off / Fatal / Error / Warn / Info / Debug / Trace
  "economy": {
    "enabled": false, // 是否启用经济系统
    "kit": "LegacyMoney", // 经济套件 LegacyMoney 或 ScoreBoard
    "scoreboardName": "Scoreboard", // 计分板对象名称
    "economyName": "money" // 经济名称
  },
  "land": {
    "landTp": true, // 是否启用领地传送
    "maxLand": 20, // 玩家最大领地数量
    "minSpacing": 16, // 领地最小间距
    "minSpacingIncludeY": true, // 领地最小间距是否包含Y轴
    "refundRate": 0.9, // 退款率(0.0~1.0，1.0为全额退款，0.9为退还90%)
    "discountRate": 1.0, // 折扣率(0.0~1.0，1.0为原价，0.9为打9折)

    "setupDrawCommand": true, // 是否注册领地范围绘制指令
    "drawRange": 64, // 绘制查询领地范围

    // v0.13.0 新增
    // 领地绘制后端，不同的后端在性能和效果上有所不同
    // DefaultParticle: 默认粒子效果 (基于点粒子, 性能较差)
    // BedrockServerClientInterfaceMod: 外部模组使用自定义粒子材质包 (性能较好, 但需要安装 BedrockServerClientInterface 模组和配套材质包)
    // MinecraftDebugShape: 基于 Minecraft 内置的 DebugShape (性能好, 无外部依赖, Minecraft 原生功能)
    // 默认情况下使用 MinecraftDebugShape 作为后端，因为其性能较好且无外部依赖 (如果您有更好的方案, 请提交 Issue 或 Pull Request)
    // v0.13.0+ (不含):
    // MinecraftDebugShape 从 PLand 剥离独立为一个通用 DebugShape Mod，使用 MinecraftDebugShape 需要安装 DebugShape.dll Mod
    "drawHandleBackend": "MinecraftDebugShape",

    "subLand": {
      "enabled": true, // 是否启用子领地
      "maxNested": 5, // 最大嵌套层数(默认5，最大16)
      "minSpacing": 8, // 子领地之间的最小间距
      "minSpacingIncludeY": true, // 子领地之间的最小间距是否包含Y轴
      "maxSubLand": 6, // 每个领地的最大子领地数量
      "calculate": "(square * 8 + height * 20)" // 价格公式
    },

    "tip": {
      "enterTip": true, // 是否启用进入领地提示
      "bottomContinuedTip": true, // 是否启用领地底部持续提示
      "bottomTipFrequency": 1 // 领地底部持续提示频率(s)
    },
    "bought": {
      "threeDimensionl": {
        "enabled": true, // 是否启用三维领地(购买)
        "calculate": "square * 25" // 领地价格计算公式
      },
      "twoDimensionl": {
        "enabled": true, // 是否启用二维领地(购买)
        "calculate": "square * 8 + height * 20" // 领地价格计算公式
      },
      "squareRange": {
        "min": 4, // 领地最小面积
        "max": 60000, // 领地最大面积
        "minHeight": 1 // 领地最小高度
      },
      "allowDimensions": [
        // 允许圈地的维度
        0, 1, 2
      ],
      "forbiddenRanges": [
        // 禁止创建领地的区域，领地管理员可以绕过此限制
        // min: 最小坐标
        // max: 最大坐标
        // dimensionId: 维度ID (0: 主世界, 1: 下界, 2: 末地)
        {
          "aabb": {
            "min": {
              "x": -100,
              "y": 0,
              "z": -100
            },
            "max": {
              "x": 100,
              "y": 255,
              "z": 100
            }
          },
          "dimensionId": 0
        }
      ],
      "dimensionPriceCoefficients": {
        // 维度价格系数，用于根据维度ID调整领地价格。键是维度ID的字符串形式，值是对应的价格系数。
        // 例如，"0": 1.0 表示主世界价格系数为1.0（原价），"1": 1.2 表示下界价格为1.2倍。
        "0": 1.0,
        "1": 1.2,
        "2": 1.5
      }
    }
  },
  "selector": {
    "tool": "minecraft:stick", // 工具
    "alias": "木棍" // 别名
  },
  "listeners": {
    // 事件监听器开关 true 为开启，false 为关闭
    // 注意：非必要情况下，请勿关闭事件监听器，否则可能导致领地功能异常
    // 如果您不知道某个事件监听器是做什么的，请不要关闭它
    // 提供的注释仅概括了大致作用，具体行为请查看源码
    // 当然，如果某个事件监听器导致游戏崩溃，您也可以选择关闭它
    "PlayerDestroyBlockEvent": true, // 玩家破坏方块
    "PlayerPlacingBlockEvent": true, // 玩家放置方块
    "PlayerInteractBlockEvent": true, // 玩家交互方块
    "FireSpreadEvent": true, // 火焰蔓延
    "PlayerAttackEvent": true, // 玩家攻击
    "PlayerPickUpItemEvent": true, // 玩家拾取物品
    "PlayerAttackBlockBeforeEvent": true, // 玩家攻击方块
    "ArmorStandSwapItemBeforeEvent": true, // 盔甲架交换物品
    "PlayerDropItemBeforeEvent": true, // 玩家丢弃物品
    "ActorRideBeforeEvent": true, // 实体骑乘
    "ExplosionBeforeEvent": true, // 爆炸
    "FarmDecayBeforeEvent": true, // 农田退化
    "MobHurtEffectBeforeEvent": true, // 生物受伤效果
    "PistonPushBeforeEvent": true, // 活塞推动
    "PlayerOperatedItemFrameBeforeEvent": true, // 玩家操作物品展示框
    "ActorTriggerPressurePlateBeforeEvent": true, // 实体触发压力板
    "ProjectileCreateBeforeEvent": true, // 投掷物创建
    "RedstoneUpdateBeforeEvent": true, // 红石更新
    "WitherDestroyBeforeEvent": true, // 凋零破坏
    "MossGrowthBeforeEvent": true, // 苔藓生长
    "LiquidTryFlowBeforeEvent": true, // 液体尝试流动
    "SculkBlockGrowthBeforeEvent": true, // 幽匿方块生长
    "SculkSpreadBeforeEvent": true, // 幽匿蔓延
    "PlayerEditSignBeforeEvent": true, // 玩家编辑告示牌
    "SpawnedMobEvent": true, // 生物生成
    "SculkCatalystAbsorbExperienceBeforeEvent": false, // 幽匿催化体吸收经验
    "PlayerInteractEntityBeforeEvent": true, // 玩家交互实体
    "BlockFallBeforeEvent": true, // 方块下落
    "ActorDestroyBlockEvent": true, // 实体破坏方块
    "MobPlaceBlockBeforeEvent": true, // 末影人放下方块
    "MobTakeBlockBeforeEvent": true, // 末影人拿走方块
    "DragonEggBlockTeleportBeforeEvent": true // 龙蛋传送
  },
  "hooks":{
    "registerMobHurtHook": true,// 注册生物受伤Hook
    "registerFishingHookHitHook": true,// 注册钓鱼竿Hook
    "registerLayEggGoalHook": true //注册海龟产卵Hook
  },
  "protection": {
    "mob": {
      "hostileMobTypeNames": [
        // 敌对生物
        // 关联权限: "allowMonsterDamage": "允许敌对生物受伤"
        "minecraft:enderman",
        "minecraft:zombie",
        "minecraft:witch"
        // ....
      ],
      "specialMobTypeNames": [
        // 特殊生物 (在 hostileMobTypeNames 后检查)
        // 关联权限: "allowSpecialDamage": "允许特殊实体受伤(船、矿车、画等)",
        "minecraft:chest_boat",
        "minecraft:painting",
        "minecraft:hopper_minecart"
        // ....
      ],
      "passiveMobTypeNames": [
        // 友好/中立生物 (在 specialMobTypeNames 后检查)
        // 关联权限: "allowPassiveDamage": "允许友好、中立生物受伤",
        "minecraft:sheep",
        "minecraft:cow",
        "minecraft:rabbit"
        // ....
      ],
      "customSpecialMobTypeNames": [
        // 自定义特殊生物、例如 Addon 生物 (最后检查)
        // 关联权限："allowCustomSpecialDamage": "允许自定义实体受伤",
      ]
    },
    "permissionMaps": {
      "itemSpecific": {
        "minecraft:skull": "allowPlace",
        "minecraft:banner": "allowPlace",
        "minecraft:glow_ink_sac": "allowPlace"
        // ...
      },
      "blockSpecific": {
        "minecraft:campfire": "useCampfire",
        "minecraft:dragon_egg": "allowAttackDragonEgg",
        "minecraft:bed": "useBed",
        "minecraft:lectern": "useLectern"
        // ...
      },
      "blockFunctional": {
        "minecraft:cartography_table": "useCartographyTable",
        "minecraft:smithing_table": "useSmithingTable",
        "minecraft:hopper": "useHopper",
        "minecraft:grindstone": "useGrindstone"
        // ...
      }
    }
  },
  "internal": {
    "devTools": false // 是否启用开发工具, 此工具依赖 OpenGL, 请确保你的设备支持 OpenGL
  }
}
```

## `protection.permissionMaps`

> v0.12.0 新增

此部分用于配置特定物品或方块交互所需的权限。这允许服主自定义哪些物品/方块在领地内可以被非成员使用。

它包含三个子对象：

- `itemSpecific`: 定义了使用特定**物品**时所需的权限。键是物品的命名空间 ID，值是权限名称。
- `blockSpecific`: 定义了与特定**方块**交互时所需的权限（通常是简单交互，如使用床）。键是方块的命名空间 ID，值是权限名称。
- `blockFunctional`: 定义了与具有复杂功能的**方块**交互时所需的权限（如工作台、熔炉等）。键是方块的命名空间 ID，值是权限名称。

默认配置中包含了大部分原版物品和方块的权限设置，您可以根据需要进行修改、添加或删除。

### 可用的权限值

以下是所有可用于 `permissionMaps` 的权限名称字符串：

| 权限名                 |
| :--------------------- |
| `allowPlace`           |
| `useFlintAndSteel`     |
| `useBoneMeal`          |
| `allowAttackDragonEgg` |
| `useBed`               |
| `allowOpenChest`       |
| `useCampfire`          |
| `useComposter`         |
| `useNoteBlock`         |
| `useJukebox`           |
| `useBell`              |
| `useDaylightDetector`  |
| `useLectern`           |
| `useCauldron`          |
| `useRespawnAnchor`     |
| `editFlowerPot`        |
| `allowDestroy`         |
| `useCartographyTable`  |
| `useSmithingTable`     |
| `useBrewingStand`      |
| `useAnvil`             |
| `useGrindstone`        |
| `useEnchantingTable`   |
| `useBarrel`            |
| `useBeacon`            |
| `useHopper`            |
| `useDropper`           |
| `useDispenser`         |
| `useLoom`              |
| `useStonecutter`       |
| `useCrafter`           |
| `useChiseledBookshelf` |
| `useCake`              |
| `useComparator`        |
| `useRepeater`          |
| `useBeeNest`           |
| `useVault`             |

**示例**：
默认情况下，`minecraft:flint_and_steel`（打火石）需要 `useFlintAndSteel` 权限。如果您想让它需要 `allowPlace` 权限，您可以这样修改：

```json
{
  "itemSpecific": {
    "minecraft:flint_and_steel": "allowPlace"
    // ... 其他物品
  }
}
```

## calculate 计算公式

?> PLand 的 `Calculate` 实现使用了 [`exprtk`](https://github.com/ArashPartow/exprtk) 库，因此你可以使用 [`exprtk`](https://github.com/ArashPartow/exprtk) 库所支持的所有函数和运算符。

|     变量      |     描述     |
| :-----------: | :----------: |
|   `height`    |   领地高度   |
|    `width`    |   领地宽度   |
|    `depth`    | 领地深度(长) |
|   `square`    |   领地面积   |
|   `volume`    |   领地体积   |
| `dimensionId` |   维度 ID    |

除此之外，价格表达式还支持调用随机数。

!> 注意：此功能仅限 `v0.8.0` 及以上版本使用。

|        函数        |             原型             |  返回值  |                描述                |
| :----------------: | :--------------------------: | :------: | :--------------------------------: |
|    `random_num`    |        `random_num()`        | `double` |   返回一个 `[0, 1)` 之间的随机数   |
| `random_num_range` | `random_num_range(min, max)` | `double` | 返回一个 `[min, max)` 之间的随机数 |

那么，我们可以写出这样的价格表达式：

```js
"square * random_num_range(10, 50)"; // 领地面积乘以 `[10, 50)` 之间的随机数
```
