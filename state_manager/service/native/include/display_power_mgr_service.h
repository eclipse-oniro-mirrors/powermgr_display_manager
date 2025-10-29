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

#ifndef DISPLAYMGR_DISPLAY_MGR_SERVICE_H
#define DISPLAYMGR_DISPLAY_MGR_SERVICE_H

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <sys/types.h>
#include "delayed_sp_singleton.h"
#include "refbase.h"
#include "iremote_object.h"
#ifdef ENABLE_SENSOR_PART
#include "sensor_agent_type.h"
#endif
#include "idisplay_power_callback.h"
#include "display_power_info.h"
#include "display_common.h"
#include "display_power_mgr_stub.h"
#include "display_xcollie.h"
#include "screen_controller.h"
#include "brightness_manager.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrService : public DisplayPowerMgrStub {
public:
    virtual ~DisplayPowerMgrService() = default;

    int32_t SetDisplayState(uint32_t id, uint32_t state, uint32_t reason, bool& result) override;
    int32_t GetDisplayState(uint32_t id, int32_t& displayState) override;
    int32_t GetDisplayIds(std::vector<uint32_t>& ids) override;
    int32_t GetMainDisplayId(uint32_t& id) override;
    int32_t SetBrightness(uint32_t value, uint32_t displayId, bool continuous, bool& result,
        int32_t& displayError) override;
    int32_t SetMaxBrightness(double value, uint32_t mode, bool& result, int32_t& displayError) override;
    int32_t SetMaxBrightnessNit(uint32_t maxNit, uint32_t mode, bool& result, int32_t& displayError) override;
    int32_t DiscountBrightness(double discount, uint32_t displayId, bool& result) override;
    int32_t OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration, bool& result) override;
    int32_t OverrideDisplayOffDelay(uint32_t delayMs, bool& result) override;

    int32_t RestoreBrightness(uint32_t displayId, uint32_t duration, bool& result) override;

    int32_t GetBrightness(uint32_t displayId, uint32_t& brightness) override;
    int32_t GetDefaultBrightness(uint32_t& defaultBrightness) override;
    int32_t GetMaxBrightness(uint32_t& maxBrightness) override;
    int32_t GetMinBrightness(uint32_t& minBrightness) override;
    int32_t AdjustBrightness(uint32_t id, int32_t value, uint32_t duration, bool& result) override;
    int32_t AutoAdjustBrightness(bool enable, bool& result) override;
    int32_t IsAutoAdjustBrightness(bool& result) override;
    int32_t RegisterCallback(const sptr<IDisplayPowerCallback>& callback, bool& result) override;
    int32_t BoostBrightness(int32_t timeoutMs, uint32_t displayId, bool& result) override;
    int32_t CancelBoostBrightness(uint32_t displayId, bool& result) override;
    int32_t GetDeviceBrightness(uint32_t displayId, uint32_t& deviceBrightness) override;
    int32_t SetCoordinated(bool coordinated, uint32_t displayId, bool& result) override;
    int32_t SetLightBrightnessThreshold(const std::vector<int32_t>& threshold,
        const sptr<IDisplayBrightnessCallback>& callback, uint32_t& result) override;
    int32_t SetScreenOnBrightness(bool& result) override;
    int32_t NotifyScreenPowerStatus(uint32_t displayId, uint32_t displayPowerStatus, int32_t& result) override;
