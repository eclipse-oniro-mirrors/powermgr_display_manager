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

void BrightnessManager::Init()
{
    BrightnessService::Get().Init();
}

void BrightnessManager::DeInit()
{
    BrightnessService::Get().DeInit();
}

void BrightnessManager::SetDisplayState(uint32_t id, DisplayState state)
{
    BrightnessService::Get().SetDisplayState(id, state);
}

DisplayState BrightnessManager::GetState()
{
    return BrightnessService::Get().GetDisplayState();
}

bool BrightnessManager::AutoAdjustBrightness(bool enable)
{
#ifdef ENABLE_SENSOR_PART
    return BrightnessService::Get().AutoAdjustBrightness(enable);
#else
    return false;
#endif
}

bool BrightnessManager::SetBrightness(uint32_t value, uint32_t gradualDuration, bool continuous)
{
    return BrightnessService::Get().SetBrightness(value, gradualDuration, continuous);
}

bool BrightnessManager::DiscountBrightness(double discount)
{
    return BrightnessService::Get().DiscountBrightness(discount);
}

double BrightnessManager::GetDiscount() const
{
    return BrightnessService::Get().GetDiscount();
}

void BrightnessManager::SetScreenOnBrightness()
{
    BrightnessService::Get().SetScreenOnBrightness();
}

uint32_t BrightnessManager::GetScreenOnBrightness() const
{
    return BrightnessService::Get().GetScreenOnBrightness(false);
}

void BrightnessManager::ClearOffset()
{
    BrightnessService::Get().ClearOffset();
}

bool BrightnessManager::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    return BrightnessService::Get().OverrideBrightness(value, gradualDuration);
}

bool BrightnessManager::RestoreBrightness(uint32_t gradualDuration)
{
    return BrightnessService::Get().RestoreBrightness(gradualDuration);
}

bool BrightnessManager::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
    return BrightnessService::Get().BoostBrightness(timeoutMs, gradualDuration);
}

bool BrightnessManager::CancelBoostBrightness(uint32_t gradualDuration)
{
    return BrightnessService::Get().CancelBoostBrightness(gradualDuration);
}

uint32_t BrightnessManager::GetBrightness()
{
    return BrightnessService::Get().GetBrightness();
}

uint32_t BrightnessManager::GetDeviceBrightness()
{
    return BrightnessService::Get().GetDeviceBrightness();
}

bool BrightnessManager::IsBrightnessOverridden() const
{
    return BrightnessService::Get().IsBrightnessOverridden();
}

bool BrightnessManager::IsBrightnessBoosted() const
{
    return BrightnessService::Get().IsBrightnessBoosted();
}

uint32_t BrightnessManager::GetCurrentDisplayId(uint32_t defaultId) const
{
    return BrightnessService::Get().GetCurrentDisplayId(defaultId);
}

void BrightnessManager::SetDisplayId(uint32_t id)
{
    return BrightnessService::Get().SetDisplayId(id);
}
} // namespace DisplayPowerMgr
} // namespace OHOS
