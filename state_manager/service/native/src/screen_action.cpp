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

#include "screen_action.h"
#include <datetime_ex.h>
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <ipc_skeleton.h>

#include "display_log.h"
#include "screen_manager_lite.h"
#include "ffrt.h"
#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
#include "screen_power_off_strategy.h"
#endif


namespace OHOS {
namespace DisplayPowerMgr {
ScreenAction::ScreenAction(uint32_t displayId) : displayId_(displayId)
{}

uint32_t ScreenAction::GetDefaultDisplayId()
{
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    uint64_t defaultId = Rosen::DisplayManagerLite::GetInstance().GetDefaultDisplayId();
    IPCSkeleton::SetCallingIdentity(identity);
    return static_cast<uint32_t>(defaultId);
}

std::vector<uint32_t> ScreenAction::GetAllDisplayId()
{
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    std::vector<uint64_t> allIds = Rosen::DisplayManagerLite::GetInstance().GetAllDisplayIds();
    IPCSkeleton::SetCallingIdentity(identity);
    std::vector<uint32_t> displayIds;
    std::transform(allIds.begin(), allIds.end(), back_inserter(displayIds), [](uint64_t id) {
        return static_cast<uint32_t>(id);
    });
    return displayIds;
}

uint32_t ScreenAction::GetDisplayId()
{
    return displayId_;
}

DisplayState ScreenAction::GetDisplayState()
{
    DisplayState state = DisplayState::DISPLAY_UNKNOWN;
    Rosen::ScreenPowerState powerState = Rosen::ScreenManagerLite::GetInstance().GetScreenPower(displayId_);
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
    DISPLAY_HILOGI(FEAT_STATE, "state=%{public}u displayId=%{public}u", static_cast<uint32_t>(state), displayId_);
    return state;
}

bool ScreenAction::EnableSkipSetDisplayState(uint32_t reason)
{
    if (reason == static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS)) {
        return true;
    }
    return false;
}

Rosen::DisplayState ScreenAction::ParseDisplayState(DisplayState state)
{
    Rosen::DisplayState ret = Rosen::DisplayState::UNKNOWN;
    switch (state) {
        case DisplayState::DISPLAY_ON:
            ret = Rosen::DisplayState::ON;
            break;
        case DisplayState::DISPLAY_OFF:
            ret = Rosen::DisplayState::OFF;
            break;
        case DisplayState::DISPLAY_DOZE:
            ret = Rosen::DisplayState::DOZE;
            break;
        case DisplayState::DISPLAY_DOZE_SUSPEND:
            ret = Rosen::DisplayState::DOZE_SUSPEND;
            break;
        default:
            ret = Rosen::DisplayState::UNKNOWN;
            break;
    }
    return ret;
}

void ScreenAction::WriteHiSysEvent(DisplayState state, int32_t beginTimeMs)
{
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    constexpr int32_t DMS_WAIT_LOCKSCREENON_TIMEOUT = 300;
    constexpr int32_t DMS_WAIT_LOCKSCREENOFF_TIMEOUT = 2000;
    constexpr int32_t DMS_WAIT_LOCKSCREENON_TYPE = 0;
    constexpr int32_t DMS_WAIT_LOCKSCREENOFF_TYPE = 1;
    int32_t endTimeMs = GetTickCount();
    if ((endTimeMs - beginTimeMs > DMS_WAIT_LOCKSCREENON_TIMEOUT) && state == DisplayState::DISPLAY_ON) {
        DISPLAY_HILOGI(FEAT_STATE, "dms wait lockscreenon timeout=%{public}d", (endTimeMs - beginTimeMs));
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "DMS_WAIT_LOCKSCREEN_TIMEOUT",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "TYPE", DMS_WAIT_LOCKSCREENON_TYPE,
            "TIME", (endTimeMs - beginTimeMs));
    } else if ((endTimeMs - beginTimeMs > DMS_WAIT_LOCKSCREENOFF_TIMEOUT) && state == DisplayState::DISPLAY_OFF) {
        DISPLAY_HILOGI(FEAT_STATE, "dms wait lockscreenoff timeout=%{public}d", (endTimeMs - beginTimeMs));
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "DMS_WAIT_LOCKSCREEN_TIMEOUT",
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR, "TYPE", DMS_WAIT_LOCKSCREENOFF_TYPE,
            "TIME", (endTimeMs - beginTimeMs));
    }
#endif
}

