# 编码铁律

## IPC 接口变更
- **禁止**直接手写主接口 stub/proxy，主接口一律经 `state_manager/service/IDisplayPowerMgr.idl` 由 `idl_gen_interface("displaymgr_interface")` 生成（`state_manager/service/BUILD.gn:125-200`：proxy/stub source_set 从生成物 filter）。新增/修改方法只改 `.idl` 与 `DisplayPowerMgrIdlTypes.idl` 类型，服务端实现改 `DisplayPowerMgrStub` 子类 `DisplayPowerMgrService`。**例外**：4 组回调方向（`IDisplayPowerCallback`/`IDisplayBrightnessCallback`/`IDisplayBrightnessListener`/`IMultiScreenDisplayStateCallback`）是 zidl 手写 proxy/stub（`state_manager/service/zidl/`），改签名必须同步 `_proxy.cpp`/`_stub.cpp` 两侧与 `interfaces/inner_api/native/include/*_ipc_interface_code.h` 码值（只追加，不复用），否则旧客户端调错接口（AGENTS.md 陷阱）。
- **禁止**修改 `[ipccode N]` 已有码值或在中间插入新方法；`[ipccode 0] SetDisplayState`、`[ipccode 21] SetLightBrightnessThreshold` 是稳定契约（`IDisplayPowerMgr.idl:24,48`）。查询类方法保持 `[customMsgOption flags=MessageOption::TF_IMAGE]` 标注（IDL :28,37-39）以维持高优先级。

## 权限与错误处理
- **禁止**在客户端做权限判断后省略服务端校验；所有写类 IPC 入口必须 `Permission::IsSystem()`（power_manager 提供）并按失败路径返回：bool 接口直接 false，带 retCode 的接口置 `ERR_PERMISSION_DENIED(201)`，系统 API 拒绝置 `lastError_ = ERR_SYSTEM_API_DENIED(202)`（范例：`display_power_mgr_service.cpp:358-369` SetBrightnessInner、:587-594 SetLightBrightnessThresholdInner）。参数校验用 `CHECK_PARAM_WITH_RET`/`RETURN_IF_WITH_RET`/`IS_OUT_RANGE`（`state_manager/utils/native/include/display_common.h:27,37`），错误码取 `DisplayErrors` 枚举（`display_mgr_errors.h:46-54`），禁止自造数字。
- **禁止**吞掉失败返回 true；亮灭屏失败必须走 Undo 回退（`SetDisplayStateInner` → `UndoSetDisplayStateInner`，`display_power_mgr_service.cpp:265-324`），回调注册失败要写 ABNORMAL_FAULT HiSysEvent（`powermanager.yaml`）。

## 并发模型
- **禁止**在亮度/状态路径直接起线程或同步 sleep；延迟任务用 `FFRTUtils::SubmitDelayTask` 提交到命名 FFRT 队列（"display_power_mgr_service"/"brightness_manager"/"brightness_animator_queue"/"screen_controller"，见 `display_power_mgr_service.cpp:89`、`brightness_service.cpp:103`、`brightness_dimming.cpp:46`），并在 DeInit/Reset 时 `FFRT_CANCEL` 句柄（`brightness_dimming.cpp:105-118`）。服务重置后残留延迟任务会访问已析构对象。
- **禁止**无锁读写共享状态：`ScreenController::state_` 访问必须持 `mutexState_` 或走 atomic（`screen_controller.h:99-100`）；亮度标志（overridden/boosted/target）用 `std::atomic`（`brightness_service.h:221-242`）；`AutoAdjustBrightnessInner` 的开关+设置写入必须持 `autoBrightnessMutex_`（`display_power_mgr_service.cpp:493-495`）。调用 Rosen 前后必须 `IPCSkeleton::ResetCallingIdentity/SetCallingIdentity` 配对（`screen_action.cpp:38-40,149,176`）。

