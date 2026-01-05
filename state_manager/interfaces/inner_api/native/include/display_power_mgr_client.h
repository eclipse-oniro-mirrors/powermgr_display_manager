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

#ifndef DISPLAYMGR_DISPLAY_MGR_CLIENT_H
#define DISPLAYMGR_DISPLAY_MGR_CLIENT_H

#include <iremote_object.h>
#include <singleton.h>
#include <vector>

#include "display_power_info.h"
#include "idisplay_power_callback.h"
#include "idisplay_power_mgr.h"
#include "power_state_machine_info.h"
#include "display_mgr_errors.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrClient : public DelayedRefSingleton<DisplayPowerMgrClient> {
    DECLARE_DELAYED_REF_SINGLETON(DisplayPowerMgrClient);

public:
    // SetScreenDisplayState: Set the display status of the spercified screen. The caller must ensure the accurary of
    // the screenId passed.
    bool SetScreenDisplayState(uint64_t screenId, DisplayState status, uint32_t reason);
    // SetDisplayState: For PowerMgrService only, updating screen status.
    bool SetDisplayState(DisplayState state,
        PowerMgr::StateChangeReason reason = PowerMgr::StateChangeReason::STATE_CHANGE_REASON_UNKNOWN,
        uint32_t id = 0);
    DisplayState GetDisplayState(uint32_t id = 0);
    std::vector<uint32_t> GetDisplayIds();
    int32_t GetMainDisplayId();
    bool SetBrightness(uint32_t value, uint32_t displayId = 0, bool continuous = false);
    bool SetMaxBrightness(double value, uint32_t enterTestMode = 0);
    bool SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode = 0);
    bool DiscountBrightness(double discount, uint32_t displayId = 0);
    bool OverrideBrightness(uint32_t value, uint32_t displayId = 0, uint32_t duration = 500);
    bool OverrideDisplayOffDelay(uint32_t delayMs);
    bool RestoreBrightness(uint32_t displayId = 0, uint32_t duration = 500);
    uint32_t GetBrightness(uint32_t displayId = 0);
    uint32_t GetDefaultBrightness();
    uint32_t GetMaxBrightness();
    uint32_t GetMinBrightness();
    bool AdjustBrightness(uint32_t value, uint32_t duration, uint32_t id = 0);
    bool AutoAdjustBrightness(bool enable);
    bool IsAutoAdjustBrightness();
    bool SetScreenOnBrightness();
    bool RegisterCallback(sptr<IDisplayPowerCallback> callback);
    bool BoostBrightness(int32_t timeoutMs, uint32_t displayId = 0);
    bool CancelBoostBrightness(uint32_t displayId = 0);
    uint32_t GetDeviceBrightness(uint32_t displayId = 0, bool useHbm = false);
    void WaitDimmingDone();
    bool SetCoordinated(bool coordinated, uint32_t displayId = 0);
    std::string RunJsonCommand(const std::string& request);
    // Registers a brightness data change listener. Returns 0 on success.
    // Both callerId (for server-side deduplication) and params (a JSON string) are optional.
    int32_t RegisterDataChangeListener(const sptr<IDisplayBrightnessListener>& listener,
        DisplayDataChangeListenerType listenerType, const std::string& callerId = "", const std::string& params = "");
    int32_t UnregisterDataChangeListener(
        DisplayDataChangeListenerType listenerType, const std::string& callerId = "");
    uint32_t SetLightBrightnessThreshold(std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback);
    DisplayErrors GetError();
    int NotifyBrightnessManagerScreenPowerStatus(uint32_t displayId, uint32_t status);
    DisplayErrors SetScreenPowerOffStrategy(PowerOffStrategy strategy, PowerMgr::StateChangeReason reason =
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);

#ifndef DISPLAY_SERVICE_DEATH_UT
private:
#endif
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

    static constexpr int32_t INVALID_DISPLAY_ID {-1};
    static constexpr int32_t DEFAULT_MAIN_DISPLAY_ID {0};
    static constexpr uint32_t BRIGHTNESS_OFF {0};
    static constexpr uint32_t BRIGHTNESS_DEFAULT {102};
    static constexpr uint32_t BRIGHTNESS_MAX {255};
    static constexpr uint32_t BRIGHTNESS_MIN {1};

    DisplayErrors lastError_ {DisplayErrors::ERR_OK};
    std::mutex mutex_;
    sptr<IDisplayPowerMgr> proxy_ {nullptr};
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ {nullptr};
#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
    sptr<IRemoteObject> token_ {nullptr};
#endif
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_GRADUAL_ANIMATOR_H
