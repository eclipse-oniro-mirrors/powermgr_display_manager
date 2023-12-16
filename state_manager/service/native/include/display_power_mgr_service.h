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
#include "event_runner.h"
#include "iremote_object.h"
#ifdef HAS_SENSORS_SENSOR_PART
#include "sensor_agent_type.h"
#endif
#include "idisplay_power_callback.h"
#include "display_power_info.h"
#include "display_common.h"
#include "display_power_mgr_stub.h"
#include "screen_controller.h"
#include "brightness_manager.h"

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
    virtual bool DiscountBrightness(double discount, uint32_t displayId) override;
    virtual bool OverrideBrightness(uint32_t value, uint32_t displayId) override;
    virtual bool OverrideDisplayOffDelay(uint32_t delayMs) override;
    virtual bool RestoreBrightness(uint32_t displayId) override;
    virtual uint32_t GetBrightness(uint32_t displayId) override;
    virtual uint32_t GetDefaultBrightness() override;
    virtual uint32_t GetMaxBrightness() override;
    virtual uint32_t GetMinBrightness() override;
    virtual bool AdjustBrightness(uint32_t id, int32_t value, uint32_t duration) override;
    virtual bool AutoAdjustBrightness(bool enable) override;
    virtual bool IsAutoAdjustBrightness() override;
    virtual bool RegisterCallback(sptr<IDisplayPowerCallback> callback) override;
    virtual bool BoostBrightness(int32_t timeoutMs, uint32_t displayId) override;
    virtual bool CancelBoostBrightness(uint32_t displayId) override;
    virtual uint32_t GetDeviceBrightness(uint32_t displayId) override;
    virtual bool SetCoordinated(bool coordinated, uint32_t displayId) override;
    virtual int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    virtual DisplayErrors GetError() override;
    void NotifyStateChangeCallback(uint32_t displayId, DisplayState state, uint32_t reason);
    void Init();
    void Deinit();
    void ClearOffset();
    void SetScreenOnBrightness();
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

#ifdef HAS_SENSORS_SENSOR_PART
    static void AmbientLightCallback(SensorEvent* event);
    void InitSensors();
    void ActivateAmbientSensor();
    void DeactivateAmbientSensor();
    bool supportLightSensor_ {false};
    bool ambientSensorEnabled_ {false};
    SensorUser sensorUser_ {};
#endif

    static const uint32_t AUTO_ADJUST_BRIGHTNESS_STRIDE = 1;
    static const uint32_t SAMPLING_RATE = 100000000;
    static const int32_t BRIGHTNESS_CHANGE_MIN = 2;
    static const int32_t LUX_TO_NIT_SQRT_RADIO = 5;
    static const time_t LUX_STABLE_TIME = 1;
    static constexpr float LUX_CHANGE_RATE_THRESHOLD = 2;
    static constexpr float LUX_CHANGE_STABLE_MIN = 10.0;
    static const int32_t NIT_MIN = 2;
    static const int32_t NIT_MAX = 450;
    static const uint32_t BRIGHTNESS_OFF = 0;
    static const uint32_t BRIGHTNESS_MIN;
    static const uint32_t BRIGHTNESS_DEFAULT;
    static const uint32_t BRIGHTNESS_MAX;
    static const uint32_t DELAY_TIME_UNSET = 0;
    static constexpr const double DISCOUNT_MIN = 0.01;
    static constexpr const double DISCOUNT_MAX = 1.00;

    friend DelayedSpSingleton<DisplayPowerMgrService>;

    DisplayPowerMgrService();
    bool IsChangedLux(float scalar);
    bool CalculateBrightness(float scalar, int32_t& brightness, int32_t& change);
    int32_t GetBrightnessFromLightScalar(float scalar);
    static void RegisterBootCompletedCallback();
    static void SetBootCompletedBrightness();
    static void SetBootCompletedAutoBrightness();
    static void RegisterSettingObservers();
    static void UnregisterSettingObservers();
    static void RegisterSettingAutoBrightnessObserver();
    static void UnregisterSettingAutoBrightnessObserver();
    static void AutoBrightnessSettingUpdateFunc(const std::string& key);
    static void SetSettingAutoBrightness(bool enable);
    static bool GetSettingAutoBrightness(const std::string& key = SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
    void ScreenOffDelay(uint32_t id, DisplayState state, uint32_t reason);

    static constexpr const char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};
    std::map<uint64_t, std::shared_ptr<ScreenController>> controllerMap_;
    bool autoBrightness_ {false};
    sptr<IDisplayPowerCallback> callback_;
    sptr<CallbackDeathRecipient> cbDeathRecipient_;

    DisplayErrors lastError_ {DisplayErrors::ERR_OK};
    time_t lastLuxTime_ {0};
    float lastLux_ {0};
    bool luxChanged_ {false};
    std::mutex mutex_;
    static std::atomic_bool isBootCompleted_;
    uint32_t displayOffDelayMs_ {0};
    bool isDisplayDelayOff_ = false;
    bool setDisplayStateRet_ = true;
    uint32_t displayId_ {0};
    DisplayState displayState_ {DisplayState::DISPLAY_UNKNOWN};
    DisplayState tempState_ {DisplayState::DISPLAY_UNKNOWN};
    uint32_t displayReason_ {0};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_SERVICE_H