## 亮度路径规则
- **禁止**绕过 `BrightnessService::UpdateBrightness` 出口直接调 `BrightnessAction::SetBrightness`；出口统一完成 折扣乘算 → 255→223 映射（`GetMappingBrightnessLevel`，`brightness_service.cpp:1073`）→ 直设或 dimming → 异步设置同步。注意常量名 `MAX_DEFAULT_BRGIHTNESS_LEVEL`（代码拼写如此，`brightness_service.cpp:50-61`）不要"顺手修正"。
- **禁止**硬编码 lux 滤波/防抖阈值或亮度曲线；从 `ConfigParse`（`/sys_prod/etc/display/` JSON，按 displayId 取 `Config{calculationConfig, luxFilterConfig, luxThresholdConfig}`，`brightness_manager/include/config_parser.h:28-47`）读取；解析失败用解析器内默认曲线（`calculation_config_parser.h:33-42`）。duration 语义：亮 2000ms/暗 5000ms/首 lux 快速 500ms/最大亮度 3000ms（`brightness_service.cpp:58-61`）。
- **禁止**在 override/boost 生效期间接受普通 SetBrightness（`CanSetBrightness = IsScreenOn && !overridden && !boosted`，`brightness_service.cpp:951-959`）；先 `ForceExitOverriddenMode` 或恢复后再设。修改 `ScreenController::SetBrightness`（`screen_controller.cpp:174-183`）前注意其 `if (CanSetBrightness())` 与日志"Cannot set brightness"极性疑似与 `BrightnessService` 的 `!CanSetBrightness()` 相反（该方法仅作 `AdjustBrightnessInner` 回退路径调用，`display_power_mgr_service.cpp:471-475`），改动需先确认意图。

## Rosen 访问与状态映射
- **禁止**在 Action 层之外调用 `Rosen::DisplayManagerLite`/`ScreenManagerLite` 写接口；状态/电源枚举映射只在 `ScreenAction::ParseDisplayState`（display 状态）/`ParseScreenPowerState`（screen 电源）/`ParseSpecialReason`（reason，PRE_BRIGHT 族与协同/投屏原因）内维护（`screen_action.cpp:94-115,189-246`），新增 DisplayState 时必须同步三处映射，漏一处会静默变为 UNKNOWN。
- **禁止**跳过 reason 语义处理：`STATE_CHANGE_REASON_PRE_BRIGHT`/`_AUTH_FAIL_SCREEN_OFF` 触发 `IsNeedSkipNextProc`（跳过后续流程，`screen_controller.cpp:283-291`）；`STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS` 触发 `EnableSkipSetDisplayState`（跳过 Rosen 直达 OnStateChanged，`screen_action.cpp:86-92`）；协同屏 + TIMEOUT reason 走 `SetSpecifiedScreenPower`（`screen_action.cpp:256-258`）。

## 日志与 DFX
- **禁止**新增未在 yaml 声明的 HiSysEvent；本仓事件集中在 `displaymanager.yaml`（SCREEN_STATE/BRIGHTNESS_NIT/BACKLIGHT_DISCOUNT/AMBIENT_LIGHT/DMS_WAIT_LOCKSCREEN_TIMEOUT）与 `powermanager.yaml`（ABNORMAL_FAULT），tag PowerStats 供电池统计消费（battery_statistics 订阅 SCREEN_STATE/BRIGHTNESS_NIT/BACKLIGHT_DISCOUNT/AMBIENT_LIGHT）。日志用 `DISPLAY_HILOG*` 宏 + `COMP_SVC/COMP_FWK/FEAT_BRIGHTNESS/FEAT_STATE` 域（idl 生成配置 log_domainid 0xD002982、log_tag DisplayPowerSvc，`service/BUILD.gn:137-138`），禁止 printf/裸 hilog。
- **禁止**删除亮度/状态变更的关键观测点：SCREEN_STATE 在每次 `SetDisplayState`（`screen_action.cpp:178-181`）、AMBIENT_LIGHT 在 lux 分档变化（`brightness_service.cpp:653-666`）、DMS_WAIT_LOCKSCREEN_TIMEOUT 在锁屏等待超 300ms（亮）/2000ms（灭）（`screen_action.cpp:120-135`）；这些是耗电统计与性能归因的数据源。
