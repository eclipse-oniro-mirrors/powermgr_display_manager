# 术语表

## 屏幕显示状态（DisplayState）
本服务对屏幕逻辑状态的统一建模，枚举 `DISPLAY_OFF=0 / DIM=1 / ON=2 / SUSPEND=3 / DELAY_OFF=4 / DOZE / DOZE_SUSPEND / UNKNOWN`（`state_manager/interfaces/inner_api/native/include/display_power_info.h:26-35`）。与 Rosen 两个体系映射：display 状态（`ScreenAction::ParseDisplayState`，ON/OFF/DOZE/DOZE_SUSPEND 四值，`screen_action.cpp:94-115`）与 screen 电源（`ParseScreenPowerState`，含 DIM=POWER_STAND_BY、SUSPEND，`screen_action.cpp:220-246`）。DOZE 族是 AOD 低功耗态；SUSPEND/DOZE 在回调通知时归一化为 OFF（`screen_controller.cpp:317-319`）。与 power_manager 的 `PowerState`（电源状态机）是不同概念：本枚举只描述屏幕。

## DIM 屏（Dim / 半亮待机）
亮屏但降低背光的待机态（Rosen `POWER_STAND_BY`）。状态机规则：OFF 态请求 DIM 被拒绝；DIM 态请求 ON 只更新本地状态不再走 Rosen（`screen_controller.cpp:108-143` 同态幂等与 case 分支）。与"灭屏前的亮度变暗"（dimming 动画）无关。
## 延迟灭屏（DELAY_OFF）
`SetDisplayState(DISPLAY_DELAY_OFF)` 触发：先置 DIM，再在 FFRT 队列上延迟任务到期后真正灭屏（`display_power_mgr_service.cpp:254-265` ScreenOffDelay → SubmitDelayTask），灭屏失败自动 Undo 恢复原状态。与 `OverrideDisplayOffDelay`（覆盖延迟时长）配套。

## 灭屏策略（PowerOffStrategy / MiscellaneousDisplayPowerStrategy）
特性开关 `display_manager_feature_poweroff_strategy`（`displaymgr.gni`，define `ENABLE_SCREEN_POWER_OFF_STRATEGY`）启用后，`SetScreenPowerOffStrategy(ALL/SPECIFIC, reason)` 经 `MiscellaneousDisplayPowerStrategy` 单例换算特定 reason 再下屏（`miscellaneous_display_power_strategy.h:26-37`、`screen_action.cpp:260-263`），用于按 token/场景区分灭屏路径。

## 亮度覆盖/增强/折扣（Override / Boost / Discount）
三种高优先亮度干预，互斥规则见 `brightness_service.cpp:951-974`：Override（覆盖保持，直到 Restore 恢复 `mBeforeOverriddenBrightness`）、Boost（限时拉到最大亮度，FFRT 延迟任务自动 CancelBoost，`brightness_service.cpp:906-927`）、Discount（0.01–1.00 折扣系数乘算，只影响实际输出不改设置值，`brightness_service.cpp:830-852`）。三者生效期间普通 SetBrightness 被门控拒绝。

## 亮度映射（Mapping Brightness Level）
用户/设置亮度域 [1,255] 到硬件背光域 [1,223] 的线性映射（`GetMappingBrightnessLevel`/`GetOrigBrightnessLevel` 互逆，`brightness_service.cpp:1073-1102`），另有 nit 映射（默认 2–600 nit）与高亮层（HBM 域 156–10000）。`GetDeviceBrightness(useHbm)` 决定返回原始值还是逆映射值（`brightness_service.cpp:810-816`）。这是"设置 255 但实际 223"差异的来源。

## 自动亮度（Auto Brightness）
环境光驱动的亮度调节：sensor lux → `LightLuxManager` 滤波防抖 → `BrightnessCalculationCurve` 曲线插值 → `UpdateCurrentBrightnessLevel`（亮 2000ms/暗 5000ms 过渡）。开关持久化在 `settings.display.auto_screen_brightness`；屏幕状态变化时经 `StateChangedSetAutoBrightness` 随亮灭启停传感器（`brightness_service.cpp:432-457`）。首 lux 到达前亮屏用缓存设置亮度并最多等待 200ms（`WAIT_FOR_FIRST_LUX_MAX_TIME`，`brightness_service.h:174`）。

