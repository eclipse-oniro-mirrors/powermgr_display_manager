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

#ifndef DISPLAYMGR_DISPLAY_MGR_SERVICE_H
#define DISPLAYMGR_DISPLAY_MGR_SERVICE_H

#include <mutex>
#include <system_ability.h>
#include <system_ability_definition.h>

#include "delayed_sp_singleton.h"
#include "display_common.h"
#include "display_power_mgr_stub.h"
#include "screen_controller.h"
#include "sensor_agent.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrService : public DisplayPowerMgrStub {
public:
    virtual ~DisplayPowerMgrService();
    virtual bool SetDisplayState(uint32_t id, DisplayState state, uint32_t reason) override;
    virtual DisplayState GetDisplayState(uint32_t id) override;
    virtual std::vector<uint32_t> GetDisplayIds() override;
    virtual uint32_t GetMainDisplayId() override;
    virtual bool SetBrightness(uint32_t value, uint32_t displayId) override;
    virtual bool OverrideBrightness(uint32_t value, uint32_t displayId) override;
    virtual bool RestoreBrightness(uint32_t displayId) override;
    virtual uint32_t GetBrightness(uint32_t displayId) override;
    virtual bool AdjustBrightness(uint32_t id, int32_t value, uint32_t duration) override;
    virtual bool AutoAdjustBrightness(bool enable) override;
    virtual bool SetStateConfig(uint32_t id, DisplayState state, int32_t value) override;
    virtual bool RegisterCallback(sptr<IDisplayPowerCallback> callback) override;
    virtual int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    void NotifyStateChangeCallback(uint32_t displayId, DisplayState state);

private:
    class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        CallbackDeathRecipient() = default;
        virtual ~CallbackDeathRecipient() = default;
        virtual void OnRemoteDied(const wptr<IRemoteObject>& remote);
    };
    static const uint32_t AUTO_ADJUST_BRIGHTNESS_DURATION = 1000;
    static const uint32_t SAMPLING_RATE = 100000000;
    static const int32_t BRIGHTNESS_CHANGE_MIN = 5;
    static const int32_t LUX_TO_NIT_SQRT_RADIO = 5;
    static const time_t LUX_STABLE_TIME = 1;
    static constexpr float LUX_CHANGE_RATE_THRESHOLD = 10;
    static constexpr float LUX_CHANGE_STABLE_MIN = 100.0;
    static const int32_t NIT_MIN = 2;
    static const int32_t NIT_MAX = 450;
    static const uint32_t BRIGHTNESS_OFF = 0;
    static const uint32_t BRIGHTNESS_MIN = 1;
    static const uint32_t BRIGHTNESS_MAX = 255;
    static void AmbientLightCallback(SensorEvent *event);

    friend DelayedSpSingleton<DisplayPowerMgrService>;

    DisplayPowerMgrService();
    void InitSensors();
    bool IsChangedLux(float scalar);
    static uint32_t GetSafeBrightness(uint32_t value);
    bool CalculateBrightness(float scalar, int32_t& brightness);
    int32_t GetBrightnessFromLightScalar(float scalar);
    void ActivateAmbientSensor();
    void DeactivateAmbientSensor();

    std::shared_ptr<ScreenAction> action_;

    std::map<uint64_t, std::shared_ptr<ScreenController>> controllerMap_;
    bool supportLightSensor_ {false};
    bool autoBrightness_ {false};
    bool ambientSensorEnabled_ {false};
    SensorUser user_;
    sptr<IDisplayPowerCallback> callback_;
    sptr<CallbackDeathRecipient> cbDeathRecipient_;

    time_t lastLuxTime_ {0};
    float lastLux_ {0};
    bool luxChanged_ {false};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_SERVICE_H