private:
    bool SetDisplayStateInner(uint32_t id, DisplayState state, uint32_t reason);
    void UndoSetDisplayStateInner(uint32_t id, DisplayState curState, uint32_t reason);
    DisplayState GetDisplayStateInner(uint32_t id);
    std::vector<uint32_t> GetDisplayIdsInner();
    uint32_t GetMainDisplayIdInner();
    bool SetBrightnessInner(uint32_t value, uint32_t displayId, bool continuous = false);
    bool SetMaxBrightnessInner(double value, uint32_t mode);
    bool SetMaxBrightnessNitInner(uint32_t maxNit, uint32_t mode);
    bool DiscountBrightnessInner(double discount, uint32_t displayId);
    bool OverrideBrightnessInner(uint32_t value, uint32_t displayId, uint32_t duration = 500);
    bool OverrideDisplayOffDelayInner(uint32_t delayMs);
    bool RestoreBrightnessInner(uint32_t displayId, uint32_t duration = 500);
    uint32_t GetBrightnessInner(uint32_t displayId);
    uint32_t GetDefaultBrightnessInner();
    uint32_t GetMaxBrightnessInner();
    uint32_t GetMinBrightnessInner();
    bool AdjustBrightnessInner(uint32_t id, int32_t value, uint32_t duration);
    bool AutoAdjustBrightnessInner(bool enable);
    bool IsAutoAdjustBrightnessInner();
    bool SetScreenOnBrightnessInner();
    bool RegisterCallbackInner(sptr<IDisplayPowerCallback> callback);
    bool BoostBrightnessInner(int32_t timeoutMs, uint32_t displayId);
    bool CancelBoostBrightnessInner(uint32_t displayId);
    uint32_t GetDeviceBrightnessInner(uint32_t displayId);
    bool SetCoordinatedInner(bool coordinated, uint32_t displayId);
    uint32_t SetLightBrightnessThresholdInner(
        std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback);
    int NotifyScreenPowerStatusInner(uint32_t displayId, uint32_t displayPowerStatus);
public:
    DisplayErrors GetError();
    virtual int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void NotifyStateChangeCallback(uint32_t displayId, DisplayState state, uint32_t reason);
    void Init();
    void Deinit();
    void Reset();
    void ClearOffset();
    void HandleBootBrightness();
    static uint32_t GetSafeBrightness(uint32_t value);
    static double GetSafeDiscount(double discount, uint32_t brightness);

private:
    class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CallbackDeathRecipient() = default;
        virtual ~CallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
    private:
        std::mutex callbackMutex_;
    };

    static const uint32_t BRIGHTNESS_OFF = 0;
    static const uint32_t BRIGHTNESS_MIN;
    static const uint32_t BRIGHTNESS_DEFAULT;
    static const uint32_t BRIGHTNESS_MAX;
    static const uint32_t DELAY_TIME_UNSET = 0;
    static constexpr const double DISCOUNT_MIN = 0.01;
    static constexpr const double DISCOUNT_MAX = 1.00;

    friend DelayedSpSingleton<DisplayPowerMgrService>;

    DisplayPowerMgrService();
    void DumpDisplayInfo(std::string& result);
    static void RegisterBootCompletedCallback();
    static void SetBootCompletedBrightness();
    static void SetBootCompletedAutoBrightness();
    static void RegisterSettingObservers();
    static void UnregisterSettingObservers();
    static void RegisterSettingAutoBrightnessObserver();
    static void UnregisterSettingAutoBrightnessObserver();
    static void AutoBrightnessSettingUpdateFunc(const std::string& key);
    static bool GetSettingAutoBrightness(const std::string& key = SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
    void ScreenOffDelay(uint32_t id, DisplayState state, uint32_t reason);
    bool IsSupportLightSensor(void);

    static constexpr const char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};
    std::map<uint64_t, std::shared_ptr<ScreenController>> controllerMap_;
    sptr<IDisplayPowerCallback> callback_;
    sptr<CallbackDeathRecipient> cbDeathRecipient_;

    DisplayErrors lastError_ {DisplayErrors::ERR_OK};
    std::mutex mutex_;
    static std::atomic_bool isBootCompleted_;
    uint32_t displayOffDelayMs_ {0};
    bool isDisplayDelayOff_ = false;
    uint32_t displayId_ {0};
    DisplayState displayState_ {DisplayState::DISPLAY_UNKNOWN};
    uint32_t displayReason_ {0};
    std::shared_ptr<PowerMgr::FFRTQueue> queue_;
    bool isInTestMode_ {false};
    std::once_flag initFlag_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_SERVICE_H
