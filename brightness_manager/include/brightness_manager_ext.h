/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef BRIGHTNESS_MANAGER_EXT_H
#define BRIGHTNESS_MANAGER_EXT_H

#include "brightness_service.h"

namespace OHOS {
namespace DisplayPowerMgr {
class BrightnessManagerExt {
public:
    BrightnessManagerExt();
    ~BrightnessManagerExt();

    BrightnessManagerExt(const BrightnessManagerExt&) = delete;
    BrightnessManagerExt& operator=(const BrightnessManagerExt&) = delete;
    BrightnessManagerExt(BrightnessManagerExt&&) = delete;
    BrightnessManagerExt& operator=(BrightnessManagerExt&&) = delete;

    void Init(uint32_t defaultMax, uint32_t defaultMin);
    void DeInit();
    void SetDisplayState(uint32_t id, DisplayState state, uint32_t reason);
    DisplayState GetState();
    bool IsSupportLightSensor(void);
    bool IsAutoAdjustBrightness(void);
    bool AutoAdjustBrightness(bool enable);
    bool SetBrightness(uint32_t value, uint32_t gradualDuration = 0, bool continuous = false);
    bool DiscountBrightness(double discount);
    double GetDiscount() const;
    void SetScreenOnBrightness();
    uint32_t GetScreenOnBrightness() const;
    bool OverrideBrightness(uint32_t value, uint32_t gradualDuration = 0);
    bool RestoreBrightness(uint32_t gradualDuration = 0);
    bool BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration = 0);
    bool CancelBoostBrightness(uint32_t gradualDuration = 0);
    bool IsBrightnessOverridden() const;
    bool IsBrightnessBoosted() const;
    uint32_t GetBrightness();
    uint32_t GetDeviceBrightness();
    void WaitDimmingDone() const;
    void ClearOffset();
    uint32_t GetCurrentDisplayId(uint32_t defaultId) const;
    void SetDisplayId(uint32_t id = 0);
    std::string RunJsonCommand(const std::string& request);
    int32_t RegisterDataChangeListener(const sptr<IDisplayBrightnessListener>& listener,
        DisplayDataChangeListenerType listenerType, const std::string& callerId, const std::string& params);
    int32_t UnregisterDataChangeListener(
        DisplayDataChangeListenerType listenerType, const std::string& callerId);
    uint32_t SetLightBrightnessThreshold(std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback);
    bool SetMaxBrightness(double value);
    bool SetMaxBrightnessNit(uint32_t maxNit);
    int NotifyScreenPowerStatus(uint32_t displayId, uint32_t status);

private:
    bool LoadBrightnessExtLibrary();
    void CloseBrightnessExtLibrary();
    bool LoadBrightnessOps();
    bool LoadBrightnessOpsEx1();
    bool LoadBrightnessControl();
    bool LoadBrightnessStatus();

    bool mBrightnessManagerExtEnable = false;
    void* mBrightnessManagerExtHandle = nullptr;
    void* mBrightnessManagerInitFunc = nullptr;
    void* mBrightnessManagerDeInitFunc = nullptr;
    void* mSetDisplayStateFunc = nullptr;
    void* mGetDisplayStateFunc = nullptr;
    void* mIsSupportLightSensorFunc = nullptr;
    void* mIsAutoAdjustBrightnessFunc = nullptr;
    void* mAutoAdjustBrightnessFunc = nullptr;
    void* mSetBrightnessFunc = nullptr;
    void* mDiscountBrightnessFunc = nullptr;
    void* mGetDiscountFunc = nullptr;
    void* mSetScreenOnBrightnessFunc = nullptr;
    void* mGetScreenOnBrightnessFunc = nullptr;
    void* mOverrideBrightnessFunc = nullptr;
    void* mRestoreBrightnessFunc = nullptr;
    void* mBoostBrightnessFunc = nullptr;
    void* mCancelBoostBrightnessFunc = nullptr;
    void* mIsBrightnessOverriddenFunc = nullptr;
    void* mIsBrightnessBoostedFunc = nullptr;
    void* mGetBrightnessFunc = nullptr;
    void* mGetDeviceBrightnessFunc = nullptr;
    void* mWaitDimmingDoneFunc = nullptr;
    void* mClearOffsetFunc = nullptr;
    void* mGetCurrentDisplayIdFunc = nullptr;
    void* mSetDisplayIdFunc = nullptr;
    void* mSetLightBrightnessThresholdFunc = nullptr;
    void* mSetMaxBrightnessFunc = nullptr;
    void* mSetMaxBrightnessNitFunc = nullptr;
    void* mNotifyScreenPowerStatusFunc = nullptr;
    void* mRunJsonCommandFunc = nullptr;
    void* mRegisterDataChangeListenerFunc = nullptr;
    void* mUnregisterDataChangeListenerFunc = nullptr;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // BRIGHTNESS_MANAGER_EXT_H