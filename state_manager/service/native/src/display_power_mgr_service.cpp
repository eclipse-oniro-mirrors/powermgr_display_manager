/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "display_power_mgr_service.h"

#include <hisysevent.h>
#include <file_ex.h>
#include <securec.h>
#include <system_ability_definition.h>
#include "errors.h"
#include "new"
#include "screen_action.h"
#ifdef ENABLE_SENSOR_PART
#include "sensor_agent.h"
#endif
#include "xcollie/watchdog.h"
#include "display_log.h"
#include "display_auto_brightness.h"
#include "display_setting_helper.h"
#include "display_param_helper.h"
#include "permission.h"
#include "setting_provider.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace DisplayPowerMgr {
using namespace OHOS::PowerMgr;
namespace {
DisplayParamHelper::BootCompletedCallback g_bootCompletedCallback;
FFRTHandle g_screenOffDelayTaskHandle;
const uint32_t GET_DISPLAY_ID_DELAY_MS = 50;
const uint32_t US_PER_MS = 1000;
const uint32_t GET_DISPLAY_ID_RETRY_COUNT = 3;
const uint32_t DEFALUT_DISPLAY_ID = 0;
}

const uint32_t DisplayPowerMgrService::BRIGHTNESS_MIN = DisplayParamHelper::GetMinBrightness();
const uint32_t DisplayPowerMgrService::BRIGHTNESS_DEFAULT = DisplayParamHelper::GetDefaultBrightness();
const uint32_t DisplayPowerMgrService::BRIGHTNESS_MAX = DisplayParamHelper::GetMaxBrightness();
std::atomic_bool DisplayPowerMgrService::isBootCompleted_ = false;
DisplayPowerMgrService::DisplayPowerMgrService() = default;

void DisplayPowerMgrService::Init()
{
    queue_ = std::make_shared<FFRTQueue> ("display_power_mgr_service");
    DISPLAY_HILOGI(COMP_SVC, "DisplayPowerMgrService Create");
    std::vector<uint32_t> displayIds;
    for (uint32_t tryCount = 0; tryCount <= GET_DISPLAY_ID_RETRY_COUNT; tryCount++) {
        displayIds = ScreenAction::GetAllDisplayId();
        if (!displayIds.empty()) {
            break;
        }
        if (tryCount < GET_DISPLAY_ID_RETRY_COUNT) {
            usleep(GET_DISPLAY_ID_DELAY_MS * US_PER_MS);
            DISPLAY_HILOGI(COMP_SVC, "cannot find any display id, retry! count: %{public}u", tryCount + 1);
        } else {
            displayIds.emplace_back(DEFALUT_DISPLAY_ID);
            DISPLAY_HILOGE(COMP_SVC, "cannot find any display id after max retry, fill with 0");
        }
    }
    for (const auto& id: displayIds) {
        DISPLAY_HILOGI(COMP_SVC, "find display, id=%{public}u", id);
        controllerMap_.emplace(id, std::make_shared<ScreenController>(id));
        BrightnessManager::Get().SetDisplayId(id);
    }

    callback_ = nullptr;
    cbDeathRecipient_ = nullptr;
#ifdef ENABLE_SENSOR_PART
    InitSensors();
#endif
    BrightnessManager::Get().Init();
    RegisterBootCompletedCallback();
}
void DisplayPowerMgrService::RegisterBootCompletedCallback()
{
    g_bootCompletedCallback = []() {
        SetBootCompletedBrightness();
        SetBootCompletedAutoBrightness();
        RegisterSettingObservers();
        isBootCompleted_ = true;
    };
    DisplayParamHelper::RegisterBootCompletedCallback(g_bootCompletedCallback);
}

void DisplayPowerMgrService::Deinit()
{
    UnregisterSettingObservers();
    BrightnessManager::Get().DeInit();
    isBootCompleted_ = false;
}

void DisplayPowerMgrService::Reset()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "reset begin");
    if (queue_) {
        queue_.reset();
        g_screenOffDelayTaskHandle = nullptr;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "destruct display_power_queue");
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "reset end");
}

