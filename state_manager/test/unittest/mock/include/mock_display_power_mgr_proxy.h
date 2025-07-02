/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_DISPLAY_POWER_MGR_PROXY_H
#define MOCK_DISPLAY_POWER_MGR_PROXY_H

#include "display_power_mgr_proxy.h"

namespace OHOS {
namespace DisplayPowerMgr {
class MockDisplayPowerMgrProxy : public DisplayPowerMgrProxy {
public:
    explicit MockDisplayPowerMgrProxy(const sptr<IRemoteObject>& remote)
        : DisplayPowerMgrProxy(remote)
    {}
    ~MockDisplayPowerMgrProxy() {}
    int32_t SetDisplayState(uint32_t id, uint32_t state, uint32_t reason, bool& bResult) override;
    int32_t GetDisplayState(uint32_t id, int32_t& displayState) override;
    int32_t GetDisplayIds(std::vector<uint32_t>& ids) override;
    int32_t GetMainDisplayId(uint32_t& id) override;
    int32_t SetBrightness(uint32_t value, uint32_t displayId, bool continuous, bool& bResult,
        int32_t& retCode) override;
    int32_t SetMaxBrightness(double value, uint32_t enterTestMode, bool& bResult, int32_t& retCode) override;
    int32_t SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode, bool& bResult, int32_t& retCode) override;
    int32_t DiscountBrightness(double value, uint32_t displayId, bool& bResult) override;
    int32_t OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration, bool& bResult) override;
    int32_t OverrideDisplayOffDelay(uint32_t delayMs, bool& bResult) override;
    int32_t RestoreBrightness(uint32_t displayId, uint32_t duration, bool& bResult) override;
    int32_t GetBrightness(uint32_t displayId, uint32_t& brightness) override;
    int32_t GetDefaultBrightness(uint32_t& defaultBrightness) override;
    int32_t GetMaxBrightness(uint32_t& maxBrightness) override;
    int32_t GetMinBrightness(uint32_t& minBrightness) override;
    int32_t AdjustBrightness(uint32_t id, int32_t value, uint32_t duration, bool& bResult) override;
    int32_t AutoAdjustBrightness(bool enable, bool& bResult) override;
    int32_t IsAutoAdjustBrightness(bool& bResult) override;
    int32_t BoostBrightness(int32_t timeoutMs, uint32_t displayId, bool& bResult) override;
    int32_t CancelBoostBrightness(uint32_t displayId, bool& bResult) override;
    int32_t GetDeviceBrightness(uint32_t displayId, uint32_t& deviceBrightness) override;
    int32_t SetCoordinated(bool coordinated, uint32_t displayId, bool& bResult) override;
    int32_t SetLightBrightnessThreshold(const std::vector<int32_t>& threshold,
        const sptr<IDisplayBrightnessCallback>& displayBrightnessCallback, uint32_t& retCode) override;
    int32_t RegisterCallback(const sptr<IDisplayPowerCallback>& displayPowercallback, bool& bResult) override;
    int32_t SetScreenOnBrightness(bool& bResult) override;
    int32_t NotifyScreenPowerStatus(uint32_t displayId, uint32_t displayPowerStatus, int32_t& retCode) override;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // MOCK_DISPLAY_POWER_MGR_PROXY_H
