/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <file_ex.h>
#include <securec.h>
#include <string_ex.h>

#include "hilog_wrapper.h"

namespace OHOS {
namespace DisplayPowerMgr {
DisplayPowerMgrService::DisplayPowerMgrService()
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService Create");
    action_ = std::make_shared<ScreenAction>();
    std::vector<uint32_t> displayIds = action_->GetDisplayIds();
    uint32_t count = displayIds.size();
    for (uint32_t i = 0; i < count; i++) {
        DISPLAY_HILOGI(MODULE_SERVICE, "find display, id=%{public}u", displayIds[i]);
        controllerMap_.emplace(displayIds[i], std::make_shared<ScreenController>(displayIds[i], action_));
    }
    callback_ = nullptr;
    cbDeathRecipient_ = nullptr;
    InitSensors();
}

DisplayPowerMgrService::~DisplayPowerMgrService()
{
}

bool DisplayPowerMgrService::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "SetDisplayState %{public}d, %{public}d", id, state);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    if (id == GetMainDisplayId()) {
        DISPLAY_HILOGI(MODULE_SERVICE, "change ambient sensor status");
        if (state == DisplayState::DISPLAY_ON) {
            ActivateAmbientSensor();
        } else if (state == DisplayState::DISPLAY_OFF) {
            DeactivateAmbientSensor();
        }
    }
    return iterater->second->UpdateState(state, reason);
}

DisplayState DisplayPowerMgrService::GetDisplayState(uint32_t id)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "GetDisplayState %{public}d", id);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    return iterater->second->GetState();
}

std::vector<uint32_t> DisplayPowerMgrService::GetDisplayIds()
{
    DISPLAY_HILOGI(MODULE_SERVICE, "GetDisplayIds");
    std::vector<uint32_t> ids;
    for (auto& iter: controllerMap_) {
        ids.push_back(iter.first);
    }
    return ids;
}

uint32_t DisplayPowerMgrService::GetMainDisplayId()
{
    uint32_t id = action_->GetDefaultDisplayId();
    DISPLAY_HILOGI(MODULE_SERVICE, "GetMainDisplayId %{public}d", id);
    return id;
}

bool DisplayPowerMgrService::SetBrightness(uint32_t value, uint32_t displayId)
{
    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(MODULE_SERVICE, "SetBrightness displayId=%{public}u, value=%{public}u", displayId, brightness);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->SetBrightness(brightness);
}

bool DisplayPowerMgrService::OverrideBrightness(uint32_t value, uint32_t displayId)
{
    auto brightness = GetSafeBrightness(value);
    DISPLAY_HILOGI(MODULE_SERVICE, "OverrideBrightness displayId=%{public}u, value=%{public}u", displayId, brightness);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->OverrideBrightness(brightness);
}

bool DisplayPowerMgrService::RestoreBrightness(uint32_t displayId)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "RestoreBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->RestoreBrightness();
}

uint32_t DisplayPowerMgrService::GetBrightness(uint32_t displayId)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "GetBrightness displayId=%{public}u", displayId);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return BRIGHTNESS_OFF;
    }
    return iter->second->GetBrightness();
}

bool DisplayPowerMgrService::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "SetDisplayState %{public}d, %{public}d, %{public}d",
        id, value, duration);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->SetBrightness(value, duration);
}

bool DisplayPowerMgrService::AutoAdjustBrightness(bool enable)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "AutoAdjustBrightness start");
    if (!supportLightSensor_) {
        DISPLAY_HILOGI(MODULE_SERVICE, "AutoAdjustBrightness not support");
        return false;
    }
    if (enable) {
        DISPLAY_HILOGI(MODULE_SERVICE, "AutoAdjustBrightness enable");
        if (autoBrightness_) {
            DISPLAY_HILOGW(MODULE_SERVICE, "AutoAdjustBrightness is already enabled");
            return true;
        }
        autoBrightness_ = true;
        if (GetDisplayState(GetMainDisplayId()) == DisplayState::DISPLAY_ON) {
            ActivateAmbientSensor();
        }
    } else {
        DISPLAY_HILOGI(MODULE_SERVICE, "AutoAdjustBrightness disable");
        if (!autoBrightness_) {
            DISPLAY_HILOGW(MODULE_SERVICE, "AutoAdjustBrightness is already disabled");
            return true;
        }
        DeactivateAmbientSensor();
        autoBrightness_ = false;
    }
    return true;
}