## 环境光 lux 与滤波（Lux / SmoothedLux / FilteredLux）
原始 lux（sensor 上报，上限 40000 钳制）→ 平滑值 `mSmoothedButNotStabledLux`（近 5 个样本去最大最小取均值 (sum-min-max)/3，`light_lux_manager.cpp:92-118`）→ 防抖后的稳定值 `mFilteredLux`（时间窗 10000ms，`light_lux_manager.cpp:25`）。GetFilteredLux 返回平滑值、GetSmoothedLux 返回稳定值（取整），两者命名与直觉相反，阅读时注意（`light_lux_manager.cpp:232-240`）。

## 折叠态与双光感（FoldStatus / AMBIENT_LIGHT1）
折叠屏主屏/副屏各有一个环境光传感器：`SENSOR_TYPE_ID_AMBIENT_LIGHT(5)` 与 `SENSOR_TYPE_ID_AMBIENT_LIGHT1(16)`（`brightness_config_parser.h:24-27`）。`BrightnessConfig::Data` 的 `displayModeMap`/`foldStatusModeMap` 配置 display 模式/折叠态到 {displayId, sensorId} 的映射（`brightness_config_parser.h:29-41`）；`FoldStatusLisener::OnFoldStatusChanged` 据此切换激活的传感器并恢复亮度（`brightness_service.cpp:147-189`）。

## Rosen（window_manager DMS，SA 4607）
图形侧显示管理服务（`DISPLAY_MANAGER_SERVICE_SA_ID = 4607`，`system_ability_definition.h`）。本服务所有屏幕状态/背光写操作的唯一通道：`DisplayManagerLite`（display 状态、背光、折叠态）与 `ScreenManagerLite`（screen 电源、协同屏）。SA 3308 启动即监听 SA 4607 上线后才初始化业务（`display_system_ability.cpp:31,39-55`）。本仓不直接调用 HDI，HDI 消费在 render service 侧。

## 协同屏（Coordinated Screen）
`SetCoordinated(true)` 后，TIMEOUT 原因的灭屏走 `SetSpecifiedScreenPower`（COLLABORATION reason）而非 `SetScreenPowerForAll`（`screen_action.cpp:256-263`），用于多设备协同场景只操作指定屏。

## 多屏状态（Multi Screen State）
特性开关 `display_manager_feature_multi_screen_state`（define `DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE`）启用的一组 API：按逻辑 displayId + screenName 置亮灭（仅 ON/OFF）、查询全部（`GetAllMultiScreenDisplayState`）、按屏订阅回调；`DISPLAY_ID_ALL = UINT64_MAX` 表示全部屏（`display_power_mgr_client.h:89-133`）。IVI 场景使用。

## 场景模式（SceneModeType）
`SCENE_MODE_BUSINESS`（商务常亮）与 `SCENE_MODE_CONSTANT`（常量低暗曲线）（`DisplayPowerMgrIdlTypes.idl:50-55`）；另有内部 `BrightnessSceneMode`（默认/游戏/视频）控制 lux 防抖参数组选择（`brightness_base.h` BrightnessSceneMode，`light_lux_manager.cpp:31-32` ScenceLabel）。游戏/视频模式还带亮度 offset 修正（`calculation_manager.h` UpdateBrightnessOffset）。

## 亮度数据变化监听（DisplayDataChangeListenerType）
8 类 listenerType 的语义分流：LIGHT_OR_BRIGHTNESS_FOR_APS（面向 APS）、STABLE_LUX、FORCE_EXIT_OVERRIDDEN_MODE、BRIGHTNESS_FOR_UI（含 dimming 全过程）、BRIGHTNESS_TARGET（仅最终目标）、FACE_DETECTION_STATE、BRIGHTNESS_SLIDER、FORCE_ENTER_CONSTANT_MODE（`DisplayPowerMgrIdlTypes.idl:18-30`）。配套 `BrightnessFeatureType`（CARE_MODE/SCREEN_FLASH/BRIGHTNESS_INFO 查询，:34-40）与 `BrightnessValueType`（相对当前区间/相对最大/绝对 nit 三种取值，:42-48）。

