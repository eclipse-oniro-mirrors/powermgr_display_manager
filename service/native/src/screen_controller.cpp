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

#include "screen_controller.h"

#include <utility>
#include <cinttypes>

#include "display_power_mgr_service.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
const int DISPLAY_FULL_BRIGHTNESS = 100;
const int DISPLAY_DIM_BRIGHTNESS = 50;
const int DISPLAY_OFF_BRIGHTNESS = 0;
const int DISPLAY_SUSPEND_BRIGHTNESS = 50;

ScreenController::ScreenController(uint64_t displayId, std::shared_ptr<ScreenAction> action)
    : displayId_(displayId), state_(DisplayState::DISPLAY_UNKNOWN), action_(std::move(action))
{
    DISPLAY_HILOGI(COMP_SVC, "ScreenController created for displayId=%{public}" PRIu64 "", displayId_);
    stateValues_.emplace(DisplayState::DISPLAY_ON, DISPLAY_FULL_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_DIM, DISPLAY_DIM_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_OFF, DISPLAY_OFF_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_SUSPEND, DISPLAY_SUSPEND_BRIGHTNESS);
    animator_ = nullptr;
}

bool ScreenController::UpdateState(DisplayState state, uint32_t reason)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(COMP_SVC,
                   "ScreenController UpdateState, displayId=%{public}" PRIu64 ", state=%{public}u, state_=%{public}u",
                   displayId_, static_cast<uint32_t>(state), static_cast<uint32_t>(state_));
    if (state == state_) {
        return true;
    }

    switch (state) {
        case DisplayState::DISPLAY_ON: // fall through
        case DisplayState::DISPLAY_OFF: {
            BeforeUpdateState(state);
            std::function<void(DisplayState)> callback =
                std::bind(&ScreenController::OnStateChanged, this, std::placeholders::_1);
            bool ret = action_->SetDisplayState(displayId_, state, callback);
            if (!ret) {
                DISPLAY_HILOGW(COMP_SVC, "SetDisplayState failed state=%{public}d", state);
                return ret;
            }
            AfterUpdateState(state);
            break;
        }
        case DisplayState::DISPLAY_DIM: // fall through
        case DisplayState::DISPLAY_SUSPEND: {
            bool ret = action_->SetDisplayPower(displayId_, state, stateChangeReason_);
            if (!ret) {
                DISPLAY_HILOGW(COMP_SVC, "SetDisplayPower failed state=%{public}d", state);
                return ret;
            }
            break;
        }
        default:
            break;
    }

    state_ = state;
    stateChangeReason_ = reason;

    DISPLAY_HILOGI(COMP_SVC, "Update screen state to %{public}u", state);
    return true;
}

void ScreenController::BeforeUpdateState(DisplayState state)
{
    if (state == DisplayState::DISPLAY_OFF) {
        beforeOffBrightness_ = action_->GetBrightness(displayId_);
        beforeOffBrightness_ = (beforeOffBrightness_ <= DISPLAY_OFF_BRIGHTNESS ||
            beforeOffBrightness_ > DISPLAY_FULL_BRIGHTNESS) ?
            DISPLAY_FULL_BRIGHTNESS : beforeOffBrightness_;
        DISPLAY_HILOGI(COMP_SVC, "Screen brightness before screen off %{public}d",
            beforeOffBrightness_);
    }
}

void ScreenController::AfterUpdateState(DisplayState state)
{
    if (state == DisplayState::DISPLAY_ON) {
        bool ret = action_->SetBrightness(displayId_, beforeOffBrightness_);
        DISPLAY_HILOGI(COMP_SVC, "Is SetBrightness %{public}d, \
            Update brightness to %{public}d", ret, beforeOffBrightness_);
    }
}