bool DisplayPowerMgrService::IsAutoAdjustBrightness()
{
    DISPLAY_HILOGW(MODULE_SERVICE, "Automatic brightness mode: %{public}d", autoBrightness_);
    return autoBrightness_;
}

void DisplayPowerMgrService::ActivateAmbientSensor()
{
    DISPLAY_HILOGI(MODULE_SERVICE, "ActivateAmbientSensor");
    if (!autoBrightness_) {
        DISPLAY_HILOGI(MODULE_SERVICE, "auto brightness is not enabled");
        return;
    }
    if (ambientSensorEnabled_) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Ambient Sensor is already on");
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
    DISPLAY_HILOGI(MODULE_SERVICE, "ActivateAmbientSensor");
    if (!autoBrightness_) {
        DISPLAY_HILOGI(MODULE_SERVICE, "auto brightness is not enabled");
        return;
    }
    if (!ambientSensorEnabled_) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Ambient Sensor is already off");
        return;
    }
    DeactivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_);
    UnsubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &user_);
    ambientSensorEnabled_ = false;
}

bool DisplayPowerMgrService::SetStateConfig(uint32_t id, DisplayState state, int32_t value)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "SetStateConfig %{public}d, %{public}d, %{public}d",
        id, state, value);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->UpdateStateConfig(state, value);
}

bool DisplayPowerMgrService::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "RegisterCallback");
    if (callback_ != nullptr) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Callback function exist");
        return false;
    }
    callback_ = callback;
    sptr<IRemoteObject> remote = callback_->AsObject();
    if (!remote->IsProxyObject()) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Callback is not proxy");
        return false;
    }
    if (cbDeathRecipient_ == nullptr) {
        cbDeathRecipient_ = new CallbackDeathRecipient();
    }
    remote->AddDeathRecipient(cbDeathRecipient_);
    return true;
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
        result.append("Display Id=");
        result.append(std::to_string(iter.first));
        result.append(" State=");
        result.append(std::to_string(static_cast<uint32_t>(iter.second->GetState())));
        if (!iter.second->IsBrightnessOverride()) {
            result.append(" Brightness=");
            result.append(std::to_string(iter.second->GetBrightness()));
        } else {
            result.append(" Brightness=");
            result.append(std::to_string(iter.second->GetBeforeOverrideBrightness()));
            result.append(" OverrideBrightness=");
            result.append(std::to_string(iter.second->GetBrightness()));
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

    if (!SaveStringToFd(fd, result)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to save dump info to fd");
    }
    return ERR_OK;
}

void DisplayPowerMgrService::InitSensors()
{
    DISPLAY_HILOGI(MODULE_SERVICE, "InitSensors start");
    SensorInfo* sensorInfo = nullptr;
    int32_t count;
    int ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Can't get sensors");
        return;
    }
    supportLightSensor_ = false;
    for (int i = 0; i < count; i++) {
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
            DISPLAY_HILOGI(MODULE_SERVICE, "AMBIENT_LIGHT Support");
            supportLightSensor_ = true;
            break;
        }
    }

    if (!supportLightSensor_) {
        DISPLAY_HILOGI(MODULE_SERVICE, "AMBIENT_LIGHT not support");
    }
    free(sensorInfo);
}

void DisplayPowerMgrService::AmbientLightCallback(SensorEvent *event)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "AmbientSensorCallback");
    if (event->sensorTypeId != SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Sensor Callback is not AMBIENT_LIGHT");
        return;
    }
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Sensor Callback no service");
        return;
    }
    uint32_t mainDispId = pms->GetMainDisplayId();
    auto mainDisp = pms->controllerMap_.find(mainDispId);
    if (mainDisp == pms->controllerMap_.end()) {
        return;
    }
    AmbientLightData* data = (AmbientLightData*)event->data;
    DISPLAY_HILOGI(MODULE_SERVICE, "AmbientLightCallback: %{public}f", data->intensity);
    int32_t brightness = static_cast<int32_t>(mainDisp->second->GetBrightness());
    if (pms->CalculateBrightness(data->intensity, brightness)) {
        pms->AdjustBrightness(mainDispId, brightness, AUTO_ADJUST_BRIGHTNESS_DURATION);
    }
}

