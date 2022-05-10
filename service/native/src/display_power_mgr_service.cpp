/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include <string_ex.h>

#include "watchdog.h"

#include "display_log.h"
#include "display_param_helper.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
const std::string DISPLAY_SERVICE_NAME = "DisplayPowerManagerService";
constexpr int32_t WATCH_DOG_DELAY_S = 60;
}
DisplayPowerMgrService::DisplayPowerMgrService()
{
}

void DisplayPowerMgrService::Init()
{
    DISPLAY_HILOGI(COMP_SVC, "DisplayPowerMgrService Create");
    if (!eventRunner_) {
        eventRunner_ = AppExecFwk::EventRunner::Create(DISPLAY_SERVICE_NAME);
        if (eventRunner_ == nullptr) {
            DISPLAY_HILOGE(COMP_SVC, "Init failed due to create EventRunner");
            return;
        }
    }

    if (!handler_) {
        handler_ = std::make_shared<DisplayEventHandler>(eventRunner_,
            DelayedSpSingleton<DisplayPowerMgrService>::GetInstance());
        std::string handlerName("DisplayPowerEventHandler");
        HiviewDFX::Watchdog::GetInstance().AddThread(handlerName, handler_, WATCH_DOG_DELAY_S);
    }

    std::vector<uint32_t> displayIds = ScreenAction::GetAllDisplayId();
    for (const auto& id : displayIds) {
        DISPLAY_HILOGI(COMP_SVC, "find display, id=%{public}u", id);
        controllerMap_.emplace(id, std::make_shared<ScreenController>(id, handler_));
    }
    callback_ = nullptr;
    cbDeathRecipient_ = nullptr;

    InitSensors();
}

bool DisplayPowerMgrService::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayState %{public}d, %{public}d", id, state);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    if (id == GetMainDisplayId()) {
        DISPLAY_HILOGI(COMP_SVC, "change ambient sensor status");
        if (state == DisplayState::DISPLAY_ON) {
            ActivateAmbientSensor();
        } else if (state == DisplayState::DISPLAY_OFF) {
            DeactivateAmbientSensor();
        }
    }
    return iterater->second->DisplayStateControl()->UpdateState(state, reason);
}

DisplayState DisplayPowerMgrService::GetDisplayState(uint32_t id)
{
    DISPLAY_HILOGI(COMP_SVC, "GetDisplayState %{public}d", id);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    return iterater->second->DisplayStateControl()->GetState();
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
    DISPLAY_HILOGI(COMP_SVC, "GetMainDisplayId %{public}d", id);
    return id;
}

bool DisplayPowerMgrService::SetBrightness(uint32_t value, uint32_t displayId)
{
    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBrightness displayId=%{public}u, value=%{public}u", displayId, brightness);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->SharedControl()->SetBrightness(brightness);
}

bool DisplayPowerMgrService::OverrideBrightness(uint32_t value, uint32_t displayId)
{
    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(COMP_SVC, "OverrideBrightness displayId=%{public}u, value=%{public}u", displayId, brightness);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->OverrideControl()->OverrideBrightness(brightness);
}

bool DisplayPowerMgrService::RestoreBrightness(uint32_t displayId)
{
    DISPLAY_HILOGI(COMP_SVC, "RestoreBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->OverrideControl()->RestoreBrightness();
}

uint32_t DisplayPowerMgrService::GetBrightness(uint32_t displayId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "GetBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return BRIGHTNESS_OFF;
    }
    return iter->second->SharedControl()->GetBrightness();
}

uint32_t DisplayPowerMgrService::GetDefaultBrightness()
{
    return DisplayParamHelper::GetInstance().GetDefaultBrightness();
}

uint32_t DisplayPowerMgrService::GetMaxBrightness()
{
    return DisplayParamHelper::GetInstance().GetMaxBrightness();
}

uint32_t DisplayPowerMgrService::GetMinBrightness()
{
    return DisplayParamHelper::GetInstance().GetMinBrightness();
}

bool DisplayPowerMgrService::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetDisplayState %{public}d, %{public}d, %{public}d",
        id, value, duration);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->SharedControl()->SetBrightness(value, duration);
}