void DisplayPowerMgrService::SetBootCompletedBrightness()
{
    uint32_t mainDisplayId = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetMainDisplayId();
    uint32_t brightness = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetBrightness(mainDisplayId);
    uint32_t currentDisplayId = BrightnessManager::Get().GetCurrentDisplayId(mainDisplayId);
    DisplayState state = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetDisplayState(mainDisplayId);
    BrightnessManager::Get().SetDisplayId(currentDisplayId);
    BrightnessManager::Get().SetDisplayState(currentDisplayId, state);
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->SetBrightness(brightness, mainDisplayId);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBootCompletedBrightness currentDisplayId=%{public}d", currentDisplayId);
}

void DisplayPowerMgrService::SetBootCompletedAutoBrightness()
{
    bool enable = GetSettingAutoBrightness();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBootCompletedAutoBrightness enable=%{public}d", enable);
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->AutoAdjustBrightness(enable);
    BrightnessManager::Get().AutoAdjustBrightness(enable);
}

void DisplayPowerMgrService::RegisterSettingObservers()
{
    uint32_t mainDisplayId = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetMainDisplayId();
    auto controllerMap = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->controllerMap_;
    auto iter = controllerMap.find(mainDisplayId);
    if (iter != controllerMap.end()) {
        iter->second->RegisterSettingBrightnessObserver();
    }
    RegisterSettingAutoBrightnessObserver();
}

void DisplayPowerMgrService::UnregisterSettingObservers()
{
    uint32_t mainDisplayId = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetMainDisplayId();
    auto controllerMap = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->controllerMap_;
    auto iter = controllerMap.find(mainDisplayId);
    if (iter != controllerMap.end()) {
        iter->second->UnregisterSettingBrightnessObserver();
    }
    UnregisterSettingAutoBrightnessObserver();
}

void DisplayPowerMgrService::RegisterSettingAutoBrightnessObserver()
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { AutoBrightnessSettingUpdateFunc(key); };
    DisplaySettingHelper::RegisterSettingAutoBrightnessObserver(updateFunc);
}

void DisplayPowerMgrService::AutoBrightnessSettingUpdateFunc(const std::string& key)
{
    bool isSettingEnable = GetSettingAutoBrightness(key);
    bool isSystemEnable = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->IsAutoAdjustBrightness();
    if (isSettingEnable == isSystemEnable) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "no need change autoAdjustSwitch");
        return;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AutoBrightnessSettingUpdateFunc isSettingEnable=%{public}d", isSettingEnable);
    BrightnessManager::Get().AutoAdjustBrightness(isSettingEnable);
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->AutoAdjustBrightness(isSettingEnable);
    if (!isSettingEnable) {
        DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->ClearOffset();
    }
}

void DisplayPowerMgrService::SetScreenOnBrightness()
{
    BrightnessManager::Get().SetScreenOnBrightness();
}

void DisplayPowerMgrService::ClearOffset()
{
    BrightnessManager::Get().ClearOffset();
}

void DisplayPowerMgrService::SetSettingAutoBrightness(bool enable)
{
    DisplaySettingHelper::SetSettingAutoBrightness(enable);
}

bool DisplayPowerMgrService::GetSettingAutoBrightness(const std::string& key)
{
    return DisplaySettingHelper::GetSettingAutoBrightness(key);
}
void DisplayPowerMgrService::ScreenOffDelay(uint32_t id, DisplayState state, uint32_t reason)
{
    isDisplayDelayOff_ = false;
    DISPLAY_HILOGI(COMP_SVC, "ScreenOffDelay %{public}d, %{public}d,  %{public}d", id, state, reason);
    auto iterator = controllerMap_.find(id);
    if (iterator == controllerMap_.end()) {
        return;
    }
    setDisplayStateRet_ = iterator->second->UpdateState(state, reason);
}

void DisplayPowerMgrService::UnregisterSettingAutoBrightnessObserver()
{
    DisplaySettingHelper::UnregisterSettingAutoBrightnessObserver();
}

