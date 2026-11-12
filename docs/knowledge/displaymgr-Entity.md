# 业务实体知识

display_manager 模块核心业务实体的完整知识定义，涵盖静态概念、动态运行、规则约束与代码映射。路径均相对仓根（`base/powermgr/display_manager`）。术语释义见 [术语表](glossary.md)，实体依赖的机制见 [技术术语知识](displaymgr-Technology.md)，整体边界见 [业务背景](business-context.md)。

## 屏幕显示状态（DisplayState）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 屏幕显示状态 / DisplayState |
| 实体定义 | 单块屏幕在显示能效域的逻辑状态单元；身份边界：它是本服务对外承诺的状态语义，内部需换算为 Rosen 的 display 状态与 screen 电源两套枚举 |
| 核心特征 | 枚举 `DISPLAY_OFF=0/DIM=1/ON=2/SUSPEND=3/DELAY_OFF=4/DOZE/DOZE_SUSPEND/UNKNOWN`；配套 `PowerOffStrategy`（ALL=0/SPECIFIC=1）与 `MultiScreenStateChangeReason`（`state_manager/interfaces/inner_api/native/include/display_power_info.h:26-35`） |
| 类型/分类 | 公共数据模型（inner_api 头，全仓共享；NAPI/多屏 API 均以其为参数） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 上游（power_manager 状态机）请求 → `SetDisplayStateInner` 校验 → `ScreenController::UpdateState` 演进 → `ScreenAction` 写 Rosen → 回调 `OnStateChanged` → `NotifyStateChangeCallback` 推给 `IDisplayPowerCallback` 订阅方（`state_manager/service/native/src/display_power_mgr_service.cpp:265-324`） |
| 状态流转 | 同态幂等返回 true（例外：reason=PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF 允许重复处理，`screen_controller.cpp:108-111`）；OFF→DIM 拒绝（:112-115）；DIM→ON 仅本地更新（:117-120）；SUSPEND/DOZE 在通知订阅方时归一化为 OFF；`UpdateCachedState` 供多屏外部同步（:317-319） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | PRE_BRIGHT/PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF reason 跳过后续流程（`IsNeedSkipNextProc`，`screen_controller.cpp:283-291`）；PRE_BRIGHT_AUTH_SUCCESS 跳过 Rosen 直达回调（`screen_action.cpp:86-92`）；DELAY_OFF 走延迟任务 + 失败 Undo |
| 并发约束 | 每屏一个 `ScreenController`，`state_` 为 atomic + `mutexState_` 保护（`screen_controller.h:99-100`） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `DisplayPowerMgrService::controllerMap_`（每 displayId 一份）；`BrightnessService` 缓存 `mState` 用于 `IsScreenOn` 门控 |
| 下游影响 | 屏幕状态回调订阅方（power_manager 等）、battery_statistics（SCREEN_STATE HiSysEvent）、自动亮度启停 |
| 平级关联 | reason 语义来自 power_manager `StateChangeReason`（`power_state_machine_info.h`）；Rosen 枚举映射只在 ScreenAction 三处 Parse 函数 |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `DisplayState`/`PowerOffStrategy`/`MultiScreenStateChangeReason`（`display_power_info.h`） |
| 核心方法 | `ScreenController::UpdateState/OnStateChanged`、`ScreenAction::ParseDisplayState/ParseScreenPowerState/ParseSpecialReason`（`screen_action.cpp:94-115,189-246`） |

