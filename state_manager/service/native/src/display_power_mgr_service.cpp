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
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <file_ex.h>
#include <securec.h>
#include <system_ability_definition.h>
#include "errors.h"
#include "ipc_skeleton.h"
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
#include "power_state_machine_info.h"
#include "setting_provider.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace DisplayPowerMgr {
using namespace OHOS::PowerMgr;
using namespace Rosen;
namespace {
DisplayParamHelper::BootCompletedCallback g_bootCompletedCallback;
FFRTHandle g_screenOffDelayTaskHandle;
const uint32_t GET_DISPLAY_ID_DELAY_MS = 50;
const uint32_t US_PER_MS = 1000;
const uint32_t GET_DISPLAY_ID_RETRY_COUNT = 3;
const uint32_t DEFALUT_DISPLAY_ID = 0;
const uint32_t TEST_MODE = 1;
const uint32_t NORMAL_MODE = 2;
const uint32_t BOOTED_COMPLETE_DELAY_TIME = 2000;
const int32_t ERR_OK = 0;
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
#ifndef FUZZ_COV_TEST
    BrightnessManager::Get().Init(BRIGHTNESS_MAX, BRIGHTNESS_MIN);
#endif
    for (const auto& id: displayIds) {
        DISPLAY_HILOGI(COMP_SVC, "find display, id=%{public}u", id);
        controllerMap_.emplace(id, std::make_shared<ScreenController>(id));
        BrightnessManager::Get().SetDisplayId(id);
    }

    callback_ = nullptr;
    cbDeathRecipient_ = nullptr;
    RegisterBootCompletedCallback();
}

