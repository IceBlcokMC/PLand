# PLand - Scripting 子系统 <badge type="tip" text="实验性(开发中)" />

PLand 的 Scripting 子系统是 PLand 内建的一套脚本系统，它基于 QuickJs 实现，提供了一套精简的脚本运行时和API，用于在 PLand 中实现一些简单给逻辑扩展。

:::info 下文将 PLand Scripting 简称为 Scripting
:::

## 前置须知

:::warning Scripting 的定位不是一个完整的通用脚本系统，而是服务于修改、定制、扩展 PLand 领地功能的轻量级脚本系统, 如果您需要更强的游戏引擎功能，请考虑基于社区的 [`LSE 引擎`](../lrca/index.md) 进行开发
:::

Scripting 分为两种运行策略：

A: PLand Private Scripting, 私有脚本, 它负责领地的脚本业务，混合辅助 C++ 层，实现完整的领地功能。  
B: Community Scripting, 社区脚本, 享受相同的 API，可以按需扩展功能，与 Private Scripting 无本质区别。

## 快速开始

Scripting 为社区脚本注册了一个加载器类型，您可以在 `manifest.json` 中指定类型为 `pland-script` 并添加依赖，即可在 PLand 中运行您的脚本。

### manifest.json

```json
{
  "name": "test-pland-script", // 您的脚本插件名称
  "entry": "index.js", // 脚本入口文件
  "version": "0.0.0", // 插件版本
  "type": "pland-script", // 插件类型
  "dependencies": [
    {
      // 声明依赖, 如果缺失此项配置，可能导致您的脚本先于
      //  PLand 被加载器尝试加载，但由于找不到 pland-scritp 类型而加载失败
      "name": "PLand"
    }
  ]
}
```

### index.js

scripting 要求每个脚本插件的入口文件必须是一个模块，并且默认导出一个类，该类必须实现以下三个方法：

```js
import { log } from "@runtime"; // 导入 runtime 模块

export default class Entry {
  onLoad() {
    // 当脚本插件被加载时调用
    log("onLoad called, test-pland-script");
  }

  onEnable() {
    // 当脚本插件被启用时调用
    log("onEnable called, test-pland-script");
  }

  onDisable() {
    // 当脚本插件被禁用时调用
    log("onDisable called, test-pland-script");
  }
}
```

这3个方法作为脚本插件的生命周期钩子，您可以在此根据需求编写您的业务逻辑。

:::warning 时序要求: 在 onEnable 之前，您不应该访问 `@minecraft`、`@pland` 模块，您需要等待 onEnable 调用之后再注册事件、命令等
:::

:::tip 如果您需要访问入口文件导出的类实例，您可以通过 `@runtime` 模块的 `self` 方法获取实例
:::

:::tip scripting 为每个脚本插件分配了一个独立的 `Plugin` 实例，您可以直接访问 `@runtime` 模块的 `pluginCtx` 属性获取必要的上下文而不是硬编码路径等参数
:::

### 测试加载

当您编写好脚本后，将脚本文件夹放置在加载器的 plugins 目录下，启动服务端即可。

scripting 支持热重载，您可以使用 `ll reload <name>` 命令重载脚本插件。

:::tip 如果遇到 PLand 私有脚本卡死，可以使用 `pland reload scripts` 重载内置脚本
:::

### 参考项目

PLand 项目下的 `tests/test-pland-script` 目录

## TypeScript 支持

我们推荐使用 TypeScript 编写脚本，以获得更好的开发体验、类型检查等。

### 类型文件

引擎相关类型文件请查看 PLand 项目 `src-scripts/types` 目录。

### tsconfig.json 参考

这里是基于 PLand 内置脚本的 tsconfig.json 配置做参考，实际项目中请根据需求自行调整。

- 工程结构 (以 PLand 为例)

```bash
D:/PLand
├─ build
├─ docs
├─ src-scripting
├─ src-scritps
│   │  .prettierrc
│   │  xmake.lua
│   │
│   ├─src
│   │  │  index.ts
│   │  │
│   │  ├─commands
│   │  │      commands.ts
│   │  │
│   │  └─i18n
│   │          i18n.ts
│   │
│   └─types
│
├─ xmake.lua
└─ tsconfig.json
```

- 配置

```json
{
  "include": ["src-scripts/src/**/*.ts", "src-scripts/types/**/*.d.ts"], // 根据实际项目调整
  "exclude": ["node_modules", "build", "docs"], // 根据实际项目调整
  "compilerOptions": {
    "outDir": "build/.dist", // 编译输出目录 (根据实际项目调整)
    "rootDir": "src-scripts/src", // 源码目录 (根据实际项目调整)
    "target": "ES2022",
    "lib": ["ES2022"], // 仅包含 ES2022 标准库
    "module": "esnext",
    "moduleResolution": "bundler",
    "verbatimModuleSyntax": true,
    "isolatedModules": true,
    "strict": true, // 开启严格模式
    "sourceMap": true // 生成 sourceMap
  }
}
```

## 注意事项

1. scripting 仅支持 es6 模块语法，不支持其他模块语法，如 CommonJS、AMD 等。
2. scripting 默认启用严格模式，您需要遵循严格模式的要求编写脚本。
3. 尽可能保证您的脚本基于事件驱动，避免热重载导致副作用问题

:::warning scripting 目前处于开发中，相关 API 或 API 行为可能发生变化
:::