## 显示能效服务（DisplayPowerMgrService / DisplaySystemAbility，SA 3308）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 显示能效系统服务 / DisplayPowerMgrService（SA 3308） |
| 实体定义 | powermgr 进程内的显示能效服务端；身份边界：IPC 41 方法的实现者与屏幕状态的唯一持有方，本身不做亮度算法（委托 BrightnessManager） |
| 核心特征 | `DisplayPowerMgrService : DisplayPowerMgrStub`（`state_manager/service/native/include/display_power_mgr_service.h:49`）；`DisplaySystemAbility : SystemAbility` 承载生命周期（`REGISTER_SYSTEM_ABILITY_BY_ID(..., DISPLAY_MANAGER_SERVICE_ID, true)`，`display_system_ability.cpp:26`，run-on-create: true）；常驻静态亮度界 `BRIGHTNESS_MIN/DEFAULT/MAX`（:81-83）与 `isBootCompleted_`（:84） |
| 类型/分类 | 系统服务（SA）；由 `DelayedSpSingleton` 持有，`DisplaySystemAbility` 发布 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | SA 上线 → OnStart 监听 SA 4607（Rosen DMS）/CES/KV（`display_system_ability.cpp:28-35`）→ 三依赖就绪后 `Init()`（FFRT 队列、displayId 重试 3×50ms 回退 0、`BrightnessManager::Get().Init(BRIGHTNESS_MAX, BRIGHTNESS_MIN)`（FUZZ_COV_TEST 除外）、`controllerMap_` 构建、`RegisterBootCompletedCallback`）→ `Publish` → `HandleBootBrightness`（:87-120,37-78） |
| 状态流转 | Init → 常驻服务 → OnStop 时 `Deinit`（解绑 observer、`BrightnessManager::Get().DeInit()`）；`Reset` 释放队列与延迟任务句柄（:130-147）；key 方法行图：HandleBootBrightness 149 / RegisterSettingObservers 178 / SetDisplayStateInner 265 / SetBrightnessInner 358 / AutoAdjustBrightnessInner 488 / RegisterCallbackInner 514 / BoostBrightnessInner 538 / NotifyStateChangeCallback 596 / SetScreenPowerOffStrategyInner 1158 / SetScreenDisplayState 1193 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 写类入口统一 `Permission::IsSystem()`；`SetBrightnessInner` 经 `GetSafeBrightness` 钳制后转 BrightnessManager（:358-369）；`AdjustBrightnessInner` 先 BrightnessManager 失败后回退 controller->SetBrightness（:459-476）；自动亮度设置写入经 `autoBrightnessQueue_` 定时器去重（:488-512）；回调单槽 `callback_` + 死亡recipient（:514-536）；IPC 入口 `NoCoroutineSwitchGuard` + `DisplayXCollie` 看护 |
| 并发约束 | `mutex_` 保护 callback_/controllerMap_ 相关路径；`autoBrightnessMutex_` 串行化自动亮度开关+设置写入（:493-495）；`screenOffDelayTaskMutex_` 保护延迟灭屏句柄 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `DisplaySystemAbility`（发布与依赖调度）；`DisplayCommonEventManager`（开机亮度触发） |
| 下游影响 | `ScreenController`（状态）、`BrightnessManager`（亮度）、`IDisplayPowerCallback` 订阅方、battery_statistics（HiSysEvent） |
| 平级关联 | 与 power/battery/thermal SA 共进程（3301/3302/3303/3308） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `DisplayPowerMgrService`（`state_manager/service/native/include/display_power_mgr_service.h` + `src/display_power_mgr_service.cpp`）、`DisplaySystemAbility`（`src/display_system_ability.cpp`） |
| 核心方法 | `Init/Deinit/Reset`、`SetDisplayStateInner/UndoSetDisplayStateInner`、`SetBrightnessInner`、`AutoAdjustBrightnessInner`、`RegisterCallbackInner`、`SetScreenPowerOffStrategyInner`、`SetScreenDisplayState`（直接调 Rosen `SetScreenPowerById` + `SetScreenOnBrightness`，:1193-1220） |

## 显示能效客户端（DisplayPowerMgrClient）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 显示能效客户端 SDK / DisplayPowerMgrClient |
| 实体定义 | 面向系统组件/NAPI/CLI 的进程内调用入口；身份边界：纯转发者，唯一本地状态是 proxy 指针、死亡通知与最近错误码，不缓存业务状态 |
| 核心特征 | `DelayedRefSingleton`（`state_manager/interfaces/inner_api/native/include/display_power_mgr_client.h:37-38`）；常量 `BRIGHTNESS_OFF{0}/DEFAULT{102}/MAX{255}/MIN{1}`（:156-159）；`std::atomic<DisplayErrors> lastError_`（:161）；多屏 API 与 `DISPLAY_ID_ALL = UINT64_MAX`（:89-133）；`ENABLE_SCREEN_POWER_OFF_STRATEGY` 下持有 `token_`（:165-167） |
| 类型/分类 | inner_api 公共 C++ SDK（客户端实现 `state_manager/frameworks/native/display_power_mgr_client.cpp`） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 各方法 → `GetProxy()`（首次 `GetSystemAbilityManager` → `GetSystemAbility(3308)`，失败置 `ERR_CONNECTION_FAIL`；挂 `DisplayDeathRecipient`）→ proxy IPC → bool/retCode 出参；服务死亡 → `OnRemoteDied` → 移除 recipient，下次调用重连（`frameworks/native/display_power_mgr_client.cpp:53-70,91`） |
| 状态流转 | proxy null ↔ connected；`lastError_` 每次失败覆盖、`GetError()` 原子交换读后复位 ERR_OK（:508） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | `SetDisplayState` 仅供 PowerMgrService 使用（:44-47 注释）；数值查询失败返回 0/默认值；多屏 API 失败路径经 `GetError()` 取错误码 |
| 并发约束 | 多线程共享单例；连接/重连持 `mutex_` |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | NAPI `brightness` 模块（`state_manager/frameworks/napi/brightness_module.cpp`）、taihe 生成层、power_manager、CLI（`tools/ohos-displayManager/src/cli_command.cpp`） |
| 下游影响 | `IDisplayPowerMgr` proxy（IDL 生成物）；`.idl` 变更需同步其调用面 |
| 平级关联 | `DisplayDeathRecipient` 内嵌类（`DISPLAY_SERVICE_DEATH_UT` 宏控制 private 可见性供单测，:135-149） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `DisplayPowerMgrClient` |
| 核心方法 | `GetProxy/OnRemoteDied/GetError`；状态族 `SetDisplayState/SetScreenDisplayState/SetScreenPowerOffStrategy`；亮度族 `SetBrightness/SetForcedBrightness/OverrideBrightness/BoostBrightness/DiscountBrightness/GetDeviceBrightness`；监听族 `RegisterCallback/RegisterDataChangeListener/SetLightBrightnessThreshold/RegisterMultiScreenDisplayStateCallback`；`RunJsonCommand` |

