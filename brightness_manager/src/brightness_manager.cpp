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

#include "brightness_manager.h"

namespace OHOS {
namespace DisplayPowerMgr {
BrightnessManager& BrightnessManager::Get()
{
    static BrightnessManager brightnessManager;
    return brightnessManager;
}

void BrightnessManager::Init(uint32_t defaultMax, uint32_t defaultMin)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    mBrightnessManagerExt.Init(defaultMax, defaultMin);
#else
    BrightnessService::Get().Init(defaultMax, defaultMin);
#endif
}

void BrightnessManager::DeInit()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    mBrightnessManagerExt.DeInit();
#else
    BrightnessService::Get().DeInit();
#endif
}

void BrightnessManager::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    mBrightnessManagerExt.SetDisplayState(id, state, reason);
#else
    BrightnessService::Get().SetDisplayState(id, state);
#endif
}

DisplayState BrightnessManager::GetState()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.GetState();
#else
    return BrightnessService::Get().GetDisplayState();
#endif
}

bool BrightnessManager::AutoAdjustBrightness(bool enable)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.AutoAdjustBrightness(enable);
#else
    #ifdef ENABLE_SENSOR_PART
        return BrightnessService::Get().AutoAdjustBrightness(enable);
    #else
        return false;
    #endif
#endif
}

bool BrightnessManager::SetBrightness(uint32_t value, uint32_t gradualDuration, bool continuous)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.SetBrightness(value, gradualDuration, continuous);
#else
    return BrightnessService::Get().SetBrightness(value, gradualDuration, continuous);
#endif
}

bool BrightnessManager::DiscountBrightness(double discount)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.DiscountBrightness(discount);
#else
    return BrightnessService::Get().DiscountBrightness(discount);
#endif
}

double BrightnessManager::GetDiscount() const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.GetDiscount();
#else
    return BrightnessService::Get().GetDiscount();
#endif
}

void BrightnessManager::SetScreenOnBrightness()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    mBrightnessManagerExt.SetScreenOnBrightness();
#else
    BrightnessService::Get().SetScreenOnBrightness();
#endif
}

uint32_t BrightnessManager::GetScreenOnBrightness() const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.GetScreenOnBrightness();
#else
    return BrightnessService::Get().GetScreenOnBrightness(false);
#endif
}

void BrightnessManager::ClearOffset()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    mBrightnessManagerExt.ClearOffset();
#else
    BrightnessService::Get().ClearOffset();
#endif
}

bool BrightnessManager::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.OverrideBrightness(value, gradualDuration);
#else
    return BrightnessService::Get().OverrideBrightness(value, gradualDuration);
#endif
}

bool BrightnessManager::RestoreBrightness(uint32_t gradualDuration)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.RestoreBrightness(gradualDuration);
#else
    return BrightnessService::Get().RestoreBrightness(gradualDuration);
#endif
}

bool BrightnessManager::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.BoostBrightness(timeoutMs, gradualDuration);
#else
    return BrightnessService::Get().BoostBrightness(timeoutMs, gradualDuration);
#endif
}

bool BrightnessManager::CancelBoostBrightness(uint32_t gradualDuration)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.CancelBoostBrightness(gradualDuration);
#else
    return BrightnessService::Get().CancelBoostBrightness(gradualDuration);
#endif
}

uint32_t BrightnessManager::GetBrightness()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.GetBrightness();
#else
    return BrightnessService::Get().GetBrightness();
#endif
}

uint32_t BrightnessManager::GetDeviceBrightness()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.GetDeviceBrightness();
#else
    return BrightnessService::Get().GetDeviceBrightness();
#endif
}

uint32_t BrightnessManager::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.SetLightBrightnessThreshold(threshold, callback);
#else
    DISPLAY_HILOGW(FEAT_BRIGHTNESS, "BrightnessManager::SetLightBrightnessThreshold not support");
    return BrightnessService::Get().SetLightBrightnessThreshold(threshold, callback);
#endif
}

bool BrightnessManager::IsBrightnessOverridden() const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.IsBrightnessOverridden();
#else
    return BrightnessService::Get().IsBrightnessOverridden();
#endif
}

bool BrightnessManager::IsBrightnessBoosted() const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.IsBrightnessBoosted();
#else
    return BrightnessService::Get().IsBrightnessBoosted();
#endif
}

uint32_t BrightnessManager::GetCurrentDisplayId(uint32_t defaultId) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.GetCurrentDisplayId(defaultId);
#else
    return BrightnessService::Get().GetCurrentDisplayId(defaultId);
#endif
}

void BrightnessManager::SetDisplayId(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    mBrightnessManagerExt.SetDisplayId(id);
#else
    BrightnessService::Get().SetDisplayId(id);
#endif
}

bool BrightnessManager::SetMaxBrightness(double value)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.SetMaxBrightness(value);
#else
    return BrightnessService::Get().SetMaxBrightness(value);
#endif
}

bool BrightnessManager::SetMaxBrightnessNit(uint32_t nit)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.SetMaxBrightnessNit(nit);
#else
    return BrightnessService::Get().SetMaxBrightnessNit(nit);
#endif
}

int BrightnessManager::NotifyScreenPowerStatus(uint32_t displayId, uint32_t status)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS,
        "BrightnessManager::NotifyScreenPowerStatus displayId:%{public}u, status:%{public}u", displayId, status);
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
    return mBrightnessManagerExt.NotifyScreenPowerStatus(displayId, status);
#else
    return BrightnessService::Get().NotifyScreenPowerStatus(displayId, status);
#endif
}
} // namespace DisplayPowerMgr
} // namespace OHOS
