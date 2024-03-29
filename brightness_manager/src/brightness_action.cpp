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

#include "brightness_action.h"

#include <hisysevent.h>
#include <ipc_skeleton.h>

#include "display_log.h"
#include "display_manager.h"
#include "dm_common.h"
#include "screen_manager.h"

namespace OHOS {
namespace DisplayPowerMgr {
BrightnessAction::BrightnessAction(uint32_t displayId) : mDisplayId(displayId)
{}

uint32_t BrightnessAction::GetDisplayId()
{
    return mDisplayId;
}

uint32_t BrightnessAction::GetCurrentDisplayId(uint32_t defaultId)
{
    uint32_t currentId = defaultId;
    bool isFoldable = Rosen::DisplayManager::GetInstance().IsFoldable();
    if (!isFoldable) {
        DISPLAY_HILOGI(FEAT_STATE, "GetCurrentDisplayId not fold phone return default id=%{public}d", defaultId);
        return currentId;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto foldMode = Rosen::DisplayManager::GetInstance().GetFoldDisplayMode();
    if (Rosen::FoldDisplayMode::MAIN == foldMode) {
        currentId = OUTTER_SCREEN_DISPLAY_ID;
    }
    DISPLAY_HILOGI(FEAT_STATE, "GetCurrentDisplayId foldMode=%{public}u", foldMode);
    IPCSkeleton::SetCallingIdentity(identity);
    return static_cast<uint32_t>(currentId);
}

void BrightnessAction::SetDisplayId(uint32_t displayId)
{
    mDisplayId = displayId;
}

DisplayState BrightnessAction::GetDisplayState()
{
    DisplayState state = DisplayState::DISPLAY_UNKNOWN;
    Rosen::ScreenPowerState powerState = Rosen::ScreenManager::GetInstance().GetScreenPower(mDisplayId);
    DISPLAY_HILOGI(FEAT_STATE, "ScreenPowerState=%{public}d", static_cast<uint32_t>(powerState));
    switch (powerState) {
        case Rosen::ScreenPowerState::POWER_ON:
            state = DisplayState::DISPLAY_ON;
            break;
        case Rosen::ScreenPowerState::POWER_STAND_BY:
            state = DisplayState::DISPLAY_DIM;
            break;
        case Rosen::ScreenPowerState::POWER_SUSPEND:
            state = DisplayState::DISPLAY_SUSPEND;
            break;
        case Rosen::ScreenPowerState::POWER_OFF:
            state = DisplayState::DISPLAY_OFF;
            break;
        default:
            break;
    }
    DISPLAY_HILOGI(FEAT_STATE, "state=%{public}u displayId=%{public}u", static_cast<uint32_t>(state), mDisplayId);
    return state;
}

uint32_t BrightnessAction::GetBrightness()
{
    std::lock_guard lock(mMutexBrightness);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    mBrightness = Rosen::DisplayManager::GetInstance().GetScreenBrightness(mDisplayId);
    IPCSkeleton::SetCallingIdentity(identity);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "displayId=%{public}u, brightness=%{public}u", mDisplayId, mBrightness);
    return mBrightness;
}

bool BrightnessAction::SetBrightness(uint32_t value)
{
    return SetBrightness(mDisplayId, value);
}

bool BrightnessAction::SetBrightness(uint32_t displayId, uint32_t value)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBrightness mDisplayId=%{public}u, displayId=%{public}u ,brightness=%{public}u",
        mDisplayId, displayId, value);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "displayId=%{public}u, brightness=%{public}u", displayId, value);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool isSucc = Rosen::DisplayManager::GetInstance().SetScreenBrightness(displayId, value);
    IPCSkeleton::SetCallingIdentity(identity);
    std::lock_guard lock(mMutexBrightness);
    mBrightness = isSucc ? value : mBrightness;
    return isSucc;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
