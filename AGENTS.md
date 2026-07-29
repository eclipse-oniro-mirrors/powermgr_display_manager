# AGENTS.md - OpenHarmony 显示能效管理（Display Manager）

## 1. 代码地图

本仓库实现 OpenHarmony 显示能效管理（Display Manager），SA ID 3308，进程名 `powermgr`，库 `libdisplaymgrservice.z.so`。仓库由**两个并列子模块**组成：`state_manager`（屏幕亮灭状态管理，承载 SA 3308）与 `brightness_manager`（亮度调节，作为库被 state_manager 调用）。核心职责是屏幕亮灭控制、亮度调节、亮度dimming、环境光-亮度计算。最重要的架构边界是**`state_manager` 持有屏幕状态机并通过 IPC 对外提供 `DisplayPowerMgrClient`，`brightness_manager` 是无状态机的能力库**。

### 非本项目维护的目录

以下目录属于其他团队或生成产物，不属于本项目维护范围，修改时请跳过：

- `state_manager/frameworks/ets/taihe/`：taihe FFI 生成产物（由 `taihe_ffi_gen` 工具生成，不要手改）
- `tools/ohos-displayManager/`：CLI 工具（独立 npm 包，仅在新增 CLI 命令时进入）

### 嵌套指引

本仓目前无嵌套 AGENTS.md / CLAUDE.md / rules / skills 文件。以下子模块可按需新建嵌套 AGENTS.md 以提供更具体的指引：

- `state_manager/`：屏幕亮灭状态管理子模块（承载 SA 3308 和 IPC 接口），可新建 `state_manager/AGENTS.md` 聚焦屏幕状态机（亮/灭/Dim）、灭屏策略、IPC stub/proxy 手写规则与 `_ipc_interface_code.h` 码值稳定性
- `brightness_manager/`：亮度调节库子模块（无 SA，被 state_manager 调用），可新建 `brightness_manager/AGENTS.md` 聚焦 dimming 动画时序、环境光-亮度曲线、`LightLuxBuffer` 过滤防抖与配置文件 schema
- `tools/ohos-displayManager/`：`display-shell` CLI 独立 npm 包，可新建 `tools/ohos-displayManager/AGENTS.md` 聚焦 CLI 命令注册、npm 发布流程
- `state_manager/utils/native/` + `state_manager/utils/`：工具层，影响范围较小，可新建嵌套 AGENTS.md
- `state_manager/service/etc/` + `brightness_manager/include/*_config_parser.h`：亮度配置默认值与解析器，schema 变更影响所有产品默认亮度行为，可单独新建 AGENTS.md 聚焦配置 schema 与产品差异
- `state_manager/test/` + `brightness_manager/test/`：测试组织目录，可新建嵌套 AGENTS.md 聚焦 unittest/systemtest/fuzztest 三类测试的运行约定

### 关键区域

#### state_manager（屏幕状态管理子模块）

- `state_manager/interfaces/inner_api/native/include/`：公共 C++ API 头文件，含 `display_power_mgr_client.h` 入口、`display_power_info.h` 数据模型、回调接口（`IDisplayPowerCallback` / `IDisplayBrightnessCallback` / `IDisplayBrightnessListener`）及对应 `*_ipc_interface_code.h` IPC 码枚举、对应 `*_stub.h`。
- `state_manager/frameworks/native/`：客户端 SDK 实现（`display_power_mgr_client.cpp`）。
- `state_manager/frameworks/napi/`：ArkTS NAPI 绑定。
- `state_manager/frameworks/ets/taihe/`：taihe FFI **生成产物**。
- `state_manager/service/zidl/`：IPC 接口实现（**本项目不使用 `.idl` 文件**，直接手写 stub/proxy）。
- `state_manager/service/native/include/` + `state_manager/service/native/src/`：服务端实现。
- `state_manager/service/etc/`：亮度配置文件。
- `state_manager/sa_profile/3308.json`：SA 3308 注册配置。
- `state_manager/utils/native/`：工具层。

#### brightness_manager（亮度调节库子模块）