bool DisplayPowerMgrService::AutoAdjustBrightness(bool enable)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AutoAdjustBrightness start");
    if (!supportLightSensor_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AutoAdjustBrightness not support");
        return false;
    }
    if (enable) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AutoAdjustBrightness enable");
        if (autoBrightness_) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "AutoAdjustBrightness is already enabled");
            return true;
        }
        autoBrightness_ = true;
        if (GetDisplayState(GetMainDisplayId()) == DisplayState::DISPLAY_ON) {
            ActivateAmbientSensor();
        }
    } else {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AutoAdjustBrightness disable");
        if (!autoBrightness_) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "AutoAdjustBrightness is already disabled");
            return true;
        }
        DeactivateAmbientSensor();
        autoBrightness_ = false;
    }
    return true;
}

bool DisplayPowerMgrService::IsAutoAdjustBrightness()
{
    DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Automatic brightness mode: %{public}d", autoBrightness_);
    return autoBrightness_;
}

void DisplayPowerMgrService::ActivateAmbientSensor()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ActivateAmbientSensor");
    if (!autoBrightness_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "auto brightness is not enabled");
        return;
    }
    if (ambientSensorEnabled_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Ambient Sensor is already on");
        return;
    }
    strcpy_s(user_.name, sizeof(user_.name), "DisplayPowerMgrService");
    user_.userData = nullptr;
    user_.callback = &AmbientLightCallback;
    SubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_);
    SetBatch(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_, SAMPLING_RATE, SAMPLING_RATE);
    ActivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_);
    SetMode(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_, SENSOR_ON_CHANGE);
    ambientSensorEnabled_ = true;
}

void DisplayPowerMgrService::DeactivateAmbientSensor()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ActivateAmbientSensor");
    if (!autoBrightness_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "auto brightness is not enabled");
        return;
    }
    if (!ambientSensorEnabled_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Ambient Sensor is already off");
        return;
    }
    DeactivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_);
    UnsubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_);
    ambientSensorEnabled_ = false;
}

bool DisplayPowerMgrService::SetStateConfig(uint32_t id, DisplayState state, int32_t value)
{
    DISPLAY_HILOGI(COMP_SVC, "SetStateConfig %{public}d, %{public}d, %{public}d",
        id, state, value);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->DisplayStateControl()->UpdateStateConfig(state, value);
}

bool DisplayPowerMgrService::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
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
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Timing max brightness: %{public}d, id: %{public}d", timeoutMs, displayId);
    RETURN_IF_WITH_RET(timeoutMs <= 0, false);
    auto iter = controllerMap_.find(displayId);
    RETURN_IF_WITH_RET(iter == controllerMap_.end(), false);
    return iter->second->OverrideControl()->BoostBrightness(timeoutMs);
}

bool DisplayPowerMgrService::CancelBoostBrightness(uint32_t displayId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Cancel boost brightness, id: %{public}d", displayId);
    auto iter = controllerMap_.find(displayId);
    RETURN_IF_WITH_RET(iter == controllerMap_.end(), false);
    return iter->second->OverrideControl()->CancelBoostBrightness();
}

void DisplayPowerMgrService::NotifyStateChangeCallback(uint32_t displayId, DisplayState state)
{
    if (callback_ != nullptr) {
        callback_->OnDisplayStateChanged(displayId, state);
    }
}

int32_t DisplayPowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::string result("DISPLAY POWER MANAGER DUMP:\n");
    for (auto& iter: controllerMap_) {
        auto control = iter.second;
        result.append("Display Id=");
        result.append(std::to_string(iter.first));
        result.append(" State=");
        result.append(std::to_string(static_cast<uint32_t>(control->DisplayStateControl()->GetState())));
        bool isOverride = control->OverrideControl()->IsBrightnessOverride();
        bool isBoots = control->OverrideControl()->IsBoostBrightness();
        result.append(" Brightness=");
        if (isOverride) {
            result.append(std::to_string(control->OverrideControl()->GetBeforeBrightness()));
            result.append(" OverrideBrightness=");
            result.append(std::to_string(control->SharedControl()->GetBrightness()));
        } else if (isBoots) {
            result.append(std::to_string(control->OverrideControl()->GetBeforeBrightness()));
            result.append(" BoostBrightness=");
            result.append(std::to_string(control->SharedControl()->GetBrightness()));
        } else {
            result.append(std::to_string(control->SharedControl()->GetBrightness()));
        }
        result.append("\n");
    }

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

    result.append("Brightness Limits: ");
    result.append("Max=" + std::to_string(GetMaxBrightness()) + " ");
    result.append("Min=" + std::to_string(GetMinBrightness()) + " ");
    result.append("Default=" + std::to_string(GetDefaultBrightness()));
    result.append("\n");

    if (!SaveStringToFd(fd, result)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to save dump info to fd");
    }
    return ERR_OK;
}

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
    free(sensorInfo);
}