## 屏幕控制器（ScreenController）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 屏幕控制器 / ScreenController |
| 实体定义 | 单块屏幕的状态机与亮度兜底执行器；身份边界：以 displayId 为身份，`controllerMap_` 一屏一份；承载状态演进规则与 reason 语义，真实写操作全部委托 `ScreenAction` |
| 核心特征 | 成员：`std::atomic<DisplayState> state_`、`mutexState_`、`cachedSettingBrightness_{102}`、`discount_`、override/boost 标志、`action_`/`animator_`（`state_manager/service/native/include/screen_controller.h:99-113`）；内嵌 `AnimateCallbackImpl : AnimateCallback`（:36-47）；静态键 `SETTING_BRIGHTNESS_KEY`（:97） |
| 类型/分类 | 服务端内部实体（非 IPC 可见）；多屏形态下增加 `screenLock_`（ffrt::mutex）与 `UpdateMultiScreenState`（:56-59） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 构造时从 `action_->GetDisplayState()` 读初值（非多屏）；状态请求经 `UpdateState`（规则见 DisplayState 实体）；亮度请求 `SetBrightness/Discount/Override/Restore/Boost`（兜底路径）；动画回调把设置亮度持久化（`AnimateCallbackImpl` → SetSettingBrightness）；`OnStateChanged` 完成回调链：`action_->SetDisplayPower` → PRE_BRIGHT 跳过 → `SkipNotify` → ON 时 `pms->SetScreenOnBrightness` → 归一化 → `NotifyStateChangeCallback`（`screen_controller.cpp:293-323`） |
| 状态流转 | `state_` 仅经 `UpdateState`（:102-158，持锁）/`UpdateCachedState` 变更；boost 经 FFRT 延迟任务自动 CancelBoost（:244-276） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 亮度门控 `CanSetBrightness = IsScreenOn && !overridden && !boosted`；`SetBrightness`（:174-183）的条件 `if (CanSetBrightness())` 与其日志"Cannot set brightness"极性疑似与 `BrightnessService` 的 `!CanSetBrightness()` 相反——该方法仅作 `AdjustBrightnessInner` 回退路径（`display_power_mgr_service.cpp:471-475`），修改前需确认意图 |
| 并发约束 | `mutexState_` 保护状态读写；全局 `g_queue`（"screen_controller"）承载 boost 取消任务（:259-261）；多屏 `GetScreenLock` 串行化同屏操作 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `DisplayPowerMgrService::controllerMap_`；`DelayedSpSingleton<DisplayPowerMgrService>` 反向引用（OnStateChanged 中） |
| 下游影响 | `ScreenAction`（状态/电源写）、settingsdata（设置亮度）、订阅方回调 |
| 平级关联 | `GradualAnimator`（"BrightnessController_<id>" 实例，亮度兜底动画）；与 `BrightnessService` 是并列的两套亮度执行路径（主路径 vs controller 回退） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `ScreenController`（`state_manager/service/native/include/screen_controller.h` + `src/screen_controller.cpp`） |
| 核心方法 | `UpdateState/OnStateChanged/SkipNotify/IsNeedSkipNextProc`、`SetBrightness/UpdateBrightness/DiscountBrightness/OverrideBrightness/BoostBrightness`、`GetScreenOnBrightness`、`UpdateMultiScreenState` |