bool DisplayPowerMgrService::IsChangedLux(float scalar)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: (%{public}d), %{public}f vs %{public}f",
        luxChanged_, lastLux_, scalar);

    if (lastLuxTime_ <= 0) {
        DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: receive lux at first time");
        lastLuxTime_ = time(0);
        lastLux_ = scalar;
        luxChanged_ = true;
        return false;
    }

    if (!luxChanged_) {
        float luxChangeMin = (lastLux_ / LUX_CHANGE_RATE_THRESHOLD) + 1;
        if (abs(scalar - lastLux_) < luxChangeMin) {
            DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: Too little change");
            return false;
        } else {
            DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: First time to change, wait for stable");
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
            DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: stable lux");
            if (sub >= LUX_STABLE_TIME) {
                DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: stable enought to change");
                lastLuxTime_ = time(0);
                lastLux_ = scalar;
                luxChanged_ = false;
                return true;
            }
        } else {
            DISPLAY_HILOGI(MODULE_SERVICE, "IsChangedLux: unstable lux, wait for stable");
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
        DISPLAY_HILOGW(MODULE_SERVICE, "brightness value is greater than max, value=%{public}u", value);
        brightnessValue = BRIGHTNESS_MAX;
    }
    if (brightnessValue < BRIGHTNESS_MIN) {
        DISPLAY_HILOGW(MODULE_SERVICE, "brightness value is less than min, value=%{public}u", value);
        brightnessValue = BRIGHTNESS_MIN;
    }
    return brightnessValue;
}

bool DisplayPowerMgrService::CalculateBrightness(float scalar, int32_t& brightness)
{
    const float lastLux = lastLux_;
    if (!IsChangedLux(scalar)) {
        DISPLAY_HILOGI(MODULE_SERVICE, "Lux is not change");
        return false;
    }
    int32_t change = GetBrightnessFromLightScalar(scalar);
    DISPLAY_HILOGI(MODULE_SERVICE, "lux: %{public}f -> %{public}f, screen: %{public}d -> %{public}d",
        lastLux, scalar, brightness, change);
    if (abs(change - brightness) < BRIGHTNESS_CHANGE_MIN
        || (scalar > lastLux && change < brightness)
        || (scalar < lastLux && change > brightness)) {
        DISPLAY_HILOGI(MODULE_SERVICE, "screen is too light/dark when calculated change");
        return false;
    }
    brightness = change;
    return true;
}

int32_t DisplayPowerMgrService::GetBrightnessFromLightScalar(float scalar)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "GetBrightnessFromLightScalar: %{public}f", scalar);
    // use simple quadratic equation (lux = (nit / 5) ^ 2) to calculate nit
    int32_t nit = static_cast<int32_t>(5 * sqrt(scalar));
    if (nit < NIT_MIN) {
        nit = NIT_MIN;
    } else if (nit > NIT_MAX) {
        nit = NIT_MAX;
    }
    DISPLAY_HILOGI(MODULE_SERVICE, "nit: %{public}d", nit);

    int32_t brightness = static_cast<int32_t>(BRIGHTNESS_MIN
        + ((BRIGHTNESS_MAX - BRIGHTNESS_MIN) * (nit - NIT_MIN) / (NIT_MAX - NIT_MIN)));
    DISPLAY_HILOGI(MODULE_SERVICE, "brightness: %{public}d", brightness);

    return brightness;
}

void DisplayPowerMgrService::CallbackDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "CallbackDeathRecipient OnRemoteDied");
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(MODULE_SERVICE, "OnRemoteDied no service");
        return;
    }

    pms->callback_ = nullptr;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
