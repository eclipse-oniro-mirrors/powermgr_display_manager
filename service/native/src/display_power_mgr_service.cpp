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

#include <cinttypes>
#include <file_ex.h>
#include <securec.h>
#include <string_ex.h>

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr uint32_t MIN_BRIGHTNESS_VALUE = 0;
constexpr uint32_t DEFAULT_BRIGHTNESS_VALUE = 102; // 255 * 40%
constexpr uint32_t MAX_BRIGHTNESS_VALUE  = 255;
}

DisplayPowerMgrService::DisplayPowerMgrService()
{
    DISPLAY_HILOGI(COMP_SVC, "DisplayPowerMgrService Create");
    action_ = std::make_shared<ScreenAction>();
    std::vector<uint64_t> displayIds = action_->GetDisplayIds();
    uint32_t count = displayIds.size();
    for (uint32_t i = 0; i < count; i++) {
        DISPLAY_HILOGI(COMP_SVC, "find display, id=%{public}" PRIu64 "", displayIds[i]);
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
    return iterater->second->UpdateState(state, reason);
}

DisplayState DisplayPowerMgrService::GetDisplayState(uint32_t id)
{
    DISPLAY_HILOGI(COMP_SVC, "GetDisplayState %{public}d", id);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    return iterater->second->GetState();
}

std::vector<uint32_t> DisplayPowerMgrService::GetDisplayIds()
{
    std::vector<uint32_t> ids;
    for (auto iter = controllerMap_.begin(); iter != controllerMap_.end(); iter++) {
        ids.push_back(iter->first);
    }
    return ids;
}

uint32_t DisplayPowerMgrService::GetMainDisplayId()
{
    uint64_t id = action_->GetDefaultDisplayId();
    DISPLAY_HILOGI(COMP_SVC, "GetMainDisplayId %{public}d", static_cast<uint32_t>(id));
    return static_cast<uint32_t>(id);
}

bool DisplayPowerMgrService::SetBrightness(uint32_t value, uint32_t displayId)
{
    auto brightnessValue = value;
    if (brightnessValue > MAX_BRIGHTNESS_VALUE) {
        DISPLAY_HILOGW(COMP_SVC, "SetBrightness value is greater than max, value=%{public}u", value);
        brightnessValue = MAX_BRIGHTNESS_VALUE;
    }
    if (brightnessValue < MIN_BRIGHTNESS_VALUE) {
        DISPLAY_HILOGW(COMP_SVC, "SetBrightness value is less than min, value=%{public}u", value);
        brightnessValue = MIN_BRIGHTNESS_VALUE;
    }
    DISPLAY_HILOGI(COMP_SVC, "SetBrightness displayId=%{public}u, value=%{public}u", displayId, brightnessValue);
    auto iter = controllerMap_.find(displayId);
    if (iter == controllerMap_.end()) {
        return false;
    }
    return iter->second->UpdateBrightness(brightnessValue);
}

bool DisplayPowerMgrService::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration)
{
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayState %{public}d, %{public}d, %{public}d",
        id, value, duration);
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->UpdateBrightness(value, duration);
}

bool DisplayPowerMgrService::AutoAdjustBrightness(bool enable)
{
    DISPLAY_HILOGI(COMP_SVC, "AutoAdjustBrightness start");
    if (!supportLightSensor_) {
        DISPLAY_HILOGI(COMP_SVC, "AutoAdjustBrightness not support");
        return false;
    }
    if (enable) {
        DISPLAY_HILOGI(COMP_SVC, "AutoAdjustBrightness enable");
        if (autoBrightness_) {
            DISPLAY_HILOGW(COMP_SVC, "AutoAdjustBrightness is already enabled");
            return true;
        }
        autoBrightness_ = true;
        if (GetDisplayState(GetMainDisplayId()) == DisplayState::DISPLAY_ON) {
            ActivateAmbientSensor();
        }
    } else {
        DISPLAY_HILOGI(COMP_SVC, "AutoAdjustBrightness disable");
        if (!autoBrightness_) {
            DISPLAY_HILOGW(COMP_SVC, "AutoAdjustBrightness is already disabled");
            return true;
        }
        DeactivateAmbientSensor();
        autoBrightness_ = false;
    }
    return true;
}

