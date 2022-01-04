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

#include "display_power_mgr_client.h"

#include <if_system_ability_manager.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "display_common.h"

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
        DISPLAY_HILOGE(MODULE_INNERKIT, "Failed to get system ability manager");
        return nullptr;
    }
    sptr<IRemoteObject> obj = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    if (obj == nullptr) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Failed to get display manager service");
        return nullptr;
    }
    sptr<IRemoteObject::DeathRecipient> dr = new DisplayDeathRecipient(*this);
    if ((obj->IsProxyObject()) && (!obj->AddDeathRecipient(dr))) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Failed to add death recipient");
        return nullptr;
    }

    proxy_ = iface_cast<IDisplayPowerMgr>(obj);
    deathRecipient_ = dr;
    DISPLAY_HILOGI(MODULE_INNERKIT, "Succeed to connect display manager service");
    return proxy_;
}

void DisplayPowerMgrClient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    if (remote == nullptr) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "OnRemoteDied failed, remote is nullptr");
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

bool DisplayPowerMgrClient::SetDisplayState(DisplayState state, uint32_t id)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return false;
    }
    return proxy->SetDisplayState(id, state);
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
        return std::vector<uint32_t>();
    }
    return proxy->GetDisplayIds();
}

int32_t DisplayPowerMgrClient::GetMainDisplayId()
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return -1;
    }
    return proxy->GetMainDisplayId();
}

bool DisplayPowerMgrClient::SetBrightness(uint32_t value, uint32_t id)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return false;
    }
    return proxy->SetBrightness(id, value);
}

bool DisplayPowerMgrClient::AdjustBrightness(uint32_t value, uint32_t duration, uint32_t id)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return false;
    }
    return proxy->AdjustBrightness(id, value, duration);
}

bool DisplayPowerMgrClient::AutoAdjustBrightness(bool enable)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return false;
    }
    return proxy->AutoAdjustBrightness(enable);
}

bool DisplayPowerMgrClient::SetStateConfig(DisplayState state, uint32_t value, uint32_t id)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return false;
    }
    return proxy->SetStateConfig(id, state, value);
}

bool DisplayPowerMgrClient::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
    auto proxy = GetProxy();
    if (proxy == nullptr) {
        return false;
    }
    return proxy->RegisterCallback(callback);
}
}  // namespace DisplayPowerMgr
}  // namespace OHOS
