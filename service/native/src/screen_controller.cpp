/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "display_common.h"
#include "display_power_mgr_service.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace DisplayPowerMgr {
const int DISPLAY_FULL_BRIGHTNESS = 100;
const int DISPLAY_DIM_BRIGHTNESS = 50;
const int DISPLAY_OFF_BRIGHTNESS = 0;
const int DISPLAY_SUSPEND_BRIGHTNESS = 50;

ScreenController::ScreenController(uint64_t devId, std::shared_ptr<ScreenAction> action)
    : devId_(devId), state_(DisplayState::DISPLAY_UNKNOWN), action_(action)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "ScreenController created: %{public}d", static_cast<uint32_t>(devId_));
    stateValues_.emplace(DisplayState::DISPLAY_ON, DISPLAY_FULL_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_DIM, DISPLAY_DIM_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_OFF, DISPLAY_OFF_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_SUSPEND, DISPLAY_SUSPEND_BRIGHTNESS);
    animator_ = nullptr;
}

bool ScreenController::UpdateState(DisplayState state, uint32_t reason)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(MODULE_SERVICE, "ScreenController UpdateState: %{public}d, %{public}d",
        static_cast<uint32_t>(devId_), static_cast<uint32_t>(state));
    if (state == state_) {
        return true;
    }

    switch (state) {
        case DisplayState::DISPLAY_ON: // fall through
        case DisplayState::DISPLAY_OFF: {
            BeforeUpdateState(state);
            std::function<void(DisplayState)> callback =
                std::bind(&ScreenController::OnStateChanged, this, std::placeholders::_1);
            bool ret = action_->SetDisplayState(devId_, state, callback);
            if (!ret) {
                DISPLAY_HILOGW(MODULE_SERVICE, "SetDisplayState failed state=%{public}d", state);
                return ret;
            }
            AfterUpdateState(state);
            break;
        }
        case DisplayState::DISPLAY_DIM: // fall through
        case DisplayState::DISPLAY_SUSPEND: {
            bool ret = action_->SetDisplayPower(devId_, state, stateChangeReason_);
            if (!ret) {
                DISPLAY_HILOGW(MODULE_SERVICE, "SetDisplayPower failed state=%{public}d", state);
                return ret;
            }
            break;
        }
        default:
            break;
    }

    state_ = state;
    stateChangeReason_ = reason;

    DISPLAY_HILOGI(MODULE_SERVICE, "Update screen state to %{public}u", state);
    return true;
}

void ScreenController::BeforeUpdateState(DisplayState state)
{
    if (state == DisplayState::DISPLAY_OFF) {
        beforeOffBrightness_ = action_->GetBrightness(devId_);
        DISPLAY_HILOGI(MODULE_SERVICE, "Screen brightness before screen off %{public}d",
            beforeOffBrightness_);
    }
}

void ScreenController::AfterUpdateState(DisplayState state)
{
    if (state == DisplayState::DISPLAY_ON) {
        bool ret = action_->SetBrightness(devId_, beforeOffBrightness_);
        DISPLAY_HILOGI(MODULE_SERVICE, "Is SetBrightness %{public}d, \
            Update brightness to %{public}d", ret, beforeOffBrightness_);
    }
}

bool ScreenController::UpdateBrightness(uint32_t value, uint32_t duraion)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(MODULE_SERVICE, "ScreenController UpdateBrightness: %{public}d, %{public}d, %{public}d",
        static_cast<uint32_t>(devId_), value, duraion);
    if (animator_ == nullptr) {
        std::string name = "ScreenController_" + std::to_string(devId_);
        std::shared_ptr<AnimateCallback> callback = shared_from_this();
        animator_ = std::make_shared<GradualAnimator>(name, callback);
    }
    if (animator_->IsAnimating()) {
        animator_->StopAnimation();
    }
    if (duraion > 0) {
        DISPLAY_HILOGI(MODULE_SERVICE, "UpdateState gradually");
        animator_->StartAnimation(brightness_, value, duraion);
        return true;
    }
    bool ret = action_->SetBrightness(devId_, value);
    if (ret) {
        brightness_ = value;
        DISPLAY_HILOGI(MODULE_SERVICE, "Update brightness to %{public}d", value);
    } else {
        DISPLAY_HILOGI(MODULE_SERVICE, "Update brightness falied! %{public}d", value);
    }
    return ret;
}

bool ScreenController::UpdateStateConfig(DisplayState state, uint32_t value)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(MODULE_SERVICE,
        "ScreenController UpdateStateConfig: Id=%{public}d, State=%{public}d, Value=%{public}d",
        static_cast<uint32_t>(devId_), static_cast<uint32_t>(state), value);
    auto iterator = stateValues_.find(state);
    if (iterator == stateValues_.end()) {
        DISPLAY_HILOGI(MODULE_SERVICE, "UpdateStateConfig No such state");
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
        brightness_ = action_->GetBrightness(devId_);
    }
    return brightness_;
}

void ScreenController::OnStart()
{
    DISPLAY_HILOGD(MODULE_SERVICE, "ScreenAnimatorCallback onStart");
}

void ScreenController::OnChanged(int32_t currentValue)
{
    brightness_ = static_cast<uint32_t>(currentValue);
    bool ret = action_->SetBrightness(devId_, brightness_);
    if (ret) {
        DISPLAY_HILOGD(MODULE_SERVICE, "Update brightness to %{public}d", brightness_);
    } else {
        DISPLAY_HILOGD(MODULE_SERVICE, "Update brightness falied! %{public}d", brightness_);
    }
}

void ScreenController::OnEnd()
{
    DISPLAY_HILOGD(MODULE_SERVICE, "ScreenAnimatorCallback OnEnd");
}

void ScreenController::OnStateChanged(DisplayState state)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "OnStateChanged %{public}d", state);
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(MODULE_SERVICE, "OnRemoteDied no service");
        return;
    }

    bool ret = action_->SetDisplayPower(devId_, state, stateChangeReason_);
    if (ret) {
        pms->NotifyStateChangeCallback(devId_, state);
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
