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
#include "hilog_wrapper.h"

namespace OHOS {
namespace DisplayPowerMgr {
const int DISPLAY_FULL_BRIGHTNESS = 100;
const int DISPLAY_DIM_BRIGHTNESS = 50;
const int DISPLAY_OFF_BRIGHTNESS = 0;
const int DISPLAY_SUSPEND_BRIGHTNESS = 50;

ScreenController::ScreenController(uint32_t devId, std::shared_ptr<ScreenAction> action)
    : devId_(devId), state_(DisplayState::DISPLAY_UNKNOWN), action_(action)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "ScreenController created: %{public}d", devId_);
    stateValues_.emplace(DisplayState::DISPLAY_ON, DISPLAY_FULL_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_DIM, DISPLAY_DIM_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_OFF, DISPLAY_OFF_BRIGHTNESS);
    stateValues_.emplace(DisplayState::DISPLAY_SUSPEND, DISPLAY_SUSPEND_BRIGHTNESS);
    animator_ = nullptr;
}

bool ScreenController::UpdateState(DisplayState state)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(MODULE_SERVICE, "ScreenController UpdateState: %{public}d, %{public}d",
        devId_, static_cast<uint32_t>(state));
    if (state == state_) {
        return true;
    }

    bool ret = action_->SetPowerState(devId_, state);
    if (!ret) {
        DISPLAY_HILOGW(MODULE_SERVICE, "SetPowerState failed state=%{public}d", state);
    }
    state_ = state;
    auto iterator = stateValues_.find(state);
    if (iterator != stateValues_.end()) {
        ret = action_->SetBrightness(devId_, iterator->second);
        if (ret) {
            brightness_ = iterator->second;
        } else {
            DISPLAY_HILOGI(MODULE_SERVICE, "set brightness falied! %{public}d", iterator->second);
        }
    }
    DISPLAY_HILOGI(MODULE_SERVICE, "Update screen state to %{public}u", state);
    return true;
}

bool ScreenController::UpdateBrightness(uint32_t value, uint32_t duraion)
{
    std::lock_guard lock(mutex_);
    DISPLAY_HILOGI(MODULE_SERVICE, "ScreenController UpdateState: %{public}d, %{public}d",
        devId_, value);
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
        animator_->StartAnimation(brightness_, value, SCREEN_BRIGHTNESS_UPDATE_DURATION);
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
        devId_, static_cast<uint32_t>(state), value);
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

void ScreenController::onStart()
{
    DISPLAY_HILOGD(MODULE_SERVICE, "ScreenAnimatorCallback onStart");
}

void ScreenController::onChanged(int32_t currentValue)
{
    brightness_ = currentValue;
    bool ret = action_->SetBrightness(devId_, currentValue);
    if (ret) {
        DISPLAY_HILOGD(MODULE_SERVICE, "Update brightness to %{public}d", currentValue);
    } else {
        DISPLAY_HILOGD(MODULE_SERVICE, "Update brightness falied! %{public}d", currentValue);
    }
}

void ScreenController::onEnd()
{
    DISPLAY_HILOGD(MODULE_SERVICE, "ScreenAnimatorCallback OnEnd");
}
} // namespace DisplayPowerMgr
} // namespace OHOS