void DisplayPowerMgrService::RegisterBootCompletedCallback()
{
    g_bootCompletedCallback = []() {
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

void DisplayPowerMgrService::HandleBootBrightness()
{
    BrightnessManager::Get().Init(BRIGHTNESS_MAX, BRIGHTNESS_MIN);
    std::call_once(initFlag_, [] {
        SetBootCompletedBrightness();
        SetBootCompletedAutoBrightness();
    });
    RegisterSettingObservers();
}

void DisplayPowerMgrService::SetBootCompletedBrightness()
{
    uint32_t mainDisplayId = 0;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetMainDisplayId(mainDisplayId);
    uint32_t brightness = BRIGHTNESS_OFF;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetBrightness(mainDisplayId, brightness);
    uint32_t currentDisplayId = BrightnessManager::Get().GetCurrentDisplayId(mainDisplayId);
    int32_t state = static_cast<int32_t>(DisplayState::DISPLAY_UNKNOWN);
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetDisplayState(mainDisplayId, state);
    BrightnessManager::Get().SetDisplayId(currentDisplayId);
    BrightnessManager::Get().SetDisplayState(currentDisplayId, static_cast<DisplayState>(state), 0);
    bool result = false;
    int32_t errCode = 0;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->SetBrightness(brightness, mainDisplayId, false,
        result, errCode);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBootCompletedBrightness currentDisplayId=%{public}d", currentDisplayId);
}

void DisplayPowerMgrService::SetBootCompletedAutoBrightness()
{
    bool enable = GetSettingAutoBrightness();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBootCompletedAutoBrightness enable=%{public}d", enable);
    bool result = false;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->AutoAdjustBrightness(enable, result);
    BrightnessManager::Get().AutoAdjustBrightness(enable);
}

void DisplayPowerMgrService::RegisterSettingObservers()
{
    uint32_t mainDisplayId = 0;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetMainDisplayId(mainDisplayId);
    auto controllerMap = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->controllerMap_;
    auto iter = controllerMap.find(mainDisplayId);
    if (iter != controllerMap.end()) {
        iter->second->RegisterSettingBrightnessObserver();
    }
    RegisterSettingAutoBrightnessObserver();
}

void DisplayPowerMgrService::UnregisterSettingObservers()
{
    uint32_t mainDisplayId = 0;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->GetMainDisplayId(mainDisplayId);
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
    bool isSystemEnable = false;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->IsAutoAdjustBrightness(isSystemEnable);
    if (isSettingEnable == isSystemEnable) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "no need change autoAdjustSwitch");
        return;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AutoBrightnessSettingUpdateFunc isSettingEnable=%{public}d", isSettingEnable);
    BrightnessManager::Get().AutoAdjustBrightness(isSettingEnable);
    bool result = false;
    DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->AutoAdjustBrightness(isSettingEnable, result);
    if (!isSettingEnable) {
        DelayedSpSingleton<DisplayPowerMgrService>::GetInstance()->ClearOffset();
    }
}

bool DisplayPowerMgrService::SetScreenOnBrightnessInner()
{
    if (!Permission::IsSystem()) {
        return false;
    }
    BrightnessManager::Get().SetScreenOnBrightness();
    return true;
}

void DisplayPowerMgrService::ClearOffset()
{
    BrightnessManager::Get().ClearOffset();
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
    iterator->second->UpdateState(state, reason);
}

void DisplayPowerMgrService::UnregisterSettingAutoBrightnessObserver()
{
    DisplaySettingHelper::UnregisterSettingAutoBrightnessObserver();
}

bool DisplayPowerMgrService::SetDisplayStateInner(uint32_t id, DisplayState state, uint32_t reason)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    uint32_t ffrtId = ffrt::this_task::get_id();
    DISPLAY_HILOGI(COMP_SVC, "[UL_POWER] SetDisplayState %{public}d, %{public}d, %{public}u, ffrtId=%{public}u",
        id, state, reason, ffrtId);
    auto iterator = controllerMap_.find(id);
    if (iterator == controllerMap_.end()) {
        if (id != DEFALUT_DISPLAY_ID) {
            return false;
        }
        id = GetMainDisplayIdInner();
        iterator = controllerMap_.find(id);
        if (iterator == controllerMap_.end()) {
            return false;
        }
    }
    BrightnessManager::Get().SetDisplayState(id, state, reason);

    if (state == DisplayState::DISPLAY_OFF || state == DisplayState::DISPLAY_DOZE) {
        if (!isDisplayDelayOff_) {
            DISPLAY_HILOGI(COMP_SVC, "screen off immediately");
            bool ret = iterator->second->UpdateState(state, reason);
            if (!ret) {
                UndoSetDisplayStateInner(id, iterator->second->GetState(), reason);
            }
            return ret;
        }
        displayId_ = id;
        displayState_ = state;
        displayReason_ = reason;
        FFRTTask task = [this]() { ScreenOffDelay(displayId_, displayState_, displayReason_); };
        g_screenOffDelayTaskHandle = FFRTUtils::SubmitDelayTask(task, displayOffDelayMs_, queue_);
        iterator->second->SetDelayOffState();
        return true;
    } else if (state == DisplayState::DISPLAY_ON) {
        if (isDisplayDelayOff_) {
            DISPLAY_HILOGI(COMP_SVC, "need remove delay task");
            FFRTUtils::CancelTask(g_screenOffDelayTaskHandle, queue_);
            isDisplayDelayOff_ = false;
            iterator->second->SetOnState();
            return true;
        }
    }
    return iterator->second->UpdateState(state, reason);
}

void DisplayPowerMgrService::UndoSetDisplayStateInner(uint32_t id, DisplayState curState, uint32_t reason)
{
    DISPLAY_HILOGI(COMP_SVC, "[UL_POWER]undo brightness SetDisplayState:%{public}u", curState);
    BrightnessManager::Get().SetDisplayState(id, curState, reason);
    if (curState == DisplayState::DISPLAY_ON) {
        BrightnessManager::Get().SetScreenOnBrightness();
    }
}

DisplayState DisplayPowerMgrService::GetDisplayStateInner(uint32_t id)
{
    DISPLAY_HILOGD(COMP_SVC, "GetDisplayState %{public}d", id);
    auto iterator = controllerMap_.find(id);
    if (iterator == controllerMap_.end()) {
        if (id != DEFALUT_DISPLAY_ID) {
            return DisplayState::DISPLAY_UNKNOWN;
        }
        id = GetMainDisplayIdInner();
        iterator = controllerMap_.find(id);
        if (iterator == controllerMap_.end()) {
            return DisplayState::DISPLAY_UNKNOWN;
        }
    }
    return iterator->second->GetState();
}

std::vector<uint32_t> DisplayPowerMgrService::GetDisplayIdsInner()
{
    std::vector<uint32_t> ids;
    for (auto& iter: controllerMap_) {
        ids.push_back(iter.first);
    }
    return ids;
}