void DisplayPowerMgrService::AmbientLightCallback(SensorEvent *event)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AmbientSensorCallback");
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
    AmbientLightData* data = (AmbientLightData*)event->data;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AmbientLightCallback: %{public}f", data->intensity);
    int32_t brightness = static_cast<int32_t>(mainDisp->second->SharedControl()->GetBrightness());
    if (pms->CalculateBrightness(data->intensity, brightness)) {
        pms->AdjustBrightness(mainDispId, brightness, AUTO_ADJUST_BRIGHTNESS_DURATION);
    }
    // Notify ambient brightness change event to battery statistics
    HiviewDFX::HiSysEvent::Write(HiviewDFX::HiSysEvent::Domain::POWERMGR, "AMBIENT_LIGHT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "level", brightness);
}

bool DisplayPowerMgrService::IsChangedLux(float scalar)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: (%{public}d), %{public}f vs %{public}f",
        luxChanged_, lastLux_, scalar);

    if (lastLuxTime_ <= 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: receive lux at first time");
        lastLuxTime_ = time(0);
        lastLux_ = scalar;
        luxChanged_ = true;
        return false;
    }

    if (!luxChanged_) {
        float luxChangeMin = (lastLux_ / LUX_CHANGE_RATE_THRESHOLD) + 1;
        if (abs(scalar - lastLux_) < luxChangeMin) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: Too little change");
            return false;
        } else {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: First time to change, wait for stable");
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
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: stable lux");
            if (sub >= LUX_STABLE_TIME) {
                DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: stable enought to change");
                lastLuxTime_ = time(0);
                lastLux_ = scalar;
                luxChanged_ = false;
                return true;
            }
        } else {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "IsChangedLux: unstable lux, wait for stable");
            luxChanged_ = true;
            return false;
        }
    }
    return false;
}

uint32_t DisplayPowerMgrService::GetSafeBrightness(uint32_t value)
{
    auto brightnessValue = value;
    if (brightnessValue > BRIGHTNESS_MAX) {
        DISPLAY_HILOGW(COMP_SVC, "brightness value is greater than max, value=%{public}u", value);
        brightnessValue = BRIGHTNESS_MAX;
    }
    if (brightnessValue < BRIGHTNESS_MIN) {
        DISPLAY_HILOGW(COMP_SVC, "brightness value is less than min, value=%{public}u", value);
        brightnessValue = BRIGHTNESS_MIN;
    }
    return brightnessValue;
}

bool DisplayPowerMgrService::CalculateBrightness(float scalar, int32_t& brightness)
{
    const float lastLux = lastLux_;
    if (!IsChangedLux(scalar)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Lux is not change");
        return false;
    }
    int32_t change = GetBrightnessFromLightScalar(scalar);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "lux: %{public}f -> %{public}f, screen: %{public}d -> %{public}d",
        lastLux, scalar, brightness, change);
    if (abs(change - brightness) < BRIGHTNESS_CHANGE_MIN
        || (scalar > lastLux && change < brightness)
        || (scalar < lastLux && change > brightness)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "screen is too light/dark when calculated change");
        return false;
    }
    brightness = change;
    return true;
}

int32_t DisplayPowerMgrService::GetBrightnessFromLightScalar(float scalar)
{
    DISPLAY_HILOGI(COMP_SVC, "GetBrightnessFromLightScalar: %{public}f", scalar);
    // use simple quadratic equation (lux = (nit / 5) ^ 2) to calculate nit
    int32_t nit = static_cast<int32_t>(5 * sqrt(scalar));
    if (nit < NIT_MIN) {
        nit = NIT_MIN;
    } else if (nit > NIT_MAX) {
        nit = NIT_MAX;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "nit: %{public}d", nit);

    int32_t brightness = static_cast<int32_t>(BRIGHTNESS_MIN
        + ((BRIGHTNESS_MAX - BRIGHTNESS_MIN) * (nit - NIT_MIN) / (NIT_MAX - NIT_MIN)));
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "brightness: %{public}d", brightness);

    return brightness;
}

void DisplayPowerMgrService::CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DISPLAY_HILOGI(COMP_SVC, "CallbackDeathRecipient OnRemoteDied");
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "OnRemoteDied no service");
        return;
    }

    pms->callback_ = nullptr;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
