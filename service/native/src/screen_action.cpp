/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "screen_action.h"

#include "display_manager.h"
#include "display_log.h"
#include "screen_manager.h"
#include "display_power_info.h"

namespace OHOS {
namespace DisplayPowerMgr {
ScreenAction::ScreenAction()
{
    DISPLAY_HILOGI(COMP_SVC, "Succeed to init");
}

uint32_t ScreenAction::GetDefaultDisplayId()
{
    uint64_t defaultId = Rosen::DisplayManager::GetInstance().GetDefaultDisplayId();
    return static_cast<uint32_t>(defaultId);
}

std::vector<uint32_t> ScreenAction::GetDisplayIds()
{
    std::vector<uint64_t> allIds = Rosen::DisplayManager::GetInstance().GetAllDisplayIds();
    if (allIds.empty()) {
        displayIds_.push_back(DEFAULT_DISPLAY_ID);
        return displayIds_;
    }
    for (const auto& id: allIds) {
        displayIds_.push_back(static_cast<uint32_t>(id));
    }
    return displayIds_;
}

DisplayState ScreenAction::GetPowerState(uint32_t displayId)
{
    DISPLAY_HILOGI(COMP_SVC, "GetPowerState of displayId=%{public}u", displayId);
    DisplayState ret = DisplayState::DISPLAY_UNKNOWN;
    Rosen::ScreenPowerState state = Rosen::ScreenManager::GetInstance().GetScreenPower(displayId);
    DISPLAY_HILOGI(COMP_SVC, "GetPowerState: %{public}d", static_cast<uint32_t>(state));
    switch (state) {
        case Rosen::ScreenPowerState::POWER_ON:
            ret = DisplayState::DISPLAY_ON;
            break;
        case Rosen::ScreenPowerState::POWER_STAND_BY:
            ret = DisplayState::DISPLAY_DIM;
            break;
        case Rosen::ScreenPowerState::POWER_SUSPEND:
            ret = DisplayState::DISPLAY_SUSPEND;
            break;
        case Rosen::ScreenPowerState::POWER_OFF:
            ret = DisplayState::DISPLAY_OFF;
            break;
        default:
            break;
    }

    return ret;
}

bool ScreenAction::SetDisplayState(uint32_t displayId, DisplayState state,
                                   const std::function<void(DisplayState)>& callback)
{
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayState: displayId=%{public}u, state=%{public}u",
                   displayId, static_cast<uint32_t>(state));

    Rosen::DisplayState rds = Rosen::DisplayState::UNKNOWN;
    switch (state) {
        case DisplayState::DISPLAY_ON:
            rds = Rosen::DisplayState::ON;
            break;
        case DisplayState::DISPLAY_OFF:
            rds = Rosen::DisplayState::OFF;
            break;
        default:
            break;
    }
    bool ret = Rosen::DisplayManager::GetInstance().SetDisplayState(rds,
        [callback](Rosen::DisplayState rosenState) {
            DISPLAY_HILOGI(COMP_SVC, "SetDisplayState Callback:%{public}d",
                static_cast<uint32_t>(rosenState));
            DisplayState state = DisplayState::DISPLAY_UNKNOWN;
            switch (rosenState) {
                case Rosen::DisplayState::ON:
                    state = DisplayState::DISPLAY_ON;
                    break;
                case Rosen::DisplayState::OFF:
                    state = DisplayState::DISPLAY_OFF;
                    break;
                default:
                    return;
            }
            callback(state);
    });
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayState:%{public}d", ret);
    return ret;
}

bool ScreenAction::SetDisplayPower(uint32_t displayId, DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayPower: displayId=%{public}u, state=%{public}u, state=%{public}u",
                   displayId, static_cast<uint32_t>(state), reason);
    Rosen::ScreenPowerState status = Rosen::ScreenPowerState::INVALID_STATE;
    switch (state) {
        case DisplayState::DISPLAY_ON:
            status = Rosen::ScreenPowerState::POWER_ON;
            break;
        case DisplayState::DISPLAY_DIM:
            status = Rosen::ScreenPowerState::POWER_STAND_BY;
            break;
        case DisplayState::DISPLAY_SUSPEND:
            status = Rosen::ScreenPowerState::POWER_SUSPEND;
            break;
        case DisplayState::DISPLAY_OFF:
            status = Rosen::ScreenPowerState::POWER_OFF;
            break;
        default:
            break;
    }
    bool ret = Rosen::ScreenManager::GetInstance().SetScreenPowerForAll(status,
        Rosen::PowerStateChangeReason::POWER_BUTTON);
    DISPLAY_HILOGI(COMP_SVC, "SetScreenPowerForAll:%{public}d", ret);
    return true;
}

uint32_t ScreenAction::GetBrightness(uint32_t displayId)
{
    auto brightness = Rosen::DisplayManager::GetInstance().GetScreenBrightness(displayId);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "displayId=%{public}u, brightness=%{public}u", displayId, brightness);
    return brightness;
}

bool ScreenAction::SetBrightness(uint32_t displayId, uint32_t value)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "displayId=%{public}u, brightness=%{public}u", displayId, value);
    return Rosen::DisplayManager::GetInstance().SetScreenBrightness(displayId, value);
}
} // namespace DisplayPowerMgr
} // namespace OHOS
