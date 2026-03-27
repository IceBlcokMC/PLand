# PLand

- [简体中文](./README.md)
- English

PLand is a land protection system for BDS, developed based on LeviLamina.

## Features

- Multiple Land Types
    - [x] 2D Lands
    - [x] 3D Lands
    - [x] Sub-lands (supports multi-level nesting)
    - [x] Lease Mode
- Land Protection
    - [x] 60+ fine-grained permissions for land protection
    - [x] Dynamically extensible interception rules (e.g., block interaction, entity damage, etc.)
    - 4-role permission model
        - [x] Administrator (global highest privilege)
        - [x] Land Owner
        - [x] Land Member (individually configurable permissions)
        - [x] Land Visitor (individually configurable permissions)
    - Permission Layers
        - [x] Environment permissions (e.g., lightning, fluid flow, moss spread, etc.)
        - [x] Role permissions (player interaction-related permissions such as using items, opening containers, etc.)
    - Coverage
        - Covers 40+ low-level interaction events
        - Additional hooks to fix privilege escalation issues
- Gameplay Extensions:
    - [x] Economy System (supports dual economy systems)
        - [x] Independent pricing formula for each land type
        - [x] Dimension price multipliers
        - [x] Configurable discount and refund rates
    - [x] Land Teleportation
    - [x] Land Transfer
    - [x] Land Boundary Visualization
    - Land Constraints
        - [x] Dimension restricted regions (disallow land creation in specific areas)
        - [x] Configurable minimum distance between lands
        - [x] Minimum land size
        - [x] Land count limit
        - [x] Sub-land depth limit
        - [x] Dimension whitelist
- GUI Coverage:
    - [x] Encapsulated GUI forms for player-friendly operations
    - [x] Paginated forms with advanced filters
    - [x] Admin-exclusive management forms
- Multi-language Support
    - [x] Simplified Chinese (built-in)
    - [x] American English (AI translated)
    - [x] Russian (AI translated)
- Performance Optimization
    - [x] Hash-based optimization for land queries
    - [x] Sub-land hierarchy precomputation + caching
    - [x] Bidirectional tables for faster lookup and deletion
- Developer Related:
    - [x] Extensive event encapsulation for extensibility
    - [x] Service layer encapsulation for code reuse
    - [x] DevTool developer toolkit
        - [x] Land visualization
        - [x] Real-time editing
        - [x] Land hierarchy visualization

> For more details, please visit the documentation site: https://iceblcokmc.github.io/PLand/

## Project Structure

```bash
C:\PLand
├─assets # Plugin resource files (languages, texts)
│  └─lang # JSON language packs for multi-language support
│
├─docs # Project documentation
│  ├─dev # Developer documentation
│  └─md # User documentation
│
├─src
│  └─pland
│     ├─aabb # Spatial / math utilities
│     ├─drawer # Land renderer
│     │  └─detail # Backend integration implementations
│     ├─economy # Dual economy integration
│     ├─events
│     │  ├─domain # Domain events, data synchronization, state updates
│     │  ├─economy # Economy-related events
│     │  └─player # Player interaction events
│     ├─gui
│     │  ├─admin # Admin-specific forms
│     │  ├─common # Shared form components
│     │  └─utils # Form utilities
│     ├─infra # Infrastructure layer
│     │  └─migrator # Data migrator
│     ├─internal # Internal implementations
│     │  ├─adapter # Adapters
│     │  │  └─telemetry # Telemetry
│     │  ├─command # Commands
│     │  └─interceptor # Interceptors
│     │      ├─detail
│     │      └─helper
│     ├─land
│     │  ├─internal # Internal land implementations
│     │  ├─repo # Repository / land registry
│     │  │  └─internal # Internal registry implementation
│     │  └─validator # Land validator
│     ├─reflect # Reflection utilities
│     ├─selector # Selection implementation
│     │  └─land # Land selector implementation
│     ├─service # Service layer
│     └─utils # Utility classes
└─src-devtool # Developer tool
    ├─components # Reusable components
    ├─deps # Dependencies
    └─menus # Menus
        ├─helper # Help menus
        │  └─element # Menu elements
        └─viewer
            └─element
```

## License

This project is licensed under the AGPL-3.0 or later.

> The developers are not responsible for any consequences resulting from the use of this software. By using this project
> and its derivative versions, you assume all associated risks.

> Special thanks to the following open-source projects for their support and contributions:

| Project Name           | Project URL                                                 |
|:-----------------------|:------------------------------------------------------------|
| LeviLamina             | https://github.com/LiteLDev/LeviLamina                      |
| exprtk                 | https://github.com/ArashPartow/exprtk                       |
| LegacyMoney            | https://github.com/LiteLDev/LegacyMoney                     |
| iListenAttentively(闭源) | https://github.com/MiracleForest/iListenAttentively-Release |
| ImGui                  | https://github.com/ocornut/imgui                            |
| glew                   | https://github.com/nigels-com/glew                          |
| ImGuiColorTextEdit     | https://github.com/goossens/ImGuiColorTextEdit              |

## Contribution

Issues and Pull Requests are welcome to help improve PLand.

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=engsr6982/PLand&type=Date)](https://star-history.com/#engsr6982/PLand&Date)