uint32_t DisplayPowerMgrService::GetMainDisplayIdInner()
{
    uint32_t id = ScreenAction::GetDefaultDisplayId();
    DISPLAY_HILOGD(COMP_SVC, "GetMainDisplayId %{public}d", id);
    return id;
}

bool DisplayPowerMgrService::SetBrightnessInner(uint32_t value, uint32_t displayId, bool continuous)
{
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        return false;
    }

    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBrightness displayId=%{public}u, value=%{public}u, continuous=%{public}d",
        displayId, brightness, continuous);
    return BrightnessManager::Get().SetBrightness(brightness, 0, continuous);
}

bool DisplayPowerMgrService::DiscountBrightnessInner(double discount, uint32_t displayId)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return BrightnessManager::Get().DiscountBrightness(discount);
}

bool DisplayPowerMgrService::OverrideBrightnessInner(uint32_t brightness, uint32_t displayId, uint32_t duration)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGI(COMP_SVC, "OverrideBrightness displayId=%{public}u, value=%{public}u, duration=%{public}d",
        displayId, brightness, duration);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return BrightnessManager::Get().OverrideBrightness(brightness, duration);
}

bool DisplayPowerMgrService::OverrideDisplayOffDelayInner(uint32_t delayMs)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    if (GetDisplayStateInner(GetMainDisplayIdInner()) != DisplayState::DISPLAY_ON || delayMs == DELAY_TIME_UNSET) {
        isDisplayDelayOff_ = false;
        return isDisplayDelayOff_;
    }
    DISPLAY_HILOGI(COMP_SVC, "OverrideDisplayOffDelay delayMs=%{public}u", delayMs);
    isDisplayDelayOff_ = true;
    displayOffDelayMs_ = delayMs;

    return isDisplayDelayOff_;
}

bool DisplayPowerMgrService::RestoreBrightnessInner(uint32_t displayId, uint32_t duration)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGI(COMP_SVC, "RestoreBrightness displayId=%{public}u, duration=%{public}d",
        displayId, duration);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    bool ret = BrightnessManager::Get().RestoreBrightness(duration);
    if (ret) {
        return true;
    }
    return iter->second->RestoreBrightness();
}

uint32_t DisplayPowerMgrService::GetBrightnessInner(uint32_t displayId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "GetBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return BRIGHTNESS_OFF;
    }
    return BrightnessManager::Get().GetBrightness();
}

uint32_t DisplayPowerMgrService::GetDefaultBrightnessInner()
{
    return BRIGHTNESS_DEFAULT;
}

uint32_t DisplayPowerMgrService::GetMaxBrightnessInner()
{
    return BRIGHTNESS_MAX;
}

uint32_t DisplayPowerMgrService::GetMinBrightnessInner()
{
    return BRIGHTNESS_MIN;
}

bool DisplayPowerMgrService::AdjustBrightnessInner(uint32_t id, int32_t value, uint32_t duration)
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

bool DisplayPowerMgrService::IsSupportLightSensor()
{
    return BrightnessManager::Get().IsSupportLightSensor();
}

bool DisplayPowerMgrService::IsAutoAdjustBrightnessInner()
{
    return BrightnessManager::Get().IsAutoAdjustBrightness();
}

bool DisplayPowerMgrService::AutoAdjustBrightnessInner(bool enable)
{
    if (!Permission::IsSystem()) {
        return false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness start");
    return BrightnessManager::Get().AutoAdjustBrightness(enable);
}

bool DisplayPowerMgrService::RegisterCallbackInner(sptr<IDisplayPowerCallback> callback)
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

bool DisplayPowerMgrService::BoostBrightnessInner(int32_t timeoutMs, uint32_t displayId)
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

bool DisplayPowerMgrService::CancelBoostBrightnessInner(uint32_t displayId)
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

uint32_t DisplayPowerMgrService::GetDeviceBrightnessInner(uint32_t displayId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "GetDeviceBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return BRIGHTNESS_OFF;
    }
    return BrightnessManager::Get().GetDeviceBrightness();
}

bool DisplayPowerMgrService::SetCoordinatedInner(bool coordinated, uint32_t displayId)
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

