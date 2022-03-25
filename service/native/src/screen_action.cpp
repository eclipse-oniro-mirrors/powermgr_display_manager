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

#include "display_common.h"
#include "display_manager.h"
#include "display_type.h"
#include "display_log.h"
#include "screen_manager.h"
#include "window_manager_service_client.h"
#include "display_power_info.h"

namespace OHOS {
namespace DisplayPowerMgr {
ScreenAction::ScreenAction()
{
    DISPLAY_HILOGI(COMP_SVC, "Succeed to init");
}

uint64_t ScreenAction::GetDefaultDisplayId()
{
    DISPLAY_HILOGI(COMP_SVC, "GetDefaultDisplayId");
    return Rosen::DisplayManager::GetInstance().GetDefaultDisplayId();
}

std::vector<uint64_t> ScreenAction::GetDisplayIds()
{
    DISPLAY_HILOGI(COMP_SVC, "GetDisplayIds");
    devIds_ = Rosen::DisplayManager::GetInstance().GetAllDisplayIds();
    if (devIds_.empty()) {
        devIds_.push_back(0);
    }
    return devIds_;
}

DisplayState ScreenAction::GetPowerState(uint64_t devId)
{
    DISPLAY_HILOGI(COMP_SVC, "GetPowerState: %{public}d", static_cast<int>(devId));

    DisplayState ret = DisplayState::DISPLAY_UNKNOWN;
    Rosen::ScreenPowerState state = Rosen::ScreenManager::GetInstance()
        .GetScreenPower(devId);
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

bool ScreenAction::SetDisplayState(uint64_t devId, DisplayState state,
    std::function<void(DisplayState)> callback)
{
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayState: devId=%{public}d, state=%{public}d",
        static_cast<int>(devId), static_cast<uint32_t>(state));

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

bool ScreenAction::SetDisplayPower(uint64_t devId, DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(COMP_SVC, "SetDisplayPower: devId=%{public}d, state=%{public}d, state=%{public}d",
        static_cast<int>(devId), static_cast<uint32_t>(state), reason);
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

uint32_t ScreenAction::GetBrightness(uint64_t devId)
{
    DISPLAY_HILOGI(COMP_SVC, "GetBrightness: %{public}d", static_cast<int>(devId));
    return Rosen::DisplayManager::GetInstance()
        .GetScreenBrightness(static_cast<uint64_t>(devId));
}

bool ScreenAction::SetBrightness(uint64_t devId, uint32_t value)
{
    DISPLAY_HILOGI(COMP_SVC, "SetBrightness: %{public}d, %{public}d",
        static_cast<int>(devId), value);
    return Rosen::DisplayManager::GetInstance()
        .SetScreenBrightness(static_cast<uint64_t>(devId), value);
}
} // namespace DisplayPowerMgr
} // namespace OHOS