## 屏幕动作（ScreenAction）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 屏幕动作执行器 / ScreenAction |
| 实体定义 | Rosen 访问的屏幕侧封装；身份边界：displayId 绑定，只做"枚举映射 + 身份重置 + 转发 + DFX 上报"，不含业务决策 |
| 核心特征 | `ScreenAction(uint32_t displayId)`；静态 `GetDefaultDisplayId/GetAllDisplayId`（`state_manager/service/native/src/screen_action.cpp:36-54`）；`coordinated_` 协同屏标志；`ENABLE_SCREEN_POWER_OFF_STRATEGY` 下引用 `MiscellaneousDisplayPowerStrategy`（:260-263） |
| 类型/分类 | 服务端内部 Action 层实体 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `SetDisplayState(state, callback)`：ParseDisplayState → `ResetCallingIdentity` → Rosen `DisplayManagerLite::SetDisplayState`（回调里 Rosen 枚举 → DisplayState、`WriteHiSysEvent`、callback）→ `SetCallingIdentity` → SCREEN_STATE HiSysEvent（:139-187）；`SetDisplayPower`：协同+TIMEOUT 走 `SetSpecifiedScreenPower`，否则经策略换算 reason 后 `SetScreenPowerForAll`（:248-264） |
| 状态流转 | 无自身状态机；`GetDisplayState` 从 Rosen `ScreenManagerLite::GetScreenPower` 实时读（:61-84） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 三处映射（display 状态/screen 电源/reason）必须与 DisplayState 演进同步；锁屏等待超时上报阈值：亮屏 300ms、灭屏 2000ms（`WriteHiSysEvent`，:117-137） |
| 并发约束 | Rosen 调用前后身份重置必须配对；无锁（无状态） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `ScreenController::action_`、`DisplayPowerMgrService`（displayId 枚举） |
| 下游影响 | Rosen DMS（真实屏幕）、HiSysEvent（SCREEN_STATE/DMS_WAIT_LOCKSCREEN_TIMEOUT） |
| 平级关联 | `BrightnessAction`（背光侧同层封装）；`MiscellaneousDisplayPowerStrategy`（灭屏策略换算） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `ScreenAction`（`state_manager/service/native/include/screen_action.h:32` + `src/screen_action.cpp`）、`MiscellaneousDisplayPowerStrategy`（`include/miscellaneous_display_power_strategy.h:26-37`） |
| 核心方法 | `SetDisplayState/SetDisplayPower/GetDisplayState/EnableSkipSetDisplayState/WriteHiSysEvent/ParseDisplayState/ParseScreenPowerState/ParseSpecialReason` |

## 亮度管理器门面（BrightnessManager / BrightnessManagerExt）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 亮度管理器 / BrightnessManager |
| 实体定义 | state_manager 进入亮度库的唯一入口门面；身份边界：按 screenId 路由到 `BrightnessService`（默认屏核心）或 `BrightnessManagerExt`（产品扩展/多屏），自身无亮度逻辑 |
| 核心特征 | `static BrightnessManager& Get()` 单例；非多屏形态成员 `BrightnessManagerExt mBrightnessManagerExt`；多屏形态 `map<uint32_t, shared_ptr<BrightnessManagerExt>> mMultiBrtMgrExt` + `shared_mutex` + `MultiScreenListener`（Rosen IScreenListener，屏连接时 CreateExt）（`brightness_manager/include/brightness_manager.h:26-101`） |
| 类型/分类 | brightness_manager 库的门面实体（被 displaymgrservice 链接） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `DisplayPowerMgrService` 各 Inner 方法 → `BrightnessManager::Get().*`（带 screenId 默认 0）→ 路由 BrightnessService 或 Ext；Init 在服务启动时传入 defaultMax/defaultMin（`display_power_mgr_service.cpp:106`） |
| 状态流转 | Init → 常驻；多屏形态随屏连接/断开动态增删 ext 实例 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 不支持多屏时 Ext 为单实例值成员（构造即 dlopen 尝试）；RunJsonCommand/SetSceneMode 等新能力同样经门面路由 |
| 并发约束 | 多屏形态读写 `mMultiBrtMgrExt` 持 `mMutex`（shared_mutex） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `DisplayPowerMgrService`（唯一上游） |
| 下游影响 | `BrightnessService`（默认屏）、`BrightnessManagerExt`（扩展库） |
| 平级关联 | 接口面与 `BrightnessManagerExt` 完全同构（ext.h:37-72 与 manager.h 方法一一对应），便于透明转发 |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `BrightnessManager`（`brightness_manager/include/brightness_manager.h`）、`BrightnessManagerExt`（`include/brightness_manager_ext.h:23-123`） |
| 核心方法 | `Init/DeInit`、`SetBrightness/OverrideBrightness/BoostBrightness/DiscountBrightness/SetForcedBrightness`、`AutoAdjustBrightness`、`RegisterDataChangeListener/SetLightBrightnessThreshold`、`RunJsonCommand/SetSceneMode`；Ext 私有 `LoadBrightnessExtLibrary/LoadBrightnessOps*/CloseBrightnessExtLibrary` |