bool ScreenController::UpdateBrightness(uint32_t value, uint32_t gradualDuration)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(COMP_SVC, "UpdateBrightness" \
                   ", displayId=%{public}" PRIu64 ", value=%{public}u, duration=%{public}u",
                   displayId_, value, gradualDuration);
    if (animator_ == nullptr) {
        std::string name = "ScreenController_" + std::to_string(displayId_);
        std::shared_ptr<AnimateCallback> callback = shared_from_this();
        animator_ = std::make_shared<GradualAnimator>(name, callback);
    }
    if (animator_->IsAnimating()) {
        animator_->StopAnimation();
    }
    if (gradualDuration > 0) {
        DISPLAY_HILOGI(COMP_SVC, "UpdateBrightness gradually");
        animator_->StartAnimation(brightness_, value, gradualDuration);
        return true;
    }
    bool ret = action_->SetBrightness(displayId_, value);
    if (ret) {
        brightness_ = value;
        DISPLAY_HILOGI(COMP_SVC, "Update brightness, value=%{public}u", value);
    } else {
        DISPLAY_HILOGI(COMP_SVC, "Update brightness failed, value=%{public}u", value);
    }
    return ret;
}

bool ScreenController::SetBrightness(uint32_t value, uint32_t gradualDuration)
{
    DISPLAY_HILOGI(COMP_SVC, "Set brightness, value=%{public}u", value);
    if (isBrightnessOverride_) {
        DISPLAY_HILOGI(COMP_SVC, "brightness is override, ignore the change");
        return false;
    }
    return UpdateBrightness(value, gradualDuration);
}

bool ScreenController::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    DISPLAY_HILOGI(COMP_SVC, "Override brightness, value=%{public}u", value);
    if (!isBrightnessOverride_) {
        isBrightnessOverride_ = true;
        beforeOverrideBrightness_ = brightness_;
    } else if (value == beforeOverrideBrightness_) {
        isBrightnessOverride_ = false;
    }
    return UpdateBrightness(value, gradualDuration);
}

bool ScreenController::UpdateStateConfig(DisplayState state, uint32_t value)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(COMP_SVC,
                   "ScreenController UpdateStateConfig" \
                   ", displayId=%{public}" PRIu64 ", state=%{public}u, value=%{public}u",
                   displayId_, static_cast<uint32_t>(state), value);
    auto iterator = stateValues_.find(state);
    if (iterator == stateValues_.end()) {
        DISPLAY_HILOGI(COMP_SVC, "UpdateStateConfig no such state");
        return false;
    }
    iterator->second = value;
    return true;
}

bool ScreenController::IsScreenOn()
{
    std::lock_guard lock(mutex_);
    return (state_ == DisplayState::DISPLAY_ON || state_ == DisplayState::DISPLAY_DIM);
}

uint32_t ScreenController::GetBrightness()
{
    std::lock_guard lock(mutex_);
    if (brightness_ == 0) {
        brightness_ = action_->GetBrightness(displayId_);
    }
    return brightness_;
}

bool ScreenController::IsBrightnessOverride()
{
    return isBrightnessOverride_.load();
}

void ScreenController::OnStart()
{
    DISPLAY_HILOGD(COMP_SVC, "ScreenAnimatorCallback onStart");
}

void ScreenController::OnChanged(int32_t currentValue)
{
    brightness_ = static_cast<uint32_t>(currentValue);
    bool ret = action_->SetBrightness(displayId_, brightness_);
    if (ret) {
        DISPLAY_HILOGD(COMP_SVC, "Update brightness, brightness_=%{public}u", brightness_);
    } else {
        DISPLAY_HILOGD(COMP_SVC, "Update brightness failed, brightness_=%{public}d", brightness_);
    }
}

void ScreenController::OnEnd()
{
    DISPLAY_HILOGD(COMP_SVC, "ScreenAnimatorCallback OnEnd");
}

void ScreenController::OnStateChanged(DisplayState state)
{
    DISPLAY_HILOGD(COMP_SVC, "OnStateChanged %{public}u", static_cast<uint32_t>(state));
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "OnRemoteDied no service");
        return;
    }

    bool ret = action_->SetDisplayPower(displayId_, state, stateChangeReason_);
    if (ret) {
        pms->NotifyStateChangeCallback(displayId_, state);
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