- `brightness_manager/include/brightness_service.h`：亮度服务入口（库 API）。
- `brightness_manager/include/brightness_manager.h`：亮度管理器（核心调度）。
- `brightness_manager/include/brightness_action.h`：亮度动作执行（设置背光）。
- `brightness_manager/include/brightness_dimming.h`：亮度渐变动画。
- `brightness_manager/include/brightness_dimming_callback.h`：dimming 回调。
- `brightness_manager/include/calculation_manager.h` + `calculation_curve.h`：环境光-亮度计算。
- `brightness_manager/include/light_lux_manager.h` + `light_lux_buffer.h` + `ilight_lux_manager.h`：环境光采集与过滤。
- `brightness_manager/include/{brightness,lux_filter,lux_threshold,calculation,config_parser_base}_config_parser.h`：配置解析。
- `brightness_manager/include/brightness_param_helper.h` + `brightness_setting_helper.h`：参数与设置工具。
- `brightness_manager/include/brightness_manager_ext.h`：扩展接口。
- `brightness_manager/include/brightness_ffrt.h`：FFRT 任务封装。
- `brightness_manager/src/`：实现。
- `brightness_manager/test/unittest/`：单元测试。

#### 仓库顶层

- `displaymgr.gni`：3 个特性开关 + part 检测。
- `displaymanager.yaml` + `powermanager.yaml`：HiSysEvent 配置。
- `bundle.json`：构建配置。
- `tools/ohos-displayManager/`：`display-shell` CLI 命令行工具。
- `figures/`：架构图。

### Where to look

| 任务类型 | 先看哪里 |
|---|---|
| 公共 API 变更 | `state_manager/interfaces/inner_api/native/include/` 头文件 -> `state_manager/frameworks/native/` 实现 -> `state_manager/frameworks/napi/` ArkTS 绑定 -> `*.map` 版本脚本 |
| IPC 接口变更 | `state_manager/service/zidl/include/*.h` + `state_manager/service/zidl/src/*.cpp` + `state_manager/interfaces/inner_api/native/include/*_ipc_interface_code.h`（IPC 码枚举） |
| 屏幕亮灭流程 | `state_manager/service/native/include/display_power_mgr_service.h` + `state_manager/service/native/src/display_power_mgr_service.cpp` |
| 亮度调节 | `brightness_manager/include/brightness_service.h` + `brightness_manager.h` + `brightness_action.h` |
| 亮度 dimming | `brightness_manager/include/brightness_dimming.h` + `brightness_dimming_callback.h` |
| 环境光-亮度计算 | `brightness_manager/include/calculation_manager.h` + `calculation_curve.h` + `calculation_config_parser.h` |
| 环境光采集 | `brightness_manager/include/light_lux_manager.h` + `light_lux_buffer.h` + `lux_filter_config_parser.h` + `lux_threshold_config_parser.h` |
| 亮度配置 | `state_manager/service/etc/` + `brightness_manager/include/{brightness,lux_filter,lux_threshold,calculation,config_parser_base}_config_parser.h` |
| 屏幕状态回调 | `state_manager/interfaces/inner_api/native/include/idisplay_power_callback.h` + `display_power_callback_stub.h` |
| 亮度变化回调 | `state_manager/interfaces/inner_api/native/include/idisplay_brightness_callback.h` + `display_brightness_callback_stub.h` |
| 亮度监听回调 | `state_manager/interfaces/inner_api/native/include/idisplay_brightness_listener.h` + `display_brightness_listener_stub.h` |
| 灭屏策略 | `displaymgr.gni` 中 `display_manager_feature_poweroff_strategy` + `state_manager/service/native/` 灭屏路径 |
| 亮度扩展配置 | `displaymgr.gni` 中 `display_manager_feature_brightnessext` + `brightness_manager/include/brightness_manager_ext.h` |
| CLI 工具 | `tools/ohos-displayManager/` + `displaymgr.gni` 中 `display_manager_feature_support_display_cli` |
| 跨仓依赖调用 | `bundle.json` 中 `deps.components`（依赖 `power_manager` / `sensor` / `window_manager` / `graphic_2d` / `image_framework`） |
| 特性开关 | `displaymgr.gni` `declare_args()` 段 + part 检测段 |
| 新增/修改测试 | `state_manager/test/{unittest,systemtest,fuzztest}/` + `brightness_manager/test/unittest/` |
| 构建配置 | `bundle.json` + `displaymgr.gni` + 子目录 `BUILD.gn` |
| DFX（日志/事件） | `displaymanager.yaml` + `powermanager.yaml` + grep `HiSysEvent` 调用点 |

### 架构分层

