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
#include "screen_controller.h"
#include "brightness_manager.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrService : public DisplayPowerMgrStub {
public:
    virtual ~DisplayPowerMgrService() = default;
    virtual bool SetDisplayState(uint32_t id, DisplayState state, uint32_t reason) override;
    virtual DisplayState GetDisplayState(uint32_t id) override;
    virtual std::vector<uint32_t> GetDisplayIds() override;
    virtual uint32_t GetMainDisplayId() override;
    virtual bool SetBrightness(uint32_t value, uint32_t displayId, bool continuous = false) override;
    virtual bool SetMaxBrightness(double value, uint32_t mode) override;
    virtual bool SetMaxBrightnessNit(uint32_t maxNit, uint32_t mode) override;
    virtual bool DiscountBrightness(double discount, uint32_t displayId) override;
    virtual bool OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration = 500) override;
    virtual bool OverrideDisplayOffDelay(uint32_t delayMs) override;
    virtual bool RestoreBrightness(uint32_t displayId, uint32_t duration = 500) override;
    virtual uint32_t GetBrightness(uint32_t displayId) override;
    virtual uint32_t GetDefaultBrightness() override;
    virtual uint32_t GetMaxBrightness() override;
    virtual uint32_t GetMinBrightness() override;
    virtual bool AdjustBrightness(uint32_t id, int32_t value, uint32_t duration) override;
    virtual bool AutoAdjustBrightness(bool enable) override;
    virtual bool IsAutoAdjustBrightness() override;
    virtual bool SetScreenOnBrightness() override;
    virtual bool RegisterCallback(sptr<IDisplayPowerCallback> callback) override;
    virtual bool BoostBrightness(int32_t timeoutMs, uint32_t displayId) override;
    virtual bool CancelBoostBrightness(uint32_t displayId) override;
    virtual uint32_t GetDeviceBrightness(uint32_t displayId) override;
    virtual bool SetCoordinated(bool coordinated, uint32_t displayId) override;
    virtual uint32_t SetLightBrightnessThreshold(
        std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback) override;
    virtual int NotifyScreenPowerStatus(uint32_t displayId, uint32_t displayPowerStatus) override;
    virtual int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    virtual DisplayErrors GetError() override;
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
