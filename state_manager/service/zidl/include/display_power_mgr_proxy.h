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

#ifndef DISPLAYMGR_DISPLAY_MGR_PROXY_H
#define DISPLAYMGR_DISPLAY_MGR_PROXY_H

#include <cstdint>
#include <vector>
#include <functional>
#include <iremote_proxy.h>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "refbase.h"
#include "display_power_info.h"
#include "idisplay_power_callback.h"
#include "idisplay_power_mgr.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrProxy : public IRemoteProxy<IDisplayPowerMgr> {
public:
    explicit DisplayPowerMgrProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDisplayPowerMgr>(impl) {}
    ~DisplayPowerMgrProxy() override = default;

    virtual bool SetDisplayState(uint32_t id, DisplayState state, uint32_t reason) override;
    virtual DisplayState GetDisplayState(uint32_t id) override;
    virtual std::vector<uint32_t> GetDisplayIds() override;
    virtual uint32_t GetMainDisplayId() override;

    virtual bool SetBrightness(uint32_t value, uint32_t displayId, bool continuous) override;
    virtual bool SetMaxBrightness(double value, uint32_t enterTestMode) override;
    virtual bool SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode) override;
    virtual bool DiscountBrightness(double value, uint32_t displayId) override;
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
    virtual DisplayErrors GetError() override;
    virtual int NotifyScreenPowerStatus(uint32_t displayId, uint32_t status) override;

private:
    static inline BrokerDelegator<DisplayPowerMgrProxy> delegator_;
    DisplayErrors lastError_ {DisplayErrors::ERR_OK};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_PROXY_H