bool DisplayPowerMgrService::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGI(COMP_SVC, "[UL_POWER] SetDisplayState %{public}d, %{public}d, %{public}u", id, state, reason);
    auto iterator = controllerMap_.find(id);
    if (iterator == controllerMap_.end()) {
        if (id != DEFALUT_DISPLAY_ID) {
            return false;
        }
        id = GetMainDisplayId();
        iterator = controllerMap_.find(id);
        if (iterator == controllerMap_.end()) {
            return false;
        }
    }

    BrightnessManager::Get().SetDisplayState(id, state);

    if (state == DisplayState::DISPLAY_OFF) {
        if (!isDisplayDelayOff_) {
            DISPLAY_HILOGI(COMP_SVC, "screen off immediately");
            return iterator->second->UpdateState(state, reason);
        }
        displayId_ = id;
        displayState_ = state;
        displayReason_ = reason;
        FFRTTask task = [this]() { ScreenOffDelay(displayId_, displayState_, displayReason_); };
        g_screenOffDelayTaskHandle = FFRTUtils::SubmitDelayTask(task, displayOffDelayMs_, queue_);
        tempState_ = iterator->second->SetDelayOffState();
        return true;
    } else if (state == DisplayState::DISPLAY_ON) {
        if (isDisplayDelayOff_) {
            DISPLAY_HILOGI(COMP_SVC, "need remove delay task");
            FFRTUtils::CancelTask(g_screenOffDelayTaskHandle, queue_);
            isDisplayDelayOff_ = false;
            tempState_ = iterator->second->SetOnState();
            return true;
        }
    }
    return iterator->second->UpdateState(state, reason);
}

DisplayState DisplayPowerMgrService::GetDisplayState(uint32_t id)
{
    DISPLAY_HILOGD(COMP_SVC, "GetDisplayState %{public}d", id);
    auto iterator = controllerMap_.find(id);
    if (iterator == controllerMap_.end()) {
        if (id != DEFALUT_DISPLAY_ID) {
            return DisplayState::DISPLAY_UNKNOWN;
        }
        id = GetMainDisplayId();
        iterator = controllerMap_.find(id);
        if (iterator == controllerMap_.end()) {
            return DisplayState::DISPLAY_UNKNOWN;
        }
    }
    return iterator->second->GetState();
}

std::vector<uint32_t> DisplayPowerMgrService::GetDisplayIds()
{
    std::vector<uint32_t> ids;
    for (auto& iter: controllerMap_) {
        ids.push_back(iter.first);
    }
    return ids;
}

uint32_t DisplayPowerMgrService::GetMainDisplayId()
{
    uint32_t id = ScreenAction::GetDefaultDisplayId();
    DISPLAY_HILOGD(COMP_SVC, "GetMainDisplayId %{public}d", id);
    return id;
}

bool DisplayPowerMgrService::SetBrightness(uint32_t value, uint32_t displayId, bool continuous)
{
    if (!Permission::IsSystem()) {
        lastError_ = DisplayErrors::ERR_SYSTEM_API_DENIED;
        return false;
    }

    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBrightness displayId=%{public}u, value=%{public}u, continuous=%{public}d",
        displayId, brightness, continuous);
    return BrightnessManager::Get().SetBrightness(brightness, 0, continuous);
}

bool DisplayPowerMgrService::DiscountBrightness(double discount, uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    bool ret = BrightnessManager::Get().DiscountBrightness(discount);
    if (ret) {
        return true;
    }
    auto brightness = iter->second->GetBrightness();
    auto safeDiscount = GetSafeDiscount(discount, brightness);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DiscountBrightness displayId=%{public}u, discount-%{public}lf",
                   displayId, safeDiscount);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "BACKLIGHT_DISCOUNT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "RATIO", safeDiscount);
    return iter->second->DiscountBrightness(safeDiscount);
}

bool DisplayPowerMgrService::OverrideBrightness(uint32_t value, uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(COMP_SVC, "OverrideBrightness displayId=%{public}u, value=%{public}u", displayId, brightness);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return BrightnessManager::Get().OverrideBrightness(brightness);
}