## 亮度服务核心（BrightnessService）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 亮度服务核心 / BrightnessService |
| 实体定义 | 默认屏亮度的唯一决策与执行核心；身份边界：单例、displayId 固定默认屏；持有亮度全链路（动作/动画/滤波/计算/设置/传感器），Ext 之外的所有亮度算法都在此 |
| 核心特征 | 单例 `Get()`；嵌套 `DimmingCallbackImpl`（动画回调 → SetSettingBrightness）与 `FoldStatusLisener`（`brightness_manager/include/brightness_service.h:54-92`）；关键常量：SETTING 键（:94,166）、`LUX_LEVEL_LENGTH=23`（:95）、`DEFAULT_BRIGHTNESS=50`、`DISCOUNT_MIN/MAX=0.01/1.00`、`WAIT_FOR_FIRST_LUX_MAX_TIME/STEP=200/10`（:168-175）；状态成员 atomic 组（overridden/boosted/target/current/sleep/first-lux，:221-244） |
| 类型/分类 | brightness_manager 核心实体；ENABLE_SENSOR_PART 下含双 SensorUser（:184-197） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | 构造：BrightnessAction(displayId) + DimmingCallbackImpl + `BrightnessDimming("BrightnessService<id>")`（cpp :75-92）；Init（call_once）：FFRT 队列 "brightness_manager"、`mDimming->Init`、InitSensors（GetAllSensors 探测 5/16 光感）、IsFoldable、ConfigParse/lux/calc InitParameters、折叠注册（:100-128）；lux 事件 → `ProcessLightLux`（门控 → IsNeedUpdateBrightness → UpdateCurrentBrightnessLevel → AMBIENT_LIGHT 分 22 档上报，:632-667）；设置亮度 → `UpdateBrightness` 统一出口（折扣→映射→直设/动画→异步设置同步，:976 起） |
| 状态流转 | override：置 `mIsBrightnessOverridden` + 记 `mBeforeOverriddenBrightness`，Restore 恢复（:874-899）；boost：拉满 + FFRT 延迟取消任务（:906-939）；折叠态变化切换 displayId/sensorId 并恢复亮度（:147-189）；睡眠 `SetSleepBrightness` 经 Override 实现限时降亮（:854-872） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 门控矩阵：CanSet=on&&!ov&&!boost、CanDiscount=on、CanOverride=on&&!boost、CanBoost=on&&!ov（:951-974）；duration 常量：亮 2000/暗 5000/快速 500/最大 3000ms（:58-61）；映射常量 `*_BRGIHTNESS_*`（拼写如此，:50-57） |
| 并发约束 | `mInitCallFlag` once；`mFirstLuxHandleLock` 保护首 lux 处理；传感器启停与状态变更并发（StateChangedSetAutoBrightness 幂等，:432-457） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `BrightnessManager` 门面 |
| 下游影响 | `BrightnessAction`（背光）、settingsdata（设置）、HiSysEvent（AMBIENT_LIGHT/BRIGHTNESS_NIT，`brightness_service.cpp:653-666,1175-1186`；BACKLIGHT_DISCOUNT 仅在 yaml 声明，本仓无写点） |
| 平级关联 | `LightLuxManager`（值成员）、`BrightnessCalculationManager`（值成员）、`BrightnessDimming`（shared_ptr）；`ScreenController` 为其并行的回退执行路径 |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `BrightnessService`（`brightness_manager/include/brightness_service.h` + `src/brightness_service.cpp`） |
| 核心方法 | `Init/DeInit`、`ProcessLightLux/UpdateCurrentBrightnessLevel`、`SetBrightness/UpdateBrightness`、`Override/Restore/Boost/CancelBoost/DiscountBrightness`、`GetMappingBrightnessLevel/GetOrigBrightnessLevel/GetMappingBrightnessNit`、`SetScreenOnBrightness`、`StateChangedSetAutoBrightness`、`FoldStatusLisener::OnFoldStatusChanged` |