```
应用层
  ├─ ArkTS 应用 -> state_manager/frameworks/napi (NAPI 绑定)
  │                └─ state_manager/frameworks/ets/taihe (taihe FFI 生成)
  └─ C++ 应用/系统组件 -> state_manager/interfaces/inner_api/native (完整 C++ API)
          ↓
客户端 SDK
  DisplayPowerMgrClient (state_manager/frameworks/native/display_power_mgr_client.cpp)
    -> IDisplayPowerMgr proxy (state_manager/service/zidl，手写)
        ↓ IPC (SystemAbility 3308)
服务端
  DisplayPowerMgrService (state_manager/service/native，继承 SystemAbility)
    ├─ 屏幕状态机 (亮/灭/Dim)
    ├─ 灭屏策略 (display_manager_feature_poweroff_strategy)
    └─ 调用 brightness_manager 库

brightness_manager（库，无 SA）
  BrightnessService (brightness_manager/include/brightness_service.h, 库入口)
    -> BrightnessManager (调度核心)
      ├─ BrightnessAction (设置背光，调用 HDI display)
      ├─ BrightnessDimming (亮度渐变动画)
      ├─ CalculationManager + CalculationCurve (环境光-亮度计算)
      ├─ LightLuxManager + LightLuxBuffer (环境光采集与过滤)
      ├─ BrightnessParamHelper + BrightnessSettingHelper (参数与设置)
      └─ BrightnessManagerExt (扩展接口，特性开关 brightnessext)
```

## 2. 知识路由

在规划或编辑前，先对任务分类，读取对应的代码路径和文档。

### Task-based routing

| 任务类型 | 读取 |
|---|---|
| 公共 API 新增/修改 | `state_manager/interfaces/inner_api/native/include/*.h` + `state_manager/frameworks/native/` + `state_manager/frameworks/napi/` + `*.map` 版本脚本 |
| IPC 接口变更 | `state_manager/service/zidl/include/*.h` + `state_manager/service/zidl/src/*.cpp` + `state_manager/interfaces/inner_api/native/include/*_ipc_interface_code.h`（**本项目不使用 `.idl`，直接手写 proxy/stub**） |
| 屏幕亮灭流程 | `state_manager/service/native/include/display_power_mgr_service.h` + `state_manager/service/native/src/display_power_mgr_service.cpp` |
| 亮度调节 | `brightness_manager/include/brightness_service.h` + `brightness_manager.h` + `brightness_action.h` + `brightness_manager/src/brightness_*.cpp` |
| 亮度 dimming | `brightness_manager/include/brightness_dimming.h` + `brightness_dimming_callback.h` + `brightness_manager/src/brightness_dimming.cpp` |
| 环境光-亮度计算 | `brightness_manager/include/calculation_manager.h` + `calculation_curve.h` + `calculation_config_parser.h` + `brightness_manager/src/calculation_*.cpp` |
| 环境光采集与过滤 | `brightness_manager/include/light_lux_manager.h` + `light_lux_buffer.h` + `lux_filter_config_parser.h` + `lux_threshold_config_parser.h` + HDI `sensor` |
| 亮度配置文件变更 | `state_manager/service/etc/` + `brightness_manager/include/{brightness,lux_filter,lux_threshold,calculation,config_parser_base}_config_parser.h` + `brightness_manager/include/config_parser.h` |
| 屏幕状态回调 | `state_manager/interfaces/inner_api/native/include/idisplay_power_callback.h` + `state_manager/service/zidl/include/display_power_callback_proxy.h` + `display_power_callback_stub.h` |
| 亮度变化回调 | `state_manager/interfaces/inner_api/native/include/idisplay_brightness_callback.h` + `display_brightness_callback_*.{h,cpp}` |
| 亮度监听回调 | `state_manager/interfaces/inner_api/native/include/idisplay_brightness_listener.h` + `display_brightness_listener_*.{h,cpp}` |
| 灭屏策略 | `displaymgr.gni` 中 `display_manager_feature_poweroff_strategy` + `state_manager/service/native/` 灭屏路径 |
| 亮度扩展配置 | `displaymgr.gni` 中 `display_manager_feature_brightnessext` + `brightness_manager/include/brightness_manager_ext.h` |
| CLI 工具 | `tools/ohos-displayManager/` + `displaymgr.gni` 中 `display_manager_feature_support_display_cli` |
| 跨仓依赖调用 | `bundle.json` 中 `deps.components` + grep 调用 `power_manager` / `sensor` / `window_manager` / `graphic_2d` 的位置 |
| 权限校验 | grep `AccessTokenKit` / `VerifyAccessToken` 调用点 |
| 特性开关 | `displaymgr.gni` `declare_args()` + part 检测段（`sensors_sensor` / `hiviewdfx_hisysevent` / `hiviewdfx_hiview`） |
| 新增/修改测试 | `state_manager/test/{unittest,systemtest,fuzztest}/` + `brightness_manager/test/unittest/` |
| 构建配置 | `bundle.json`（依赖、syscap、rom/ram）+ `displaymgr.gni`（特性开关、part 检测）+ 各 `BUILD.gn` |
| DFX（日志/事件） | `displaymanager.yaml` + `powermanager.yaml`（HiSysEvent 配置）+ grep `HiSysEvent` 调用点 |

