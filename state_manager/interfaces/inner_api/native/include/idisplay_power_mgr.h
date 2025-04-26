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

#ifndef DISPLAYMGR_IDISPLAY_MGR_H
#define DISPLAYMGR_IDISPLAY_MGR_H

#include <iremote_broker.h>
#include <vector>

#include "display_mgr_errors.h"
#include "display_power_info.h"
#include "idisplay_brightness_callback.h"
#include "idisplay_power_callback.h"

namespace OHOS {
namespace DisplayPowerMgr {
class IDisplayPowerMgr : public IRemoteBroker {
public:
    virtual bool SetDisplayState(uint32_t id, DisplayState state, uint32_t reason) = 0;
    virtual DisplayState GetDisplayState(uint32_t id) = 0;
    virtual std::vector<uint32_t> GetDisplayIds() = 0;
    virtual uint32_t GetMainDisplayId() = 0;
    virtual bool SetBrightness(uint32_t value, uint32_t displayId, bool continuous) = 0;
    virtual bool SetMaxBrightness(double value, uint32_t enterTestMode) = 0;
    virtual bool SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode) = 0;
    virtual bool DiscountBrightness(double discount, uint32_t displayId) = 0;
    virtual bool OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration = 500) = 0;
    virtual bool OverrideDisplayOffDelay(uint32_t delayMs) = 0;
    virtual bool RestoreBrightness(uint32_t displayId, uint32_t duration = 500) = 0;
    virtual uint32_t GetBrightness(uint32_t displayId) = 0;
    virtual uint32_t GetDefaultBrightness() = 0;
    virtual uint32_t GetMaxBrightness() = 0;
    virtual uint32_t GetMinBrightness() = 0;
    virtual bool AdjustBrightness(uint32_t id, int32_t value, uint32_t duration) = 0;
    virtual bool AutoAdjustBrightness(bool enable) = 0;
    virtual bool IsAutoAdjustBrightness() = 0;
    virtual bool SetScreenOnBrightness() = 0;
    virtual bool RegisterCallback(sptr<IDisplayPowerCallback> callback) = 0;
    virtual bool BoostBrightness(int32_t timeoutMs, uint32_t displayId) = 0;
    virtual bool CancelBoostBrightness(uint32_t displayId) = 0;
    virtual uint32_t GetDeviceBrightness(uint32_t displayId) = 0;
    virtual bool SetCoordinated(bool coordinated, uint32_t displayId) = 0;
    virtual uint32_t SetLightBrightnessThreshold(
        std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback) = 0;
    virtual DisplayErrors GetError() = 0;
    virtual int NotifyScreenPowerStatus(uint32_t displayId, uint32_t status) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.displaypowermgr.IDisplayPowerMgr");
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_IDISPLAY_MGR_H