## 亮度动作（BrightnessAction）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 亮度动作执行器 / BrightnessAction |
| 实体定义 | Rosen 背光访问封装；身份边界：displayId 绑定，只做背光读写与状态查询转发（AGENTS.md"BrightnessAction 封装 HDI 访问"中的 HDI 实指经 Rosen 间接到达的显示 HDI，本仓无直接 HDI 调用） |
| 核心特征 | `BrightnessAction(uint32_t displayId)`（`brightness_manager/include/brightness_action.h:30`）；`GetDisplayState` 经 `Rosen::ScreenManagerLite::GetScreenPower`；静态 `GetDefaultDisplayId/GetAllDisplayId` |
| 类型/分类 | brightness_manager Action 层实体 |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `BrightnessService::UpdateBrightness` 直设路径与 `DimmingCallbackImpl::OnChanged` 动画路径 → `mAction->SetBrightness`（映射后值）→ Rosen → 背光生效；查询路径 GetBrightness/GetDisplayState 实时读 Rosen |
| 状态流转 | 无状态 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 只接受映射域亮度（[1,223]）；调用身份重置同 ScreenAction 规则 |
| 并发约束 | 无锁；动画回调与直设可能交错，由上层 StopDimming 串行化 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `BrightnessService::mAction`、`DimmingCallbackImpl::mAction` |
| 下游影响 | Rosen DMS（背光真源） |
| 平级关联 | `ScreenAction`（状态侧同层） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `BrightnessAction`（`brightness_manager/include/brightness_action.h` + `src/brightness_action.cpp`） |
| 核心方法 | `SetBrightness/GetBrightness/GetDisplayState/SetCoordinated/GetDefaultDisplayId/GetAllDisplayId` |

## 亮度渐变动画（BrightnessDimming / GradualAnimator / AnimateCallback）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 亮度渐变动画 / BrightnessDimming（主路径）与 GradualAnimator（controller 回退路径） |
| 实体定义 | 亮度变化的 FFRT 步进动画执行器；身份边界：主路径 BrightnessDimming 服务于 BrightnessService（库内），回退路径 GradualAnimator 服务于 ScreenController 兜底；二者算法同构但独立实现 |
| 核心特征 | `BrightnessDimming(name, BrightnessDimmingCallback)`：`enable_shared_from_this`，FFRTQueue "brightness_animator_queue"（`brightness_manager/src/brightness_dimming.cpp:46`）；`GradualAnimator`：`DEFAULT_UPDATE_TIME=30`ms、`STRIDE_ABSOLUTE_MIN=1`（`state_manager/service/native/include/gradual_animator.h:44-45`）；回调接口 `OnStart/OnChanged/OnEnd/DiscountBrightness`（`AnimateCallback`/`BrightnessDimmingCallback`） |
| 类型/分类 | 两个平行动画实体（分属两子模块，避免跨模块依赖） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `StartDimming(from,to,duration)`：算 `mTotalSteps=duration/updateTime`、stride=Δ/steps（下限 1，步数按余数补偿），首步 FFRTUtils::SubmitDelayTask 入队（cpp :63-103）；`NextStep` 每步回调 OnChanged 并续排下一步，越界即收尾 OnEnd + notify（:140-174）；`StopDimming` 置停 + 取消句柄 + OnEnd（:105-118） |
| 状态流转 | idle → dimming（mDimming=true）→ idle；`WaitDimmingDone` 以 600ms 周期 wait_for 轮询（覆盖典型 500ms 动画，:125-133） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 动画运行中重复 StartDimming 直接忽略（:65-69）；重启动画必须先 StopDimming（上层 UpdateBrightness 出口保证） |
| 并发约束 | 句柄 `g_animatorTaskHandle` 全局 + `mAnimatorHandleLock`；`WaitDimmingDone` 注释明示 FFRT 下可能线程切换 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `BrightnessService::mDimming`（主）、`ScreenController::animator_`（回退，名 "BrightnessController_<id>"） |
| 下游影响 | `BrightnessAction`（每步实际背光）、settingsdata（OnEnd 持久化） |
| 平级关联 | `DimmingCallbackImpl`/`AnimateCallbackImpl` 两个回调适配器均实现折扣透传（DiscountBrightness） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `BrightnessDimming`（`brightness_manager/include/brightness_dimming.h` + `src/brightness_dimming.cpp`）、`GradualAnimator`（`state_manager/service/native/include/gradual_animator.h`） |
| 核心方法 | `StartDimming/StopDimming/IsDimming/WaitDimmingDone/GetDimmingUpdateTime/NextStep`（GradualAnimator 对应 StartAnimation/StopAnimation/IsAnimating/NextStep） |