bool DisplayPowerMgrService::OverrideDisplayOffDelay(uint32_t delayMs)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    if (GetDisplayState(GetMainDisplayId()) != DisplayState::DISPLAY_ON || delayMs == DELAY_TIME_UNSET) {
        isDisplayDelayOff_ = false;
        return isDisplayDelayOff_;
    }
    DISPLAY_HILOGI(COMP_SVC, "OverrideDisplayOffDelay delayMs=%{public}u", delayMs);
    isDisplayDelayOff_ = true;
    displayOffDelayMs_ = delayMs;

    return isDisplayDelayOff_;
}

bool DisplayPowerMgrService::RestoreBrightness(uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGI(COMP_SVC, "RestoreBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    bool ret = BrightnessManager::Get().RestoreBrightness();
    if (ret) {
        return true;
    }
    return iter->second->RestoreBrightness();
}

uint32_t DisplayPowerMgrService::GetBrightness(uint32_t displayId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "GetBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return BRIGHTNESS_OFF;
    }
    return BrightnessManager::Get().GetBrightness();
}

uint32_t DisplayPowerMgrService::GetDefaultBrightness()
{
    return BRIGHTNESS_DEFAULT;
}

uint32_t DisplayPowerMgrService::GetMaxBrightness()
{
    return BRIGHTNESS_MAX;
}

uint32_t DisplayPowerMgrService::GetMinBrightness()
{
    return BRIGHTNESS_MIN;
}

bool DisplayPowerMgrService::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AdjustBrightness %{public}d, %{public}d, %{public}d",
                   id, value, duration);
    auto iterator = controllerMap_.find(id);
    if (iterator == controllerMap_.end()) {
        return false;
    }
    bool ret = BrightnessManager::Get().SetBrightness(value, duration);
    if (ret) {
        return true;
    }
    return iterator->second->SetBrightness(value, duration);
}

bool DisplayPowerMgrService::AutoAdjustBrightness(bool enable)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness start");
    if (!supportLightSensor_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness not support");
        SetSettingAutoBrightness(false);
        return false;
    }
    if (enable) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness enable");
        if (autoBrightness_) {
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness is already enabled");
            return true;
        }
        autoBrightness_ = true;
        BrightnessManager::Get().AutoAdjustBrightness(true);
    } else {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness disable");
        if (!autoBrightness_) {
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness is already disabled");
            return true;
        }
        autoBrightness_ = false;
        BrightnessManager::Get().AutoAdjustBrightness(false);
    }
    SetSettingAutoBrightness(enable);
    return true;
}

#ifdef ENABLE_SENSOR_PART
void DisplayPowerMgrService::ActivateAmbientSensor()
{
    if (!autoBrightness_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "auto brightness is not enabled");
        return;
    }
    if (ambientSensorEnabled_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Ambient Sensor is already on");
        return;
    }
    (void) strcpy_s(sensorUser_.name, sizeof(sensorUser_.name), "DisplayPowerMgrService");
    sensorUser_.userData = nullptr;
    sensorUser_.callback = &AmbientLightCallback;
    SubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &sensorUser_);
    SetBatch(SENSOR_TYPE_ID_AMBIENT_LIGHT, &sensorUser_, SAMPLING_RATE, SAMPLING_RATE);
    ActivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &sensorUser_);
    SetMode(SENSOR_TYPE_ID_AMBIENT_LIGHT, &sensorUser_, SENSOR_ON_CHANGE);
    ambientSensorEnabled_ = true;
}

void DisplayPowerMgrService::DeactivateAmbientSensor()
{
    if (!autoBrightness_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "auto brightness is not enabled");
        return;
    }
    if (!ambientSensorEnabled_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Ambient Sensor is already off");
        return;
    }
    DeactivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &sensorUser_);
    UnsubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &sensorUser_);
    ambientSensorEnabled_ = false;
}
#endif