### Path-based routing

| 修改路径 | 需了解的上下文 |
|---|---|
| `state_manager/interfaces/inner_api/native/` | 所有 C++ 消费者的公共 API，变更需同步 `state_manager/frameworks/napi/`、`state_manager/frameworks/ets/taihe/` 两套绑定并检查 `*.map` 版本脚本 |
| `state_manager/service/zidl/` | IPC 接口层，**手写 proxy/stub**（不通过 `.idl` 生成），修改接口签名需同时改 `_stub`/`_proxy` 两套文件并同步 `_ipc_interface_code.h` 码值 |
| `state_manager/service/native/include/display_power_mgr_service.h` | 服务端核心类，修改前需理解屏幕状态机、灭屏策略、与 brightness_manager 的调用关系 |
| `state_manager/service/etc/` | 亮度配置默认值，影响所有产品的默认亮度行为 |
| `state_manager/sa_profile/3308.json` | SA 3308 注册配置 |
| `brightness_manager/include/brightness_service.h` | 亮度库入口，被 `DisplayPowerMgrService` 调用 |
| `brightness_manager/include/brightness_manager.h` | 亮度调度核心，修改影响所有亮度调节场景 |
| `brightness_manager/include/brightness_action.h` | 亮度动作执行，直接调用 HDI display，修改需评估 HDI 兼容性 |
| `brightness_manager/include/brightness_dimming.h` | 亮度渐变动画，修改影响动画流畅度 |
| `brightness_manager/include/calculation_manager.h` + `calculation_curve.h` | 环境-亮度计算核心，修改影响自动亮度体验 |
| `brightness_manager/include/light_lux_manager.h` | 环境光采集核心，HDI 上报频率和过滤策略影响系统负载 |
| `brightness_manager/include/*_config_parser.h` | 配置解析器，schema 变更需评估配置文件兼容性 |
| `state_manager/frameworks/ets/taihe/` | taihe FFI **生成产物**，不要手改 |
| `displaymgr.gni` | 3 个特性开关 + part 检测，开关变更需同步 `bundle.json` |
| `state_manager/utils/native/` + `state_manager/utils/` | 工具层，影响范围较小 |
| `displaymanager.yaml` + `powermanager.yaml` | HiSysEvent 事件定义，新增事件需在此声明 |

### Vocabulary-based routing

当任务、issue、日志、API 名称中出现以下术语时，先理解其含义和风险再动手：