## 环境光管理器（LightLuxManager / LightLuxBuffer）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 环境光管理器 / LightLuxManager（含 LightLuxBuffer） |
| 实体定义 | 自动亮度的数据前处理实体；身份边界：从原始 lux 到"稳定 lux"的滤波防抖计算器，不决策最终亮度（曲线在 Calculation 侧） |
| 核心特征 | `LightLuxManager : ILightLuxManager`，双缓冲 `mLuxBuffer`（原始）/`mLuxBufferFilter`（平滑值）（`brightness_manager/include/light_lux_manager.h:27`）；常量 `LIGHT_LUX_BUFFER_RANGE=10000ms`、`LIGHT_MAX_LUX=40000`、`LIGHT_LUX_BUFFER_MIN=5`、日志限流 2000ms/6 条（`src/light_lux_manager.cpp:25-30`）；场景标签 DefaultMode/GameMode/VideoMode（:31-32） |
| 类型/分类 | brightness_manager 数据处理实体（BrightnessService 值成员） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `SetLux`：GetValidLux 钳制 [0,40000] → 原始缓冲 Prune+Push → `CalcSmoothLux`（近 5 样本去最大最小 (sum-min-max)/3，不足 5 个取最新，:92-118）→ `IsUpdateLuxSuccess`：平滑缓冲 Prune+Push → 防抖判定（`GetNextBrightenTime/GetNextDarkenTime`：反向扫描平滑缓冲找 earliestValidTime + 响应时间，:120-156）→ 到期则 `UpdateParam`（更新 mFilteredLux 与亮/暗 delta 阈值，按场景模式查 luxThresholdConfig，:158-173） |
| 状态流转 | 首样本 `mIsFirstLux=true`（免防抖立即生效）；`ClearLuxData` 清双缓冲（传感器停用/折叠切换时，:248-253） |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 防抖 delta 初值 brighten 120/darken 110，随后按配置折线 CalcDelta 动态更新；场景模式经 `SetSceneMode` 切换参数组（模式数据 `GetCurrentModeData`，:277-280） |
| 并发约束 | sensor 回调线程调用 SetLux 与查询并发；缓冲自带时序（时间戳成员） |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `BrightnessService::mLightLuxManager`（唯一写方）；`ProcessLightLux` 消费其结果 |
| 下游影响 | `IsNeedUpdateBrightness` 决定是否触发亮度更新 |
| 平级关联 | `LightLuxBuffer` 环形缓冲（Push/Prune/Clear/GetData/GetTime/ToString，`brightness_manager/include/light_lux_buffer.h`）；`CalcDelta` 与 Calculation 曲线共用 `PointXy` 折线结构 |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `LightLuxManager`、`LightLuxBuffer`、`ILightLuxManager` |
| 核心方法 | `SetLux/CalcSmoothLux/IsUpdateLuxSuccess/IsNeedUpdateBrightness/UpdateParam/GetFilteredLux/GetSmoothedLux/ClearLuxData/InitParameters` |