void DisplayPowerMgrService::ActivateAmbientSensor()
{
    DISPLAY_HILOGI(COMP_SVC, "ActivateAmbientSensor");
    if (!autoBrightness_) {
        DISPLAY_HILOGI(COMP_SVC, "auto brightness is not enabled");
        return;
    }
    if (ambientSensorEnabled_) {
        DISPLAY_HILOGI(COMP_SVC, "Ambient Sensor is already on");
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
    DISPLAY_HILOGI(COMP_SVC, "ActivateAmbientSensor");
    if (!autoBrightness_) {
        DISPLAY_HILOGI(COMP_SVC, "auto brightness is not enabled");
        return;
    }
    if (!ambientSensorEnabled_) {
        DISPLAY_HILOGI(COMP_SVC, "Ambient Sensor is already off");
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
    return iterater->second->UpdateStateConfig(state, value);
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

void DisplayPowerMgrService::NotifyStateChangeCallback(uint32_t displayId, DisplayState state)
{
    if (callback_ != nullptr) {
        callback_->OnDisplayStateChanged(displayId, state);
    }
}

int32_t DisplayPowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::string result("DISPLAY POWER MANAGER DUMP:\n");
    for (auto iter = controllerMap_.begin(); iter != controllerMap_.end(); iter++) {
        result.append("Display Id=");
        result.append(std::to_string(iter->first));
        result.append(" State=");
        result.append(std::to_string(static_cast<uint32_t>(iter->second->GetState())));
        result.append(" Brightness=");
        result.append(std::to_string(iter->second->GetBrightness()));
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
        DISPLAY_HILOGE(COMP_SVC, "Failed to save dump info to fd");
    }
    return ERR_OK;
}

void DisplayPowerMgrService::InitSensors()
{
    DISPLAY_HILOGI(COMP_SVC, "InitSensors start");
    SensorInfo* sensorInfo = nullptr;
    int32_t count;
    int ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "Can't get sensors");
        return;
    }
    supportLightSensor_ = false;
    for (int i = 0; i < count; i++) {
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
            DISPLAY_HILOGI(COMP_SVC, "AMBIENT_LIGHT Support");
            supportLightSensor_ = true;
            break;
        }
    }

    if (!supportLightSensor_) {
        DISPLAY_HILOGI(COMP_SVC, "AMBIENT_LIGHT not support");
    }
    free(sensorInfo);
}

void DisplayPowerMgrService::AmbientLightCallback(SensorEvent *event)
{
    DISPLAY_HILOGI(COMP_SVC, "AmbientSensorCallback");
    if (event->sensorTypeId != SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        DISPLAY_HILOGI(COMP_SVC, "Sensor Callback is not AMBIENT_LIGHT");
        return;
    }
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "Sensor Callback no service");
        return;
    }
    uint32_t mainDispId = pms->GetMainDisplayId();
    auto mainDisp = pms->controllerMap_.find(mainDispId);
    if (mainDisp == pms->controllerMap_.end()) {
        return;
    }
    AmbientLightData* data = (AmbientLightData*)event->data;
    DISPLAY_HILOGI(COMP_SVC, "AmbientLightCallback: %{public}f", data->intensity);
    int32_t brightness = static_cast<int32_t>(mainDisp->second->GetBrightness());
    if (pms->CalculateBrightness(data->intensity, brightness)) {
        pms->AdjustBrightness(mainDispId, brightness, AUTO_ADJUST_BRIGHTNESS_DURATION);
    }
}

bool DisplayPowerMgrService::IsChangedLux(float scalar)
{
    DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: (%{public}d), %{public}f vs %{public}f",
        luxChanged_, lastLux_, scalar);

    if (lastLuxTime_ <= 0) {
        DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: receive lux at first time");
        lastLuxTime_ = time(0);
        lastLux_ = scalar;
        luxChanged_ = true;
        return false;
    }

    if (!luxChanged_) {
        float luxChangeMin = (lastLux_ / LUX_CHANGE_RATE_THRESHOLD) + 1;
        if (abs(scalar - lastLux_) < luxChangeMin) {
            DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: Too little change");
            return false;
        } else {
            DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: First time to change, wait for stable");
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
            DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: stable lux");
            if (sub >= LUX_STABLE_TIME) {
                DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: stable enought to change");
                lastLuxTime_ = time(0);
                lastLux_ = scalar;
                luxChanged_ = false;
                return true;
            }
        } else {
            DISPLAY_HILOGI(COMP_SVC, "IsChangedLux: unstable lux, wait for stable");
            luxChanged_ = true;
            return false;
        }
    }
    return false;
}

bool DisplayPowerMgrService::CalculateBrightness(float scalar, int32_t& brightness)
{
    const float lastLux = lastLux_;
    if (!IsChangedLux(scalar)) {
        DISPLAY_HILOGI(COMP_SVC, "Lux is not change");
        return false;
    }
    int32_t change = GetBrightnessFromLightScalar(scalar);
    DISPLAY_HILOGI(COMP_SVC, "lux: %{public}f -> %{public}f, screen: %{public}d -> %{public}d",
        lastLux, scalar, brightness, change);
    if (abs(change - brightness) < BRIGHTNESS_CHANGE_MIN
        || (scalar > lastLux && change < brightness)
        || (scalar < lastLux && change > brightness)) {
        DISPLAY_HILOGI(COMP_SVC, "screen is too light/dark when calculated change");
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
    DISPLAY_HILOGI(COMP_SVC, "nit: %{public}d", nit);

    int32_t brightness = BRIGHTNESS_MIN
        + ((BRIGHTNESS_MAX - BRIGHTNESS_MIN) * (nit - NIT_MIN) / (NIT_MAX - NIT_MIN));
    DISPLAY_HILOGI(COMP_SVC, "brightness: %{public}d", brightness);

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