| 术语 | 含义与风险 | 读取 |
|---|---|---|
| DisplayPowerMgr | Display Power Manager，本服务简称，SA 3308 | 本文件 + `state_manager/sa_profile/3308.json` |
| state_manager | 屏幕状态管理子模块，承载 SA 3308 和 IPC 接口 | `state_manager/` |
| brightness_manager | 亮度调节库子模块，无 SA，被 state_manager 调用 | `brightness_manager/` |
| BrightnessService | 亮度服务入口（库 API） | `brightness_manager/include/brightness_service.h` |
| BrightnessManager | 亮度调度核心 | `brightness_manager/include/brightness_manager.h` |
| BrightnessAction | 亮度动作执行（设置背光） | `brightness_manager/include/brightness_action.h` |
| BrightnessDimming | 亮度渐变动画 | `brightness_manager/include/brightness_dimming.h` |
| CalculationManager | 环境-亮度计算管理器 | `brightness_manager/include/calculation_manager.h` |
| CalculationCurve | 环境-亮度曲线 | `brightness_manager/include/calculation_curve.h` |
| LightLuxManager | 环境光管理器 | `brightness_manager/include/light_lux_manager.h` |
| LightLuxBuffer | 环境光缓冲（防抖） | `brightness_manager/include/light_lux_buffer.h` |
| BrightnessConfigParser | 亮度配置解析器 | `brightness_manager/include/brightness_config_parser.h` |
| DisplayPowerCallback | 屏幕状态回调（亮/灭/Dim） | `state_manager/interfaces/inner_api/native/include/idisplay_power_callback.h` |
| DisplayBrightnessCallback | 亮度变化回调 | `state_manager/interfaces/inner_api/native/include/idisplay_brightness_callback.h` |
| DisplayBrightnessListener | 亮度监听回调 | `state_manager/interfaces/inner_api/native/include/idisplay_brightness_listener.h` |
| Dim 屏 | 屏幕处于 Dim 状态（半亮待机） | `state_manager/service/native/include/display_power_mgr_service.h` |
| 灭屏策略 | `display_manager_feature_poweroff_strategy` 控制的灭屏流程优化策略 | `displaymgr.gni` + `state_manager/service/native/` |
| 亮度扩展 | `display_manager_feature_brightnessext` 控制的扩展亮度配置 | `displaymgr.gni` + `brightness_manager/include/brightness_manager_ext.h` |
| SystemAbility / SA | OpenHarmony 系统服务框架，本服务 SA ID 3308 | `state_manager/sa_profile/3308.json` |
| sptr | OpenHarmony 共享指针（`refbase.h`），非 `std::shared_ptr` | 全项目 |
| ErrCode | 错误码返回类型 | `state_manager/interfaces/inner_api/native/include/display_power_errors.h`（如有） |
| FFRT | Function Flow Runtime，华为协作式异步任务调度框架 | `brightness_manager/include/brightness_ffrt.h` |
| HWTEST_F | OpenHarmony 测试用例宏 | 全项目测试代码 |
| 特性开关 | `displaymgr.gni` 中 `declare_args()` 控制条件编译 | `displaymgr.gni` |
| part 检测 | `displaymgr.gni` 中检测 `global_parts_info` 决定是否编译某 part 支持 | `displaymgr.gni` |
| powermgr 进程 | 本仓与 power_manager/battery_manager/thermal_manager 共享同一进程，多 SA 共进程 | `state_manager/sa_profile/3308.json` |
| 跨仓依赖 | 本仓调用 `power_manager`（电源状态查询）/ `sensor`（环境光）/ `window_manager`（窗口管理）/ `graphic_2d`（图形）等 | `bundle.json` 中 `deps.components` |

在计划阶段，必须声明：
- **任务分类**（如：公共 API 变更 / IPC 接口变更 / 屏幕亮灭流程变更 / 亮度调节变更 / dimming 变更 / 环境-亮度计算变更 / 配置变更 / 新增特性 / 测试修改）
- **已读取的代码路径和文档**（具体到文件路径）
- **发现的约束**（架构不变量、禁止事项、特性开关依赖、跨仓依赖）
- **是否需要同步修改其他层**：
  - 公共 API 变更 -> 同步 NAPI / taihe 两套绑定 + `*.map` 版本脚本 + IPC 码枚举
  - `state_manager/service/zidl/` 接口变更 -> 同步 stub + proxy + `_ipc_interface_code.h`
  - 特性开关变更 -> 同步 `bundle.json` 的 `features` 列表
  - 跨仓依赖调用变更 -> 检查 `power_manager` / `sensor` / `window_manager` 等是否受影响
  - `displaymanager.yaml` / `powermanager.yaml` 变更 -> 同步 HiSysEvent 事件定义
  - 亮度配置文件变更 -> 评估对产品亮度行为的影响

## 3. 约束边界

### 架构不变量

