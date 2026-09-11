/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#ifndef BRIGHTNESS_MANAGER_H
#define BRIGHTNESS_MANAGER_H

#include "brightness_service.h"
#include "brightness_manager_ext.h"

namespace OHOS {
namespace DisplayPowerMgr {
class BrightnessManager {
public:
    BrightnessManager(const BrightnessManager&) = delete;
    BrightnessManager& operator=(const BrightnessManager&) = delete;
    BrightnessManager(BrightnessManager&&) = delete;
    BrightnessManager& operator=(BrightnessManager&&) = delete;

    static BrightnessManager& Get();

    void Init(uint32_t defaultMax, uint32_t defaultMin);
    void DeInit();
    bool GetFeatureSupport(BrightnessFeatureType feature, uint32_t screenId = 0);
    void SetDisplayState(uint32_t id, DisplayState state, uint32_t reason);
    DisplayState GetState(uint32_t screenId = 0);
    bool IsSupportLightSensor(uint32_t screenId = 0);
    bool IsAutoAdjustBrightness(uint32_t screenId = 0);
    bool AutoAdjustBrightness(bool enable, uint32_t screenId = 0);
    bool SetForcedBrightness(double value, uint32_t duration, BrightnessValueType valueType, uint32_t screenId = 0);
    bool SetBrightness(uint32_t value, uint32_t gradualDuration = 0, bool continuous = false, uint32_t screenId = 0);
    bool DiscountBrightness(double discount, uint32_t screenId = 0);
    double GetDiscount(uint32_t screenId = 0) const;
    void SetScreenOnBrightness(uint32_t screenId = 0);
    uint32_t GetScreenOnBrightness(uint32_t screenId = 0) const;
    bool OverrideBrightness(uint32_t value, uint32_t gradualDuration = 0, uint32_t screenId = 0);
    bool RestoreBrightness(uint32_t gradualDuration = 0, uint32_t screenId = 0);
    bool BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration = 0, uint32_t screenId = 0);
    bool CancelBoostBrightness(uint32_t gradualDuration = 0, uint32_t screenId = 0);
    bool IsBrightnessOverridden(uint32_t screenId = 0) const;
    bool IsBrightnessBoosted(uint32_t screenId = 0) const;
    uint32_t GetBrightness(uint32_t screenId = 0);
    uint32_t GetDeviceBrightness(bool useHbm = false, uint32_t screenId = 0);
    void WaitDimmingDone(uint32_t screenId = 0) const;
    void ClearOffset(uint32_t screenId = 0);
    std::string RunJsonCommand(const std::string& request, uint32_t screenId = 0);
    int32_t RegisterDataChangeListener(const sptr<IDisplayBrightnessListener>& listener,
        DisplayDataChangeListenerType listenerType, const std::string& callerId,
        const std::string& params, uint32_t screenId = 0);
    int32_t UnregisterDataChangeListener(
        DisplayDataChangeListenerType listenerType, const std::string& callerId, uint32_t screenId = 0);
    uint32_t SetLightBrightnessThreshold(std::vector<int32_t> threshold,
        sptr<IDisplayBrightnessCallback> callback, uint32_t screenId = 0);
    uint32_t GetCurrentDisplayId(uint32_t defaultId) const;
    void SetDisplayId(uint32_t id = 0);
    bool SetMaxBrightness(double value, uint32_t screenId = 0);
    bool SetMaxBrightnessNit(uint32_t nit, uint32_t screenId = 0);
    int NotifyScreenPowerStatus(uint32_t displayId, uint32_t status);
    bool SetSceneMode(SceneModeType type, bool enable, uint32_t id = 0);

private:
    BrightnessManager() = default;
    virtual ~BrightnessManager() = default;

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    std::shared_ptr<BrightnessManagerExt> GetExt(uint32_t screenId = 0) const;
    void CreateExt(const uint32_t screenId);
    void RegisterScreenListener();
    void UnRegisterScreenListener();

    class MultiScreenListener : public Rosen::ScreenManagerLite::IScreenListener {
    public:
        virtual void OnConnect(uint64_t screenId) override;
        virtual void OnDisconnect(uint64_t screenId) override {}
        virtual void OnChange(uint64_t screenId) override {}
    };
#endif
private:
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    std::map<uint32_t, std::shared_ptr<BrightnessManagerExt>> mMultiBrtMgrExt;
    uint32_t defaultMax_;
    uint32_t defaultMin_;
    mutable std::shared_mutex mMutex;
    sptr<MultiScreenListener> multiScreenListener_{nullptr};
#else
    BrightnessManagerExt mBrightnessManagerExt;
#endif
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // BRIGHTNESS_MANAGER_H