bool DisplayPowerMgrService::IsAutoAdjustBrightness()
{
    return autoBrightness_;
}

bool DisplayPowerMgrService::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(COMP_SVC, "RegisterCallback");
    if (callback_ != nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "Callback function exist");
        return false;
    }
    callback_ = callback;
    sptr<IRemoteObject> remote = callback_->AsObject();
    if (!remote->IsProxyObject()) {
        DISPLAY_HILOGE(COMP_FWK, "Callback is not proxy");
        return false;
    }
    if (cbDeathRecipient_ == nullptr) {
        cbDeathRecipient_ = new CallbackDeathRecipient();
    }
    remote->AddDeathRecipient(cbDeathRecipient_);
    return true;
}

bool DisplayPowerMgrService::BoostBrightness(int32_t timeoutMs, uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Timing boost brightness: %{public}d, id: %{public}d", timeoutMs, displayId);
    RETURN_IF_WITH_RET(timeoutMs <= 0, false);
    auto iter = controllerMap_.find(displayId);
    RETURN_IF_WITH_RET(iter == controllerMap_.end(), false);
    return BrightnessManager::Get().BoostBrightness(timeoutMs);
}

bool DisplayPowerMgrService::CancelBoostBrightness(uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Cancel boost brightness, id: %{public}d", displayId);
    auto iter = controllerMap_.find(displayId);
    RETURN_IF_WITH_RET(iter == controllerMap_.end(), false);
    bool ret = BrightnessManager::Get().CancelBoostBrightness();
    if (ret) {
        return true;
    }
    return iter->second->CancelBoostBrightness();
}

uint32_t DisplayPowerMgrService::GetDeviceBrightness(uint32_t displayId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "GetDeviceBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return BRIGHTNESS_OFF;
    }
    return BrightnessManager::Get().GetDeviceBrightness();
}

bool DisplayPowerMgrService::SetCoordinated(bool coordinated, uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGD(FEAT_STATE, "Set coordinated=%{public}d, displayId=%{public}u", coordinated, displayId);
    auto iter = controllerMap_.find(displayId);
    RETURN_IF_WITH_RET(iter == controllerMap_.end(), false);
    iter->second->SetCoordinated(coordinated);
    return true;
}

uint32_t DisplayPowerMgrService::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    if (!Permission::IsSystem()) {
        return static_cast<uint32_t>(ERR_PERMISSION_DENIED);
    }
    return BrightnessManager::Get().SetLightBrightnessThreshold(threshold, callback);
}

void DisplayPowerMgrService::NotifyStateChangeCallback(uint32_t displayId, DisplayState state, uint32_t reason)
{
    std::lock_guard lock(mutex_);
    if (callback_ != nullptr) {
        callback_->OnDisplayStateChanged(displayId, state, reason);
    }
}

void DisplayPowerMgrService::DumpDisplayInfo(std::string& result)
{
    for (auto& iter: controllerMap_) {
        auto control = iter.second;
        result.append("Display Id=").append(std::to_string(iter.first));
        result.append(" State=").append(std::to_string(static_cast<uint32_t>(BrightnessManager::Get().GetState())));
        result.append(" Discount=").append(std::to_string(BrightnessManager::Get().GetDiscount()));
        result.append(" Brightness=").append(std::to_string(BrightnessManager::Get().GetBrightness()));
        if (BrightnessManager::Get().IsBrightnessOverridden()) {
            result.append(" OverrideBrightness=")
                .append(std::to_string(BrightnessManager::Get().GetScreenOnBrightness()));
        }
        if (BrightnessManager::Get().IsBrightnessBoosted()) {
            result.append(" BoostBrightness=")
                .append(std::to_string(BrightnessManager::Get().GetScreenOnBrightness()));
        }
        result.append("\n");
        result.append("DeviceBrightness=");
        result.append(std::to_string(BrightnessManager::Get().GetDeviceBrightness())).append("\n");
    }
}