- **客户端不持有业务状态**：`DisplayPowerMgrClient` 仅做 IPC 转发，屏幕状态由 `DisplayPowerMgrService` 持有
- **`state_manager` 是 SA 入口，`brightness_manager` 是被调用的库**：禁止 brightness_manager 反向调用 state_manager 的 IPC 接口
- **HDI 是屏幕状态和环境光的唯一真相源**：所有屏幕状态变更必须经 HDI 上报或服务端决策，环境光数据必须从 HDI sensor 采集
- **公共 API 表达稳定能力意图，不暴露 HDI 句柄或服务端内部字段**
- **权限校验在服务端入口完成**：`setDisplayBrightness` / `setDisplayState` 等写操作必须有权限校验
- **IPC 调用必须设置死亡通知**，回调订阅必须支持客户端异常断开后自动清理
- **跨进程回调必须用 Parcelable 序列化**
- **DFX（日志、HiSysEvent、HiCollie、错误码）必须观测所有屏幕状态变更、亮度变更和 HDI 调用**
- **HDI 上下线必须能自动重连**
- **亮度调节必须有 dimming 动画**（除非显式禁用），避免突跳影响体验
- **环境光-亮度计算必须用配置文件中的曲线**，禁止硬编码阈值

### 禁止事项

- **不要修改公共 API 签名、错误码、权限行为或屏幕状态语义**，除非任务明确要求；修改 `*.map` 中已有符号的可见性属于破坏性变更
- **不要直接编辑 `state_manager/frameworks/ets/taihe/` 下的生成文件**，应修改 `.taihe` 源文件后由构建系统重生成
- **不要为通过测试而删除日志、HiSysEvent 事件、错误码或诊断信息**
- **不要绕过现有的 DFX、安全、兼容性检查**
- **不要直接修改 HDI 接口**：本仓是 HDI 消费方，HDI 接口由 `drivers_interface_display` / `drivers_interface_light` 等仓维护
- **不要在 `state_manager/service/zidl/` 中只改 stub 不改 proxy**（或反之）：两套必须同步
- **不要在 `brightness_manager/` 中反向调用 `state_manager/` 的 IPC 接口**：单向依赖
- **不要在 `brightness_manager/` 中直接调用 HDI 接口**：所有 HDI 访问必须经 `BrightnessAction` 封装
- **不要硬编码亮度阈值或环境光-亮度曲线**：必须用配置文件
- **不要在亮度调节路径中执行阻塞 IO**：影响动画流畅度
- **不要在 `powermgr` 进程中执行长耗时同步操作**：本进程与 power/battery/thermal 共享
- **不要引入新的生产依赖**而不经过 `bundle.json` 评审
- **不要绕过 `DisplayPowerMgrService` 直接修改屏幕状态字段**

### 需确认后再修改

- **公共 API 签名变更**（需确认兼容性影响和版本策略，更新 `*.map` 版本脚本）
- **IPC 码值变更**（`*_ipc_interface_code.h`，新增接口追加新码值，不能复用或调整已有码值）
- **`displaymgr.gni` 特性开关默认值翻转**（特别是 `display_manager_feature_poweroff_strategy` / `display_manager_feature_brightnessext`）
- **屏幕状态机新增状态或转移规则**（需评估所有依赖状态的业务：UI、应用框架、系统服务）
- **亮度配置文件 schema 变更**（需确认产品团队评审，影响所有产品默认亮度行为）
- **环境光-亮度曲线变更**（需确认 UX 团队评审，影响自动亮度体验）
- **跨仓依赖调用变更**（需确认 `power_manager` / `sensor` / `window_manager` / `graphic_2d` 接口稳定性）
- **`displaymgr.gni` part 检测段变更**（需确认 `bundle.json` 的 `deps.components` 同步）
- **新增外部依赖**（需确认许可证、包大小、`bundle.json` 同步）
- **`displaymanager.yaml` / `powermanager.yaml` 中 HiSysEvent 事件变更**（需确认 DFX 团队评审）
- **CLI 工具命令变更**（需确认 `tools/ohos-displayManager/` npm 包版本同步）

### 项目特定陷阱