uint32_t DisplayPowerMgrService::SetLightBrightnessThresholdInner(
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
    result.append(IsSupportLightSensor() ? "TRUE" : "FALSE");
    result.append("\n");

    result.append("Auto Adjust Brightness: ");
    result.append(IsAutoAdjustBrightnessInner() ? "ON" : "OFF");
    result.append("\n");
#endif

    result.append("Brightness Limits: ").append("Max=" + std::to_string(GetMaxBrightnessInner()) + " ");
    result.append("Min=" + std::to_string(GetMinBrightnessInner()) + " ");
    result.append("Default=" + std::to_string(GetDefaultBrightnessInner())).append("\n");

    if (!SaveStringToFd(fd, result)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to save dump info to fd");
    }
    return ERR_OK;
}

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

DisplayErrors DisplayPowerMgrService::GetError()
{
    DisplayErrors tmpError = static_cast<DisplayErrors>(lastError_.load());
    lastError_ = static_cast<int32_t>(DisplayErrors::ERR_OK);
    return tmpError;
}

void DisplayPowerMgrService::UnregisterCallbackInner()
{
    std::lock_guard lock(mutex_);
    callback_ = nullptr;
}

void DisplayPowerMgrService::CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DISPLAY_HILOGI(COMP_SVC, "CallbackDeathRecipient OnRemoteDied");
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "OnRemoteDied no service");
        return;
    }

    pms->UnregisterCallbackInner();
}

/**
* @brief Function to limit maximum screen brightness
* @param value  The max brightness level that needs to be restricted
* @param mode  0 = default mode, set param value as maxBrightness;
*               1 = enter testMode, when running unittest set maxBrightness to default value;
*               2 = exit testMode
* @return false = set failed; true = set Sucess
*/
bool DisplayPowerMgrService::SetMaxBrightnessInner(double value, uint32_t mode)
{
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        DISPLAY_HILOGE(COMP_SVC, "SetMaxBrightness Permission Error!");
        return false;
    }
    if (mode == TEST_MODE) {
        isInTestMode_ = true;
        DISPLAY_HILOGI(COMP_SVC, "SetMaxBrightness enter TestMode");
    }
    if (mode == NORMAL_MODE) {
        isInTestMode_ = false;
        DISPLAY_HILOGI(COMP_SVC, "SetMaxBrightness cancel TestMode");
    }
    if (isInTestMode_) {
        DISPLAY_HILOGI(COMP_SVC, "in the TestMode SetMaxBrightness to Default Value!");
        double maxBrightness = 1.0;
        return BrightnessManager::Get().SetMaxBrightness(maxBrightness);
    }
    return BrightnessManager::Get().SetMaxBrightness(value);
}

/**
* @brief Function to limit maximum screen brightness
* @param maxNit  The max brightness Nit that needs to be restricted
* @param mode  0 = default mode, set param value as maxBrightness;
*               1 = enter testMode, when running unittest set maxBrightness to default value;
*               2 = exit testMode
* @return false = set failed; true = set Sucess
*/
bool DisplayPowerMgrService::SetMaxBrightnessNitInner(uint32_t maxNit, uint32_t mode)
{
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        DISPLAY_HILOGE(COMP_SVC, "SetMaxBrightness Permission Error!");
        return false;
    }
    if (mode == TEST_MODE) {
        isInTestMode_ = true;
        DISPLAY_HILOGI(COMP_SVC, "SetMaxBrightness enter TestMode");
    }
    if (mode == NORMAL_MODE) {
        isInTestMode_ = false;
        DISPLAY_HILOGI(COMP_SVC, "SetMaxBrightness cancel TestMode");
    }
    if (isInTestMode_) {
        DISPLAY_HILOGI(COMP_SVC, "in the TestMode SetMaxBrightness to Default Value!");
        uint32_t default_max_nit = 600;
        return BrightnessManager::Get().SetMaxBrightnessNit(default_max_nit);
    }
    return BrightnessManager::Get().SetMaxBrightnessNit(maxNit);
}

int DisplayPowerMgrService::NotifyScreenPowerStatusInner(uint32_t displayId, uint32_t displayPowerStatus)
{
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        return -1; // -1 means failed
    }
    DISPLAY_HILOGI(COMP_SVC, "[UL_POWER]NotifyScreenPowerStatus displayId=%{public}u, Status=%{public}u", displayId,
        displayPowerStatus);
    return BrightnessManager::Get().NotifyScreenPowerStatus(displayId, displayPowerStatus);
}

