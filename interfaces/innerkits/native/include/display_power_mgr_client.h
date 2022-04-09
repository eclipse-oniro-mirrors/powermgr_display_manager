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

#ifndef DISPLAYMGR_DISPLAY_MGR_CLIENT_H
#define DISPLAYMGR_DISPLAY_MGR_CLIENT_H

#include <iremote_object.h>
#include <singleton.h>
#include <vector>

#include "display_power_info.h"
#include "idisplay_power_callback.h"
#include "idisplay_power_mgr.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrClient : public DelayedRefSingleton<DisplayPowerMgrClient> {
    DECLARE_DELAYED_REF_SINGLETON(DisplayPowerMgrClient);

public:
    bool SetDisplayState(DisplayState state,
        PowerMgr::StateChangeReason reason = PowerMgr::StateChangeReason::STATE_CHANGE_REASON_UNKNOWN,
        uint32_t id = 0);
    DisplayState GetDisplayState(uint32_t id = 0);
    std::vector<uint32_t> GetDisplayIds();
    int32_t GetMainDisplayId();
    bool SetBrightness(uint32_t value, uint32_t displayId = 0);
    bool AdjustBrightness(uint32_t value, uint32_t duration, uint32_t id = 0);
    bool SetStateConfig(DisplayState state, uint32_t value, uint32_t id = 0);
    bool AutoAdjustBrightness(bool enable);
    bool RegisterCallback(sptr<IDisplayPowerCallback> callback);

private:
    class DisplayDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit DisplayDeathRecipient(DisplayPowerMgrClient& client) : client_(client) {}
        ~DisplayDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        DisplayPowerMgrClient& client_;
    };

    sptr<IDisplayPowerMgr> GetProxy();
    void OnRemoteDied(const wptr<IRemoteObject>& remote);

    std::mutex mutex_;
    sptr<IDisplayPowerMgr> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_GRADUAL_ANIMATOR_H