int32_t DisplayPowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    if (!isBootCompleted_) {
        return ERR_NO_INIT;
    }
    if (!Permission::IsSystem()) {
        return ERR_PERMISSION_DENIED;
    }
    std::string result("DISPLAY POWER MANAGER DUMP:\n");
    DumpDisplayInfo(result);
#ifdef ENABLE_SENSOR_PART
    result.append("Support Ambient Light: ");
    if (supportLightSensor_) {
        result.append("TRUE");
    } else {
        result.append("FALSE");
    }
    result.append("\n");

    result.append("Auto Adjust Brightness: ");
    if (autoBrightness_) {
        result.append("ON");
    } else {
        result.append("OFF");
    }
    result.append("\n");
#endif

    result.append("Brightness Limits: ").append("Max=" + std::to_string(GetMaxBrightness()) + " ");
    result.append("Min=" + std::to_string(GetMinBrightness()) + " ");
    result.append("Default=" + std::to_string(GetDefaultBrightness())).append("\n");

    if (!SaveStringToFd(fd, result)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to save dump info to fd");
    }
    return ERR_OK;
}

#ifdef ENABLE_SENSOR_PART
void DisplayPowerMgrService::InitSensors()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "InitSensors start");
    SensorInfo* sensorInfo = nullptr;
    int32_t count;
    int ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Can't get sensors");
        return;
    }
    supportLightSensor_ = false;
    for (int i = 0; i < count; i++) {
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AMBIENT_LIGHT Support");
            supportLightSensor_ = true;
            break;
        }
    }

    if (!supportLightSensor_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AMBIENT_LIGHT not support");
    }
}

void DisplayPowerMgrService::AmbientLightCallback(SensorEvent* event)
{
    if (event->sensorTypeId != SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Sensor Callback is not AMBIENT_LIGHT");
        return;
    }
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Sensor Callback no service");
        return;
    }
    uint32_t mainDispId = pms->GetMainDisplayId();
    auto mainDisp = pms->controllerMap_.find(mainDispId);
    if (mainDisp == pms->controllerMap_.end()) {
        return;
    }
    if (mainDisp->second->IsBrightnessOverridden() || mainDisp->second->IsBrightnessBoosted()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Overwrite or boost brightness scene, auto brightness is invalid");
        return;
    }
    AmbientLightData* data = (AmbientLightData*) event->data;
    int32_t brightness = static_cast<int32_t>(mainDisp->second->GetCachedSettingBrightness());
    uint32_t animationUpdateTime = mainDisp->second->GetAnimationUpdateTime();
    int32_t changeBrightness = 0;
    if (pms->CalculateBrightness(data->intensity, brightness, changeBrightness)) {
        double discountStride = static_cast<double>(AUTO_ADJUST_BRIGHTNESS_STRIDE / mainDisp->second->GetDiscount());
        uint32_t gradualDuration = floor(changeBrightness / discountStride) * animationUpdateTime;
        pms->AdjustBrightness(mainDispId, brightness, gradualDuration);
    }
}

bool DisplayPowerMgrService::IsChangedLux(float scalar)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "changed: %{public}d, %{public}f vs %{public}f",
                   luxChanged_, lastLux_, scalar);
    if (lastLuxTime_ <= 0) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "receive lux at first time");
        lastLuxTime_ = time(0);
        lastLux_ = scalar;
        luxChanged_ = true;
        return false;
    }

    if (!luxChanged_) {
        float luxChangeMin = (lastLux_ / LUX_CHANGE_RATE_THRESHOLD) + 1;
        if (abs(scalar - lastLux_) < luxChangeMin) {
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Too little change");
            return false;
        } else {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "First time to change, wait for stable");
            luxChanged_ = true;
            return false;
        }
    } else {
        float luxChangeMin = (lastLux_ / LUX_CHANGE_RATE_THRESHOLD) + 1;
        if (luxChangeMin < LUX_CHANGE_STABLE_MIN) {
            luxChangeMin = LUX_CHANGE_STABLE_MIN;
        }
        if (abs(scalar - lastLux_) >= luxChangeMin) {
            time_t currentTime = time(0);
            time_t sub = currentTime - lastLuxTime_;
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "stable lux");
            if (sub >= LUX_STABLE_TIME) {
                DISPLAY_HILOGI(FEAT_BRIGHTNESS, "stable enought to change");
                lastLuxTime_ = time(0);
                lastLux_ = scalar;
                luxChanged_ = false;
                return true;
            }
        } else {
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "unstable lux, wait for stable");
            luxChanged_ = true;
            return false;
        }
    }
    return false;
}
#endif

