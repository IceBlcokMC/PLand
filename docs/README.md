# PLand

- 简体中文
- [English](./README_EN.md)

基于 LeviLamina 开发、适用于 BDS 的领地系统 PLand。

## 功能特性

- 多种领地类型
    - [x] 2D 领地
    - [x] 3D 领地
    - [x] 子领地(支持多级嵌套)
    - [x] 租赁模式
- 领地保护
    - [x] 60+ 权限精细保护领地
    - [x] 可动态扩展部分拦截配置(如：交互方块、实体受伤等)
    - 4 角色权限模型
        - [x] 管理员(全局最高权限)
        - [x] 领地主人
        - [x] 领地成员(可独立配置权限)
        - [x] 领地访客(可独立配置权限)
    - 权限分层
        - [x] 环境权限(如：闪电、流体、苔藓蔓延等)
        - [x] 角色权限(玩家交互相关的权限，如：使用物品、打开箱子等...)
    - 覆盖率
        - 覆盖 40+ 底层交互事件
        - 额外 Hook 修复越权漏洞
- 玩法拓展:
    - [x] 经济系统(支持双经济系统)
        - [x] 每种领地类型均支持独立价格计算公式
        - [x] 维度价格倍率
        - [x] 折扣率、退款率可配置
    - [x] 领地传送
    - [x] 领地转让
    - [x] 领地范围绘制
    - 领地约束
        - [x] 维度禁止区域(禁止在一些区域创建领地)
        - [x] 可配置领地间距
        - [x] 领地最小范围
        - [x] 领地数量限制
        - [x] 子领地层级限制
        - [x] 维度白名单
- GUI 覆盖:
    - [x] 封装 GUI 表单，方便玩家操作
    - [x] 分页表单、高级过滤器
    - [x] 管理员专属管理表单
- 多语言支持
    - [x] 简体中文 (内置)
    - [x] 美式英语 (AI 翻译)
    - [x] 俄语 (AI 翻译)
- 性能优化
    - [x] 领地查询哈希优化
    - [x] 子领地层级预计算 + 缓存
    - [x] 双向表加速查询和删除
- 开发者相关:
    - [x] 封装大量事件，可扩展
    - [x] 封装 Service 代码复用
    - [x] DevTool 开发者工具
        - [x] 领地可视化查看
        - [x] 实时编辑
        - [x] 领地层级可视化

> 更多内容，请移步文档站：https://iceblcokmc.github.io/PLand/

## 工程结构

```bash
C:\PLand
├─assets # 插件资源文件（语言、文本）
│  └─lang # 多语言支持的 JSON 语言包
│
├─docs # 项目文档
│  ├─dev # 开发者文档
│  └─md # 用户文档
│
├─src
│  └─pland
│     ├─aabb # 空间/数学计算类
│     ├─drawer # 领地绘制器
│     │  └─detail # 后端对接实现
│     ├─economy # 双经济对接
│     ├─events
│     │  ├─domain # 领域事件、数据同步、状态更新
│     │  ├─economy # 经济相关事件
│     │  └─player # 玩家操作事件
│     ├─gui
│     │  ├─admin # 管理员专用表单
│     │  ├─common # 公共表单组件
│     │  └─utils # 表单工具类
│     ├─infra # 基础设施
│     │  └─migrator # 数据迁移器
│     ├─internal # 内部实现
│     │  ├─adapter # 适配器
│     │  │  └─telemetry # 遥测
│     │  ├─command # 命令
│     │  └─interceptor # 拦截器
│     │      ├─detail
│     │      └─helper
│     ├─land
│     │  ├─internal # 领地内部实现
│     │  ├─repo # 仓库、领地注册表
│     │  │  └─internal # 注册表内部实现
│     │  └─validator # 领地验证器
│     ├─reflect # 反射
│     ├─selector # 选区实现
│     │  └─land # 选区器实现
│     ├─service # 服务类
│     └─utils # 工具类
└─ src-devtool # 开发者工具
    ├─components # 可复用组件
    ├─deps # 依赖
    └─menus # 菜单
        ├─helper # 帮助菜单
        │  └─element # 菜单元素
        └─viewer
            └─element
```

## 开源协议

本项目采用 [AGPL-3.0 or later](LICENSE) 开源协议。

> 开发者不对使用本软件造成的任何后果负责，当您使用本项目以及衍生版本时，您需要自行承担风险。

> 感谢以下开源项目对本项目的支持与帮助。

| 项目名称                   | 项目地址                                                        |
|:-----------------------|:------------------------------------------------------------|
| LeviLamina             | https://github.com/LiteLDev/LeviLamina                      |
| exprtk                 | https://github.com/ArashPartow/exprtk                       |
| LegacyMoney            | https://github.com/LiteLDev/LegacyMoney                     |
| iListenAttentively(闭源) | https://github.com/MiracleForest/iListenAttentively-Release |
| ImGui                  | https://github.com/ocornut/imgui                            |
| glew                   | https://github.com/nigels-com/glew                          |
| ImGuiColorTextEdit     | https://github.com/goossens/ImGuiColorTextEdit              |

## 贡献

欢迎提交 Issue 和 Pull Request，共同完善 PLand。

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=engsr6982/PLand&type=Date)](https://star-history.com/#engsr6982/PLand&Date)