bool ScreenAction::SetDisplayState(DisplayState state, const std::function<void(DisplayState)>& callback)
{
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    int32_t beginTimeMs = GetTickCount();
#endif
    uint32_t ffrtId = ffrt::this_task::get_id();
    DISPLAY_HILOGI(FEAT_STATE, "[UL_POWER] SetDisplayState displayId=%{public}u, state=%{public}u, ffrtId=%{public}u",
        displayId_, static_cast<uint32_t>(state), ffrtId);
    Rosen::DisplayState rds = ParseDisplayState(state);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool ret = Rosen::DisplayManagerLite::GetInstance().SetDisplayState(rds,
        [callback](Rosen::DisplayState rosenState) {
        DISPLAY_HILOGI(FEAT_STATE, "[UL_POWER] SetDisplayState Callback:%{public}d", static_cast<uint32_t>(rosenState));
        DisplayState state;
        switch (rosenState) {
            case Rosen::DisplayState::ON:
                state = DisplayState::DISPLAY_ON;
                break;
            case Rosen::DisplayState::OFF:
                state = DisplayState::DISPLAY_OFF;
                break;
            case Rosen::DisplayState::ON_SUSPEND:
                state = DisplayState::DISPLAY_SUSPEND;
                break;
            case Rosen::DisplayState::DOZE:
                state = DisplayState::DISPLAY_DOZE;
                break;
            case Rosen::DisplayState::DOZE_SUSPEND:
                state = DisplayState::DISPLAY_DOZE_SUSPEND;
                break;
            default:
                return;
        }
        callback(state);
    });
    WriteHiSysEvent(state, beginTimeMs);
    IPCSkeleton::SetCallingIdentity(identity);
    // Notify screen state change event to battery statistics
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "SCREEN_STATE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "STATE", static_cast<int32_t>(state));
#endif
    ffrtId = ffrt::this_task::get_id();
    DISPLAY_HILOGI(FEAT_STATE,
        "[UL_POWER] SetDisplayState: displayId=%{public}u, state=%{public}u, ret=%{public}d, ffrtId=%{public}u",
        displayId_, static_cast<uint32_t>(state), ret, ffrtId);
    return ret;
}

Rosen::PowerStateChangeReason ScreenAction::ParseSpecialReason(uint32_t reason)
{
    auto changeReason = Rosen::PowerStateChangeReason::POWER_BUTTON;
    switch (reason) {
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT;
            break;
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON;
            break;
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS;
            break;
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF;
            break;
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_SWITCH):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_SWITCH;
            break;
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_POWER_KEY):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_POWER_KEY;
            break;
#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
        case static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPCAST):
            changeReason = Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_APPCAST;
            break;
#endif
        default:
            break;
    }
    return changeReason;
}

Rosen::ScreenPowerState ScreenAction::ParseScreenPowerState(DisplayState state)
{
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
        case DisplayState::DISPLAY_DOZE:
            status = Rosen::ScreenPowerState::POWER_DOZE;
            break;
        case DisplayState::DISPLAY_DOZE_SUSPEND:
            status = Rosen::ScreenPowerState::POWER_DOZE_SUSPEND;
            break;
        default:
            break;
    }
    return status;
}

bool ScreenAction::SetDisplayPower(DisplayState state, uint32_t reason)
{
    uint32_t ffrtId = ffrt::this_task::get_id();
    DISPLAY_HILOGI(FEAT_STATE,
        "[UL_POWER] SetDisplayPower displayId=%{public}u, state=%{public}u, reason=%{public}u, ffrtId=%{public}u",
        displayId_, static_cast<uint32_t>(state), reason, ffrtId);
    Rosen::ScreenPowerState status = ParseScreenPowerState(state);
    bool ret = false;
    auto changeReason = ParseSpecialReason(reason);
    if (coordinated_ && reason == static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_TIMEOUT)) {
        ret = Rosen::ScreenManagerLite::GetInstance().SetSpecifiedScreenPower(
            displayId_, status, Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_COLLABORATION);
    }
#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
    else if (ScreenPowerOffStrategy::GetInstance().IsSpecificStrategy() &&
        status != Rosen::ScreenPowerState::POWER_ON) {
        DISPLAY_HILOGI(FEAT_STATE, "enable specific screen power strategy");
        ret = Rosen::ScreenManagerLite::GetInstance().SetSpecifiedScreenPower(displayId_, status,
            ParseSpecialReason(static_cast<uint32_t>(ScreenPowerOffStrategy::GetInstance().GetReason())));
    }
#endif
    else {
        ret = Rosen::ScreenManagerLite::GetInstance().SetScreenPowerForAll(status, changeReason);
    }
    ffrtId = ffrt::this_task::get_id();
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    if (!ret) {
        HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "ABNORMAL_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT, "TYPE", "SCREEN_ON_OFF", "REASON", "SetDisplayPower failed");
    }
#endif
    DISPLAY_HILOGI(FEAT_STATE, "[UL_POWER] SetDisplayPower state=%{public}u, reason=%{public}u, "
        "ret=%{public}d, coordinated=%{public}d, ffrtId=%{public}u",
        static_cast<uint32_t>(state), reason, ret, coordinated_, ffrtId);
    return (state == DisplayState::DISPLAY_DIM || state == DisplayState::DISPLAY_SUSPEND) ? true : ret;
}

uint32_t ScreenAction::GetBrightness()
{
    std::lock_guard lock(mutexBrightness_);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    brightness_ = Rosen::DisplayManagerLite::GetInstance().GetScreenBrightness(displayId_);
    IPCSkeleton::SetCallingIdentity(identity);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "displayId=%{public}u, brightness=%{public}u", displayId_, brightness_);
    return brightness_;
}

bool ScreenAction::SetBrightness(uint32_t value)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBrightness displayId=%{public}u, brightness=%{public}u", displayId_, value);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    bool isSucc = Rosen::DisplayManagerLite::GetInstance().SetScreenBrightness(displayId_, value);
    IPCSkeleton::SetCallingIdentity(identity);
    std::lock_guard lock(mutexBrightness_);
    brightness_ = isSucc ? value : brightness_;
    return isSucc;
}

void ScreenAction::SetCoordinated(bool coordinated)
{
    coordinated_ = coordinated;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