uint32_t DisplayPowerMgrService::GetSafeBrightness(uint32_t value)
{
    auto brightnessValue = value;
    if (brightnessValue > BRIGHTNESS_MAX) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "brightness value is greater than max, value=%{public}u", value);
        brightnessValue = BRIGHTNESS_MAX;
    }
    if (brightnessValue < BRIGHTNESS_MIN) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "brightness value is less than min, value=%{public}u", value);
        brightnessValue = BRIGHTNESS_MIN;
    }
    return brightnessValue;
}

double DisplayPowerMgrService::GetSafeDiscount(double discount, uint32_t brightness)
{
    auto safeDiscount = discount;
    if (safeDiscount > DISCOUNT_MAX) {
        DISPLAY_HILOGD(COMP_SVC, "discount value is greater than max, discount=%{public}lf", discount);
        safeDiscount = DISCOUNT_MAX;
    }
    if (safeDiscount < DISCOUNT_MIN) {
        DISPLAY_HILOGD(COMP_SVC, "discount value is less than min, discount=%{public}lf", discount);
        safeDiscount = DISCOUNT_MIN;
    }
    if (static_cast<uint32_t>(BRIGHTNESS_MIN / safeDiscount) > BRIGHTNESS_MAX) {
        DISPLAY_HILOGD(COMP_SVC, "brightness than max, brightness=%{public}u, discount=%{public}lf",
                       static_cast<uint32_t>(BRIGHTNESS_MIN / safeDiscount), discount);
        safeDiscount = static_cast<double>(BRIGHTNESS_MIN / static_cast<double>(brightness));
    }

    return safeDiscount;
}

bool DisplayPowerMgrService::CalculateBrightness(float scalar, int32_t& brightness, int32_t& change)
{
    const float lastLux = lastLux_;
#ifdef ENABLE_SENSOR_PART
    if (!IsChangedLux(scalar)) {
        return false;
    }
#endif
    int32_t calcBrightness = GetBrightnessFromLightScalar(scalar);
    int32_t difference = abs(calcBrightness - brightness);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "lux: %{public}f -> %{public}f, screen: %{public}d -> %{public}d",
                   lastLux, scalar, brightness, calcBrightness);
    if (difference < BRIGHTNESS_CHANGE_MIN
        || (scalar > lastLux && calcBrightness < brightness)
        || (scalar < lastLux && calcBrightness > brightness)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "screen is too light/dark when calculated change");
        return false;
    }
    brightness = calcBrightness;
    change = difference;
    return true;
}

int32_t DisplayPowerMgrService::GetBrightnessFromLightScalar(float scalar)
{
    uint32_t brightness = DisplayAutoBrightness::CalculateAutoBrightness(scalar);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "scalar: %{public}f, brightness: %{public}d", scalar, brightness);
    if (brightness > BRIGHTNESS_MAX) {
        brightness = BRIGHTNESS_MAX;
    } else if (brightness < BRIGHTNESS_MIN) {
        brightness = BRIGHTNESS_MIN;
    }
    return static_cast<int32_t>(brightness);
}

DisplayErrors DisplayPowerMgrService::GetError()
{
    DisplayErrors tmpError = lastError_;
    lastError_ = DisplayErrors::ERR_OK;
    return tmpError;
}

void DisplayPowerMgrService::CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DISPLAY_HILOGI(COMP_SVC, "CallbackDeathRecipient OnRemoteDied");
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "OnRemoteDied no service");
        return;
    }

    std::lock_guard lock(callbackMutex_);
    pms->callback_ = nullptr;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
