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

#include "display_power_mgr_client.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>
#include "new"
#include "refbase.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "display_log.h"
#include "display_common.h"
#include "display_power_info.h"
#include "idisplay_power_callback.h"
#include "idisplay_power_mgr.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace DisplayPowerMgr {
DisplayPowerMgrClient::DisplayPowerMgrClient() = default;
DisplayPowerMgrClient::~DisplayPowerMgrClient() = default;
namespace {
constexpr int32_t DEFAULT_VALUE = -1;
constexpr uint32_t BRIGHTNESS_DEFAULT_PROXY = 0;
}
sptr<IDisplayPowerMgr> DisplayPowerMgrClient::GetProxy()
{
    std::lock_guard lock(mutex_);
    if (proxy_ != nullptr) {
        return proxy_;
    }

    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        DISPLAY_HILOGE(COMP_FWK, "Failed to get system ability manager");
        lastError_ = DisplayErrors::ERR_CONNECTION_FAIL;
        return nullptr;
    }
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    if (obj == nullptr) {
        lastError_ = DisplayErrors::ERR_CONNECTION_FAIL;
        static uint32_t count = 0;
        DISPLAY_HILOGE(COMP_FWK, "Failed to get display manager service, count=%{public}u", ++count);
        return nullptr;
    }
    sptr<IRemoteObject::DeathRecipient> dr = new DisplayDeathRecipient(*this);
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(dr))) {
        DISPLAY_HILOGE(COMP_FWK, "Failed to add death recipient");
        lastError_ = DisplayErrors::ERR_CONNECTION_FAIL;
        return nullptr;
    }

    proxy_ = iface_cast<IDisplayPowerMgr>(obj);
    deathRecipient_ = dr;
    DISPLAY_HILOGI(COMP_FWK, "Succeed to connect display manager service, pid=%{public}d", getpid());
    return proxy_;
}

void DisplayPowerMgrClient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        DISPLAY_HILOGE(COMP_FWK, "remote is nullptr");
        return;
    }

    std::lock_guard lock(mutex_);
    RETURN_IF(proxy_ == nullptr);

    auto serviceRemote = proxy_->AsObject();
    if ((serviceRemote != nullptr) && (serviceRemote == remote.promote())) {
        serviceRemote->RemoveDeathRecipient(deathRecipient_);
        proxy_ = nullptr;
    }
}

bool DisplayPowerMgrClient::SetDisplayState(DisplayState state,
    PowerMgr::StateChangeReason reason, uint32_t id)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->SetDisplayState(id, static_cast<uint32_t>(state), static_cast<uint32_t>(reason), result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetDisplayState, ret = %{public}d", ret);
        return false;
    }
    return result;
}

DisplayState DisplayPowerMgrClient::GetDisplayState(uint32_t id)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    int32_t displayState = static_cast<int32_t>(DisplayState::DISPLAY_UNKNOWN);
    auto ret = proxy->GetDisplayState(id, displayState);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetDisplayState, ret = %{public}d", ret);
        return DisplayState::DISPLAY_UNKNOWN;
    }
    return static_cast<DisplayState>(displayState);
}

std::vector<uint32_t> DisplayPowerMgrClient::GetDisplayIds()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return {};
    }
    std::vector<uint32_t> ids;
    auto ret = proxy->GetDisplayIds(ids);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetDisplayIds, ret = %{public}d", ret);
    }
    return ids;
}

int32_t DisplayPowerMgrClient::GetMainDisplayId()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, INVALID_DISPLAY_ID);
    uint32_t id = DEFAULT_MAIN_DISPLAY_ID;
    auto ret = proxy->GetMainDisplayId(id);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetMainDisplayId, ret = %{public}d", ret);
        return DEFAULT_MAIN_DISPLAY_ID;
    }
    return id;
}

bool DisplayPowerMgrClient::SetBrightness(uint32_t value, uint32_t displayId, bool continuous)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    int32_t displayError = ERR_OK;
    auto ret = proxy->SetBrightness(value, displayId, continuous, result, displayError);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetBrightness, ret = %{public}d", ret);
        return false;
    }
    lastError_ = static_cast<DisplayErrors>(displayError);
    return result;
}

bool DisplayPowerMgrClient::DiscountBrightness(double discount, uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->DiscountBrightness(discount, displayId, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DiscountBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->OverrideBrightness(value, displayId, duration, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "OverrideBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::OverrideDisplayOffDelay(uint32_t delayMs)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->OverrideDisplayOffDelay(delayMs, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "OverrideDisplayOffDelay, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::RestoreBrightness(uint32_t displayId, uint32_t duration)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->RestoreBrightness(displayId, duration, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "RestoreBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

uint32_t DisplayPowerMgrClient::GetBrightness(uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_OFF);
    uint32_t brightness = BRIGHTNESS_OFF;
    auto ret = proxy->GetBrightness(displayId, brightness);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetBrightness, ret = %{public}d", ret);
        return BRIGHTNESS_OFF;
    }
    return brightness;
}

uint32_t DisplayPowerMgrClient::GetDefaultBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_DEFAULT);
    uint32_t brightness = BRIGHTNESS_DEFAULT_PROXY;
    auto ret = proxy->GetDefaultBrightness(brightness);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetDefaultBrightness, ret = %{public}d", ret);
        return BRIGHTNESS_DEFAULT_PROXY;
    }
    return brightness;
}