## 亮度计算（BrightnessCalculationManager / BrightnessCalculationCurve / ConfigParse）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 亮度计算器 / BrightnessCalculationManager + BrightnessCalculationCurve |
| 实体定义 | 稳定 lux → 目标亮度的曲线插值实体；身份边界：Manager 管场景 offset 与用户修正，Curve 管基础曲线插值；配置由 ConfigParse 解析供给 |
| 核心特征 | `GetInterpolatedValue(lux)` 产 0-1 标量；`UpdateBrightnessOffset(posBrightness, lux)`（含游戏/相机模式与人体工程学 reset，`brightness_manager/include/calculation_manager.h:28`）；Curve 的 `InitParameters` 从 ConfigParse 按 displayId 0 取曲线（`include/calculation_curve.h:29`）；`ConfigParse` 单例 `GetBrightnessConfig()` 返回 `map<int, Config>`（`include/config_parser.h:28-47`，Config = calculationConfig + luxFilterConfig + luxThresholdConfig） |
| 类型/分类 | brightness_manager 计算实体（BrightnessService 值成员） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | `UpdateCurrentBrightnessLevel` → `GetBrightnessLevel(lux)` = `GetInterpolatedValue(lux) × 255`（`brightness_service.cpp:669-689`）；曲线默认回退 9 点折线（0→5、5→17、…、7000→255，`calculation_config_parser.h:33-42`）；折叠态/显示模式 → `GetDisplayIdWithFoldstatus/GetSensorIdWithDisplayMode` 从 brightnessConfig 映射（声明于 `brightness_service.h:153-158`） |
| 状态流转 | 用户切换 `UpdateCurrentUserId`；场景 offset 随模式启停增减 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | 插值下限钳 1（`CalcDelta` 同款规则）；曲线点必须单调递增（解析侧保证） |
| 并发约束 | 与 lux 回调线程并发读，offset 更新集中在状态变化路径 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `BrightnessService::mBrightnessCalculationManager` |
| 下游影响 | 目标亮度 → dimming/直设 |
| 平级关联 | `LightLuxManager`（输入）、`ConfigParse` + 解析器族（`config_parser_base.h:31-69`：LoadConfigPath 面板名/版本回退；`brightness_config_parser.h:24-41` 屏-传感器映射；`lux_filter_config_parser.h:25-33` 滤波参数；`lux_threshold_config_parser.h:28-40` 防抖参数） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `BrightnessCalculationManager`、`BrightnessCalculationCurve`、`ConfigParse`、`ConfigParserBase`、各 `*ConfigParser` |
| 核心方法 | `InitParameters/GetInterpolatedValue/UpdateBrightnessOffset/SetGameModeEnable/SetCameraModeEnable`；Curve `GetCurrentBrightness/UpdateCurveAmbientLux/GetDisplayIdWithFoldstatus`；ConfigParse `Initialize/GetBrightnessConfig/GetScreenConfig` |

## 显示公共事件管理器（DisplayCommonEventManager / SettingHelper 族）

### 实体概念
| 子维度 | 内容 |
|---|---|
| 实体名称 | 显示公共事件管理器 / DisplayCommonEventManager |
| 实体定义 | settingsdata 就绪监测与开机亮度触发实体；身份边界：只处理两个 CommonEvent（DATA_SHARE_READY/USER_SWITCHED）与 DataShare 连接探测，不参与常态亮度业务 |
| 核心特征 | `CommonEventSubscriber` 子类；`SETTING_URI_PROXY = "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true"`、`SETTINGS_DATA_EXT_URI = "datashare_ext"`（`state_manager/service/native/src/display_common_event_mgr.cpp:32-34`）；kvDataReady 状态位 |
| 类型/分类 | 服务端辅助实体（`DisplaySystemAbility` 持有） |

### 上下文与场景
| 子维度 | 内容 |
|---|---|
| 交互流程 | CES 上线 → 订阅（`display_system_ability.cpp:56-71`）→ `OnReceiveEvent`：DATA_SHARE_READY → `CheckIfSettingsDataReady`（经 `GetDpmsRemoteObj` + DataShareHelper::Create 探测，:77-119）→ `SetKvDataReady` → `HandleBootBrightness`（:45-56,131-135）；KV SA 上线的 OnAddSystemAbility 分支同样触发（`display_system_ability.cpp:72-78`） |
| 状态流转 | kvDataReady false → true（一次性）；USER_SWITCHED 在多屏形态处理用户切换 |

### 规格与约束
| 子维度 | 内容 |
|---|---|
| 业务规则 | HandleBootBrightness 仅在服务已 Init 且 kvDataReady 后执行；开机亮度使用缓存设置值（`display_power_mgr_service.cpp:149-165`） |
| 并发约束 | 事件回调线程与 SA 上线回调可能并发触发，内部有状态防重 |

### 知识关联
| 子维度 | 内容 |
|---|---|
| 上层依赖 | `DisplaySystemAbility::commonEventManager_` |
| 下游影响 | `DisplayPowerMgrService::HandleBootBrightness` |
| 平级关联 | `DisplaySettingHelper`（设置 observer，power_manager setting_provider 复用，`brightness_manager/include/brightness_setting_helper.h:29-46`）、`DisplayParamHelper`（系统参数 + boot 回调，`state_manager/service/native/include/display_param_helper.h`）、`BrightnessParamHelper`（sleep 参数） |

### 代码与符号
| 子维度 | 内容 |
|---|---|
| 核心类 | `DisplayCommonEventManager`（`state_manager/service/native/include/display_common_event_mgr.h:27` + `src/display_common_event_mgr.cpp`） |
| 核心方法 | `OnReceiveEvent/CheckIfSettingsDataReady/SetKvDataReady/HandleBootBrightness` |
