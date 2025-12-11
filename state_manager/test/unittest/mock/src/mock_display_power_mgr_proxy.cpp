/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mock_display_power_mgr_proxy.h"
namespace {
constexpr int32_t ERR_FAIL = -1;
}
namespace OHOS {
namespace DisplayPowerMgr {
int32_t MockDisplayPowerMgrProxy::SetDisplayState(uint32_t id, uint32_t state, uint32_t reason, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetDisplayState(uint32_t id, int32_t& displayState)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetDisplayIds(std::vector<uint32_t>& ids)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetMainDisplayId(uint32_t& id)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetBrightness(uint32_t value, uint32_t displayId, bool continuous, bool& isResult,
    int32_t& retCode)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetMaxBrightness(double value, uint32_t enterTestMode, bool& isResult,
    int32_t& retCode)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode, bool& isResult,
    int32_t& retCode)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::DiscountBrightness(double value, uint32_t displayId, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration,
    bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::OverrideDisplayOffDelay(uint32_t delayMs, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::RestoreBrightness(uint32_t displayId, uint32_t duration, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetBrightness(uint32_t displayId, uint32_t& brightness)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetDefaultBrightness(uint32_t& defaultBrightness)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetMaxBrightness(uint32_t& maxBrightness)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetMinBrightness(uint32_t& minBrightness)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::AutoAdjustBrightness(bool enable, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::IsAutoAdjustBrightness(bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::BoostBrightness(int32_t timeoutMs, uint32_t displayId, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::CancelBoostBrightness(uint32_t displayId, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::GetDeviceBrightness(uint32_t displayId, bool useHbm, uint32_t& deviceBrightness)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetCoordinated(bool coordinated, uint32_t displayId, bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetLightBrightnessThreshold(const std::vector<int32_t>& threshold,
    const sptr<IDisplayBrightnessCallback>& displayBrightnessCallback, uint32_t& retCode)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::RegisterCallback(const sptr<IDisplayPowerCallback>& displayPowercallback,
    bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetScreenOnBrightness(bool& isResult)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::NotifyScreenPowerStatus(uint32_t displayId, uint32_t displayPowerStatus,
    int32_t& retCode)
{
    return ERR_FAIL;
}

int32_t MockDisplayPowerMgrProxy::SetScreenDisplayState(uint64_t screenId, uint32_t status, uint32_t reason)
{
    if (status == static_cast<uint32_t>(DisplayState::DISPLAY_ON) ||
        status == static_cast<uint32_t>(DisplayState::DISPLAY_OFF)) {
        return ERR_OK;
    }
    return ERR_FAIL;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