- **手写 proxy/stub 同步**：本项目不使用 `.idl`，修改接口签名必须同时改 stub（服务端入参解码）和 proxy（客户端出参编码），漏改一侧会导致 IPC 数据错乱
- **IPC 码值稳定性**：`*_ipc_interface_code.h` 中的码值是 ABI 契约，新增接口必须追加新码值，不能复用或调整已有码值，否则旧客户端会调用错误接口
- **双模块单向依赖**：`state_manager` 依赖 `brightness_manager`，反向调用会导致循环依赖
- **特性开关的 `defines` 双写**：`displaymgr.gni` 中开关既要改 `declare_args()` 默认值，也要在对应 `if` 块更新 `defines`，漏掉一处会导致特性开关失效
- **`powermgr` 多 SA 共进程**：SA 3301/3302/3303/3308 在同一进程，一个 SA 崩溃会拖死全部
- **亮度 dimming 动画时序**：dimming 是异步动画，调用方连续触发 dimming 需有取消和合并策略
- **环境光过滤的防抖**：`LightLuxBuffer` 的过滤窗口大小影响亮度响应速度，调整需评估抖动和延迟的权衡
- **环境光-亮度曲线的产品差异**：不同产品有不同的曲线配置，修改时需评估对其他产品的影响
- **HDI sensor 上报频率**：环境光 sensor 上报频率高，处理需异步非阻塞，否则会导致 sensor 数据堆积
- **`state_manager/service/etc/` 配置文件**：修改默认亮度行为需评估对开机首屏的影响
- **`tools/ohos-displayManager/` 是独立 npm 包**：CLI 命令变更需同步发布 npm 包版本
- **跨仓依赖时序**：`state_manager` 启动时调用 `power_manager` 查询电源状态，`power_manager` 未就绪时需有重试机制

## 4. 验证闭环

### 最小验证

```bash
# 构建 display_manager 子系统（从 OpenHarmony 根目录执行）
./build.sh --product-name rk3568 --build-target display_manager

# 构建全部测试
./build.sh --product-name rk3568 --build-target display_manager_test
```

### 任务特定验证

| 任务类型 | 验证命令 |
|---|---|
| 公共 API 变更 | `./build.sh --product-name rk3568 --build-target display_manager` + 同步构建依赖本仓的下游仓 + 跑 `state_manager/test/unittest/` |
| IPC 接口变更 | 重新构建 proxy/stub + 跑 `state_manager/test/unittest/` + `state_manager/test/fuzztest/` |
| 屏幕亮灭流程 | 跑 `state_manager/test/systemtest/` 亮灭屏场景 + 真机验证灭屏策略 |
| 亮度调节 | 跑 `brightness_manager/test/unittest/` + 真机验证亮度调节流畅度 |
| 亮度 dimming | 跑 `brightness_manager/test/unittest/` dimming 用例 + 真机验证动画流畅度 |
| 环境-亮度计算 | 跑 `brightness_manager/test/unittest/` calculation 用例 + 真机验证自动亮度曲线 |
| 环境光采集 | 跑 `brightness_manager/test/unittest/` lux 用例 + 真机验证环境光响应 |
| 亮度配置变更 | 真机验证默认亮度行为 + 评估对其他产品的影响 |
| 回调订阅变更 | 跑 `state_manager/test/fuzztest/` 回调相关 fuzzer |
| 特性开关翻转 | 重新构建 `display_manager` 全量 + 验证 `bundle.json` 中 `features` / `deps.components` 同步 |
| 跨仓依赖调用 | 同步构建 `power_manager` / `sensor` / `window_manager` + 验证调用接口未变 |
| CLI 工具 | 跑 `tools/ohos-displayManager/tests/` + 真机验证 CLI 命令 |

### Done 定义

- 构建通过（子系统 + 单元测试 + 模糊测试 + brightness_manager 库 + CLI 工具）
- 无新增编译警告
- 变更范围与任务要求一致，未夹带未关联的重构
- IPC 接口变更已同步 stub/proxy/IPC 码三处
- 特性开关变更已同步 `displaymgr.gni` + `bundle.json`
- 公共 API 变更已同步 NAPI / taihe / `*.map` 版本脚本
- 亮度配置变更已评估对产品亮度行为的影响
- 跨仓依赖调用变更已评估对下游仓的影响
- 涉及 dimming 动画/环境光过滤的变更已评估对体验的影响

### 最终响应期望

完成报告必须包含：
1. 修改的文件清单（按 `file:line` 引用）
2. 任务分类与对应验证命令的执行结果
3. 是否触发跨层同步修改（NAPI / taihe / `*.map` / IPC 码 / `bundle.json` / `displaymanager.yaml` / `powermanager.yaml`）
4. 是否影响特性开关默认值或跨仓依赖
5. 是否触及架构不变量或需确认事项
6. 涉及双模块的变更需额外说明依赖方向（state_manager -> brightness_manager）

### 无法验证时

如果构建环境不可用，列出应执行的命令并说明预期结果，明确标注「未验证」字样，不能假称已通过。涉及屏幕亮灭/亮度动画/环境光采集的变更，必须人工复核代码逻辑并说明无法在沙箱验证的限制。