uint32_t DisplayPowerMgrClient::GetMaxBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_MAX);
    uint32_t brightness = BRIGHTNESS_DEFAULT_PROXY;
    auto ret = proxy->GetMaxBrightness(brightness);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetMaxBrightness, ret = %{public}d", ret);
        return BRIGHTNESS_DEFAULT_PROXY;
    }
    return brightness;
}

uint32_t DisplayPowerMgrClient::GetMinBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_MIN);
    uint32_t brightness = BRIGHTNESS_DEFAULT_PROXY;
    auto ret = proxy->GetMinBrightness(brightness);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetMinBrightness, ret = %{public}d", ret);
        return BRIGHTNESS_DEFAULT_PROXY;
    }
    return brightness;
}

bool DisplayPowerMgrClient::AdjustBrightness(uint32_t value, uint32_t duration, uint32_t id)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->AdjustBrightness(id, value, duration, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "AdjustBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::AutoAdjustBrightness(bool enable)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->AutoAdjustBrightness(enable, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "AutoAdjustBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::IsAutoAdjustBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->IsAutoAdjustBrightness(result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "IsAutoAdjustBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::SetScreenOnBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->SetScreenOnBrightness(result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetScreenOnBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
    if (callback == nullptr) {
        DISPLAY_HILOGE(COMP_FWK, "callback is nullptr");
        return false;
    }

    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->RegisterCallback(callback, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "RegisterCallback, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::BoostBrightness(int32_t timeoutMs, uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->BoostBrightness(timeoutMs, displayId, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "BoostBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

bool DisplayPowerMgrClient::CancelBoostBrightness(uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    auto ret = proxy->CancelBoostBrightness(displayId, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "CancelBoostBrightness, ret = %{public}d", ret);
        return false;
    }
    return result;
}

uint32_t DisplayPowerMgrClient::GetDeviceBrightness(uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, 0);
    uint32_t brightness = BRIGHTNESS_OFF;
    auto ret = proxy->GetDeviceBrightness(displayId, brightness);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "GetDeviceBrightness, ret = %{public}d", ret);
        return BRIGHTNESS_OFF;
    }
    return brightness;
}

void DisplayPowerMgrClient::WaitDimmingDone()
{
    auto proxy = GetProxy();
    RETURN_IF(proxy == nullptr);
    auto ret = proxy->WaitDimmingDone();
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "WaitDimmingDone, ret = %{public}d", ret);
    }
}

bool DisplayPowerMgrClient::SetCoordinated(bool coordinated, uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, 0);
    bool result = false;
    auto ret = proxy->SetCoordinated(coordinated, displayId, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetCoordinated, ret = %{public}d", ret);
        return false;
    }
    return result;
}

uint32_t DisplayPowerMgrClient::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, 0);
    uint32_t result = BRIGHTNESS_DEFAULT_PROXY;
    auto ret = proxy->SetLightBrightnessThreshold(threshold, callback, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetLightBrightnessThreshold, ret = %{public}d", ret);
        return BRIGHTNESS_DEFAULT_PROXY;
    }
    return result;
}

DisplayErrors DisplayPowerMgrClient::GetError()
{
    DisplayErrors tmpError = lastError_;
    if (lastError_ != DisplayErrors::ERR_OK) {
        lastError_ = DisplayErrors::ERR_OK;
    }
    return tmpError;
}

bool DisplayPowerMgrClient::SetMaxBrightness(double value, uint32_t enterTestMode)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    int32_t displayError = ERR_OK;
    auto ret = proxy->SetMaxBrightness(value, enterTestMode, result, displayError);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetMaxBrightness, ret = %{public}d", ret);
        return false;
    }
    lastError_ = static_cast<DisplayErrors>(displayError);
    return result;
}

bool DisplayPowerMgrClient::SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    bool result = false;
    int32_t displayError = ERR_OK;
    auto ret = proxy->SetMaxBrightnessNit(maxNit, enterTestMode, result, displayError);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SetMaxBrightnessNit, ret = %{public}d", ret);
        return false;
    }
    lastError_ = static_cast<DisplayErrors>(displayError);
    return result;
}

int DisplayPowerMgrClient::NotifyBrightnessManagerScreenPowerStatus(uint32_t displayId, uint32_t status)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, -1); // -1 means failed
    int32_t result = DEFAULT_VALUE;
    auto ret = proxy->NotifyScreenPowerStatus(displayId, status, result);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "NotifyBrightnessManagerScreenPowerStatus, ret = %{public}d", ret);
        return DEFAULT_VALUE;
    }
    lastError_ = static_cast<DisplayErrors>(result);
    return result;
}

}  // namespace DisplayPowerMgr
}  // namespace OHOS
