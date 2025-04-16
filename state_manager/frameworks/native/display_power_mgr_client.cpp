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
    return proxy->SetDisplayState(id, state, static_cast<uint32_t>(reason));
}

DisplayState DisplayPowerMgrClient::GetDisplayState(uint32_t id)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    return proxy->GetDisplayState(id);
}

std::vector<uint32_t> DisplayPowerMgrClient::GetDisplayIds()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return {};
    }
    return proxy->GetDisplayIds();
}

int32_t DisplayPowerMgrClient::GetMainDisplayId()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, INVALID_DISPLAY_ID);
    return static_cast<int32_t>(proxy->GetMainDisplayId());
}

bool DisplayPowerMgrClient::SetBrightness(uint32_t value, uint32_t displayId, bool continuous)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->SetBrightness(value, displayId, continuous);
}

bool DisplayPowerMgrClient::DiscountBrightness(double discount, uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->DiscountBrightness(discount, displayId);
}

bool DisplayPowerMgrClient::OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->OverrideBrightness(value, displayId, duration);
}

bool DisplayPowerMgrClient::OverrideDisplayOffDelay(uint32_t delayMs)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->OverrideDisplayOffDelay(delayMs);
}

bool DisplayPowerMgrClient::RestoreBrightness(uint32_t displayId, uint32_t duration)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->RestoreBrightness(displayId, duration);
}

uint32_t DisplayPowerMgrClient::GetBrightness(uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_OFF);
    return proxy->GetBrightness(displayId);
}

uint32_t DisplayPowerMgrClient::GetDefaultBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_DEFAULT);
    return proxy->GetDefaultBrightness();
}

uint32_t DisplayPowerMgrClient::GetMaxBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_MAX);
    return proxy->GetMaxBrightness();
}

uint32_t DisplayPowerMgrClient::GetMinBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, BRIGHTNESS_MIN);
    return proxy->GetMinBrightness();
}

bool DisplayPowerMgrClient::AdjustBrightness(uint32_t value, uint32_t duration, uint32_t id)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->AdjustBrightness(id, value, duration);
}

bool DisplayPowerMgrClient::AutoAdjustBrightness(bool enable)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->AutoAdjustBrightness(enable);
}

bool DisplayPowerMgrClient::IsAutoAdjustBrightness()
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->IsAutoAdjustBrightness();
}

bool DisplayPowerMgrClient::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
    if (callback == nullptr) {
        DISPLAY_HILOGE(COMP_FWK, "callback is nullptr");
        return false;
    }

    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->RegisterCallback(callback);
}

bool DisplayPowerMgrClient::BoostBrightness(int32_t timeoutMs, uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->BoostBrightness(timeoutMs, displayId);
}

bool DisplayPowerMgrClient::CancelBoostBrightness(uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->CancelBoostBrightness(displayId);
}

uint32_t DisplayPowerMgrClient::GetDeviceBrightness(uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, 0);
    return proxy->GetDeviceBrightness(displayId);
}

bool DisplayPowerMgrClient::SetCoordinated(bool coordinated, uint32_t displayId)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, 0);
    return proxy->SetCoordinated(coordinated, displayId);
}

uint32_t DisplayPowerMgrClient::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, 0);
    return proxy->SetLightBrightnessThreshold(threshold, callback);
}

DisplayErrors DisplayPowerMgrClient::GetError()
{
    if (lastError_ != DisplayErrors::ERR_OK) {
        DisplayErrors tmpError = lastError_;
        lastError_ = DisplayErrors::ERR_OK;
        return tmpError;
    }
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, DisplayErrors::ERR_CONNECTION_FAIL);
    return proxy->GetError();
}

bool DisplayPowerMgrClient::SetMaxBrightness(double value, uint32_t enterTestMode)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->SetMaxBrightness(value, enterTestMode);
}

bool DisplayPowerMgrClient::SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, false);
    return proxy->SetMaxBrightnessNit(maxNit, enterTestMode);
}

int DisplayPowerMgrClient::NotifyBrightnessManagerScreenPowerStatus(uint32_t displayId, uint32_t status)
{
    auto proxy = GetProxy();
    RETURN_IF_WITH_RET(proxy == nullptr, -1); // -1 means failed
    int ret = proxy->NotifyScreenPowerStatus(displayId, status);
    return ret;
}

}  // namespace DisplayPowerMgr
}  // namespace OHOS