ErrCode DisplayPowerMgrService::SetScreenDisplayState(uint64_t screenId, uint32_t state, uint32_t reason)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetScreenDisplayState");
    if (!Permission::IsSystem()) {
        return static_cast<ErrCode>(DisplayErrors::ERR_SYSTEM_API_DENIED);
    }
    Rosen::ScreenPowerState status = Rosen::ScreenPowerState::POWER_ON;
    if (state == static_cast<uint32_t>(DisplayState::DISPLAY_ON)) {
        status = Rosen::ScreenPowerState::POWER_ON;
    } else if (state == static_cast<uint32_t>(DisplayState::DISPLAY_OFF)) {
        status = Rosen::ScreenPowerState::POWER_OFF;
    } else {
        return static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID);
    }
    bool ret = Rosen::DisplayManagerLite::GetInstance().SetScreenPowerById(static_cast<Rosen::ScreenId>(screenId),
        status, static_cast<Rosen::PowerStateChangeReason>(reason));
    DISPLAY_HILOGI(COMP_SVC,
        "SetScreenDisplayState, screenId: %{public}u, status: %{public}u, reason: %{public}u, ret: %{public}u",
        static_cast<uint32_t>(screenId), status, reason, ret);
    if (ret == false) {
        return static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID);
    }
    if (state == static_cast<uint32_t>(DisplayState::DISPLAY_ON)) {
        BrightnessManager::Get().SetScreenOnBrightness();
    }
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetDisplayState(uint32_t id, uint32_t state, uint32_t reason, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetDisplayState");
    result = SetDisplayStateInner(id, static_cast<DisplayState>(state), reason);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetDisplayState(uint32_t id, int32_t& displayState)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDisplayState");
    displayState = static_cast<int32_t>(GetDisplayStateInner(id));
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetDisplayIds(std::vector<uint32_t>& ids)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDisplayIds");
    auto idsTemp = GetDisplayIdsInner();
    ids = std::move(idsTemp);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetMainDisplayId(uint32_t& id)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetMainDisplayId");
    id = GetMainDisplayIdInner();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetBrightness(uint32_t value, uint32_t displayId, bool continuous, bool& result,
    int32_t& displayError)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetBrightness");
    result = SetBrightnessInner(value, displayId, continuous);
    displayError = static_cast<int32_t>(GetError());
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::DiscountBrightness(double discount, uint32_t displayId, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::DiscountBrightness");
    result = DiscountBrightnessInner(discount, displayId);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::OverrideBrightness(uint32_t brightness, uint32_t displayId, uint32_t duration,
    bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::OverrideBrightness");
    result = OverrideBrightnessInner(brightness, displayId, duration);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::OverrideDisplayOffDelay(uint32_t delayMs, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::OverrideDisplayOffDelay");
    result = OverrideDisplayOffDelayInner(delayMs);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::RestoreBrightness(uint32_t displayId, uint32_t duration, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetBrightness");
    result = RestoreBrightnessInner(displayId, duration);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetBrightness(uint32_t displayId, uint32_t& brightness)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetBrightness");
    brightness = GetBrightnessInner(displayId);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetDefaultBrightness(uint32_t& defaultBrightness)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDefaultBrightness");
    defaultBrightness = GetDefaultBrightnessInner();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetMaxBrightness(uint32_t& maxBrightness)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetMaxBrightness");
    maxBrightness = GetMaxBrightnessInner();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetMinBrightness(uint32_t& minBrightness)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetMinBrightness");
    minBrightness = GetMinBrightnessInner();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::AdjustBrightness");
    result = AdjustBrightnessInner(id, value, duration);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::AutoAdjustBrightness(bool enable, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::AutoAdjustBrightness");
    result = AutoAdjustBrightnessInner(enable);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::IsAutoAdjustBrightness(bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::IsAutoAdjustBrightness");
    result = IsAutoAdjustBrightnessInner();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::RegisterCallback(const sptr<IDisplayPowerCallback>& callback, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::RegisterCallback");
    result = RegisterCallbackInner(callback);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::BoostBrightness(int32_t timeoutMs, uint32_t displayId, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::BoostBrightness");
    result = BoostBrightnessInner(timeoutMs, displayId);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::CancelBoostBrightness(uint32_t displayId, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::CancelBoostBrightness");
    result = CancelBoostBrightnessInner(displayId);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::GetDeviceBrightness(uint32_t displayId, uint32_t& deviceBrightness)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDeviceBrightness");
    deviceBrightness = GetDeviceBrightnessInner(displayId);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetCoordinated(bool coordinated, uint32_t displayId, bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetCoordinated");
    result = SetCoordinatedInner(coordinated, displayId);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetLightBrightnessThreshold(const std::vector<int32_t>& threshold,
    const sptr<IDisplayBrightnessCallback>& callback, uint32_t& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetLightBrightnessThreshold");
    result = SetLightBrightnessThresholdInner(threshold, callback);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetMaxBrightness(double value, uint32_t mode, bool& result, int32_t& displayError)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetMaxBrightness");
    result = SetMaxBrightnessInner(value, mode);
    displayError = static_cast<int32_t>(GetError());
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetMaxBrightnessNit(uint32_t maxNit, uint32_t mode, bool& result,
    int32_t& displayError)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetMaxBrightnessNit");
    result = SetMaxBrightnessNitInner(maxNit, mode);
    displayError = static_cast<int32_t>(GetError());
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::SetScreenOnBrightness(bool& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::SetScreenOnBrightness");
    result = SetScreenOnBrightnessInner();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::NotifyScreenPowerStatus(uint32_t displayId, uint32_t displayPowerStatus,
    int32_t& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::NotifyScreenPowerStatus");
    result = NotifyScreenPowerStatusInner(displayId, displayPowerStatus);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::WaitDimmingDone()
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::WaitDimmingDone");
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        return -1; // -1 means failed
    }
    BrightnessManager::Get().WaitDimmingDone();
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::RunJsonCommand(const std::string& request, std::string& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::RunJsonCommand");
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        return -1; // -1 means failed
    }
    if (request.length() > MAX_PARAMS_LENGTH) {
        DISPLAY_HILOGE(COMP_SVC, "RunJsonCommand, params too long: %{public}zu", request.length());
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_PARAM_INVALID);
        return -1;
    }
    result = BrightnessManager::Get().RunJsonCommand(request);
    return ERR_OK;
}

std::string DisplayPowerMgrService::GetCallerIdWithPid(const std::string& callerId)
{
    return std::to_string(IPCSkeleton::GetCallingPid()) + "_" + callerId;
}

ErrCode DisplayPowerMgrService::RegisterDataChangeListener(const sptr<IDisplayBrightnessListener>& listener,
    DisplayDataChangeListenerType listenerType, const std::string& callerId, const std::string& params,
    int32_t& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::RegisterDataChangeListener");
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        return -1; // -1 means failed
    }
    if (callerId.length() > MAX_PARAMS_LENGTH || params.length() > MAX_PARAMS_LENGTH) {
        DISPLAY_HILOGE(COMP_SVC, "RegisterDataChangeListener, params too long: %{public}zu, %{public}zu",
            callerId.length(), params.length());
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_PARAM_INVALID);
        return -1;
    }
    auto id = GetCallerIdWithPid(callerId);
    result = BrightnessManager::Get().RegisterDataChangeListener(listener, listenerType, id, params);
    return ERR_OK;
}

ErrCode DisplayPowerMgrService::UnregisterDataChangeListener(
    DisplayDataChangeListenerType listenerType, const std::string& callerId, int32_t& result)
{
    DisplayXCollie displayXCollie("DisplayPowerMgrService::UnregisterDataChangeListener");
    if (!Permission::IsSystem()) {
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED);
        return -1; // -1 means failed
    }
    if (callerId.length() > MAX_PARAMS_LENGTH) {
        DISPLAY_HILOGE(COMP_SVC, "UnregisterDataChangeListener, params too long: %{public}zu", callerId.length());
        lastError_ = static_cast<int32_t>(DisplayErrors::ERR_PARAM_INVALID);
        return -1;
    }
    result = BrightnessManager::Get().UnregisterDataChangeListener(listenerType, GetCallerIdWithPid(callerId));
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