## 开机亮度（Boot Brightness）
依赖就绪链：SA 3308 上线（run-on-create: true）→ 监听 SA 4607/CES/KV 三者上线 → 全就绪后 `HandleBootBrightness` 按设置缓存值恢复亮度（`display_system_ability.cpp:37-78`、`display_common_event_mgr.cpp:131-135`）；boot 完成经 `DisplayParamHelper::RegisterBootCompletedCallback` 置 `isBootCompleted_`（`display_power_mgr_service.cpp:122-128`）。与 battery_statistics 的懒启动（3304 run-on-create: false）相反，本 SA 是常驻启动。

## powermgr 共进程
SA 3301（power）/3302（battery）/3303（thermal）/3308（display）共享 `powermgr` 进程（`sa_profile/3308.json`）。任何一个 SA 崩溃或阻塞会拖垮其余服务，因此亮度/状态路径禁止长耗时同步操作（AGENTS.md 陷阱）。

## zidl 与 ipccode
zidl 目录（`state_manager/service/zidl/`）存放**手写**的回调方向 proxy/stub（服务端 → 客户端的事件推送），与 `.idl` 生成的主接口相对。`[ipccode N]` 是 IDL 中显式声明的 IPC 码（仅 SetDisplayState=0、SetLightBrightnessThreshold=21 显式标注，其余按顺序生成，`IDisplayPowerMgr.idl:24,48`）；回调码在 `*_ipc_interface_code.h` 手工维护（如 `ON_DISPLAY_STATE_CHANGED = 0`，`display_power_callback_ipc_interface_code.h:22-24`），均为只追加的 ABI 契约。

## TF_IMAGE 消息
`[customMsgOption flags=MessageOption::TF_IMAGE]`：IDL 中为高频查询（GetMainDisplayId/GetDefaultBrightness/GetMaxBrightness/GetMinBrightness）声明的消息标志，走高优先级投递路径（`IDisplayPowerMgr.idl:28,37-39`）。

## 亮度设置键与系统参数
设置项（settingsdata，DataShare）：`settings.display.screen_brightness_status`（亮度值）、`settings.display.auto_screen_brightness`（自动开关）。系统参数（`display.para`）：`const.display.brightness.default=102`、`.max=255`、`.min=1`（客户端 SDK 同名回退常量 `BRIGHTNESS_DEFAULT{102}/MAX{255}/MIN{1}`，`display_power_mgr_client.h:156-159`）及 sleep 系列（`BrightnessParamHelper`）。两类键不要混用：前者用户可见，后者产品配置。

## 亮度配置文件（brightness_config.json）
`/sys_prod/etc/display/` 根 + 按显示形态分目录（`brightness_config/` 默认、`full/brightness_config/`、`sub/brightness_config/`），文件名 `brightness_config.json`，支持 `brightness_config_<panelName>_<panelVersion>.json` 产品回退（`config_parser_base.cpp:31-39,84-127`）。内容含计算曲线（calculationConfig）、lux 滤波（luxFilterConfig）、防抖阈值（luxThresholdConfig，按 DefaultMode/GameMode/VideoMode 分组）、屏-传感器映射（brightnessConfig）。

## DisplayErrors（错误码）
业务错误码枚举：`ERR_OK=0`、`ERR_PERMISSION_DENIED=201`、`ERR_SYSTEM_API_DENIED=202`、`ERR_PARAM_INVALID=401`、`ERR_CONNECTION_FAIL=4700101`、`ERR_STATE_CHANGE_FAILED=4700102`、`ERR_REGISTRATION_FAILED=4700103`（`state_manager/utils/native/include/display_mgr_errors.h:46-54`）。201/202/401 对齐 ArkTS API 错误码语义；4700101+ 是 `DISPLAYFWK_SERVICE_ERR_OFFSET`（`ErrCodeOffset(SUBSYS_POWERMNG, DISPLAY_MODULE_TYPE_SERVICE)`）派生的部件内错误。客户端经 `GetError()` 读最近一次错误（原子交换复位，`display_power_mgr_client.cpp:508`）。

## FFRT（Function Flow Runtime）
OpenHarmony 协作式异步任务框架。本仓用法：命名 FFRTQueue（服务队列/动画队列/控制器队列）、`FFRTUtils::SubmitDelayTask/CancelTask`（延迟灭屏、cancel boost、dimming 步进）。`WaitDimmingDone` 的注释提示"may trigger thread switching in ffrt"——FFRT 协程切换会让传统线程假设失效（`brightness_dimming.cpp:125-133`）。
