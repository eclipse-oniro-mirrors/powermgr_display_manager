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

#include "display_log.h"
#include "display_param_helper.h"
#include "display_power_mgr_service.h"

using namespace std;
namespace OHOS {
namespace DisplayPowerMgr {
constexpr uint32_t DISPLAY_FULL_BRIGHTNESS = 255;
constexpr uint32_t DISPLAY_OFF_BRIGHTNESS = 0;

ScreenController::ScreenController(uint32_t displayId, const shared_ptr<DisplayEventHandler>& handler)
{
    DISPLAY_HILOGI(COMP_SVC, "ScreenController created for displayId=%{public}u", displayId);
    sharedControl_ = make_shared<SharedController>(displayId);
    stateControl_ = make_shared<DisplayStateController>(sharedControl_);
    overrideControl_ = make_unique<OverrideController>(sharedControl_, stateControl_, handler);
}

ScreenController::AnimateController ::AnimateController (const shared_ptr<ScreenAction>& action)
    : action_(action)
{
}

void ScreenController::AnimateController ::OnStart()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ScreenAnimatorCallback onStart");
}

void ScreenController::AnimateController ::OnChanged(uint32_t currentValue)
{
    bool ret = action_->SetBrightness(currentValue);
    if (ret) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Update brightness, brightness=%{public}u", currentValue);
    } else {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Update brightness failed, brightness=%{public}d", currentValue);
    }
}

void ScreenController::AnimateController ::OnEnd()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ScreenAnimatorCallback OnEnd");
}

ScreenController::SharedController::SharedController(const uint32_t displayId)
{
    action_ = make_shared<ScreenAction>(displayId);
    string name = "BrightnessController_" + to_string(displayId);
    shared_ptr<AnimateCallback> callback = make_shared<AnimateController >(action_);
    animator_ = make_shared<GradualAnimator>(name, callback);
}

bool ScreenController::SharedController::SetBrightness(uint32_t value, uint32_t gradualDuration)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Set brightness, value=%{public}u", value);
    if (!isAllow_) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "brightness is not allowed, ignore the change");
        return false;
    }
    return UpdateBrightness(value, gradualDuration);
}

uint32_t ScreenController::SharedController::GetBrightness()
{
    lock_guard lock(mutex_);
    return action_->GetBrightness();
}

void ScreenController::SharedController::AllowAdjustBrightness(bool allow)
{
    isAllow_ = allow;
}

shared_ptr<ScreenAction>& ScreenController::SharedController::GetAction()
{
    return action_;
}

bool ScreenController::SharedController::UpdateBrightness(uint32_t value, uint32_t gradualDuration)
{
    lock_guard lock(mutex_);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Update brightness, value=%{public}u, duration=%{public}u",
                   value, gradualDuration);
    
    if (animator_->IsAnimating()) {
        animator_->StopAnimation();
    }
    if (gradualDuration > 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Update brightness gradually");
        animator_->StartAnimation(action_->GetBrightness(), value, gradualDuration);
        return true;
    }
    bool isSucc = action_->SetBrightness(value);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Updated brightness is %{public}s, brightness: %{public}u",
        isSucc ? "succ" : "failed", value);
    return isSucc;
}

ScreenController::OverrideController::OverrideController(const shared_ptr<SharedController>& sharedControl,
    const std::shared_ptr<DisplayStateController>& stateControl,
    const shared_ptr<DisplayEventHandler>& handler)
    : sharedControl_(sharedControl), stateControl_(stateControl), handler_(handler)
{
    DisplayEventHandler::EventCallback callback = bind([&]() { CancelBoostBrightness(); });
    handler_->EmplaceCallBack(DisplayEventHandler::Event::EVENT_CANCEL_BOOST_BRIGHTNESS, callback);
}

bool ScreenController::OverrideController::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    lock_guard lock(mutexOverride_);
    bool screenOn = stateControl_->IsScreenOn();
    // Cannot be called under overwrite or off screen
    if (isBoostBrightness_ || !screenOn) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot override brightness. isBoost: %{public}d, screenOn: %{public}d",
            isBoostBrightness_.load(), screenOn);
        return false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Override brightness, value=%{public}u", value);
    if (!isBrightnessOverride_) {
        SaveBeforeBrightness(isBrightnessOverride_);
    }
    return sharedControl_->UpdateBrightness(value, gradualDuration);
}

bool ScreenController::OverrideController::RestoreBrightness(uint32_t gradualDuration)
{
    lock_guard lock(mutexOverride_);
    if (!isBrightnessOverride_ || isBoostBrightness_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not override, no need to restore");
        return false;
    }
    return BrightnessBeforeRestore(isBrightnessOverride_, gradualDuration);
}

bool ScreenController::OverrideController::IsBrightnessOverride() const
{
    return isBrightnessOverride_;
}

uint32_t ScreenController::OverrideController::GetBeforeBrightness() const
{
    return beforeBrightness_;
}

bool ScreenController::OverrideController::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
    lock_guard lock(mutexOverride_);
    bool screenOn = stateControl_->IsScreenOn();
    // Calls are not allowed under Boost or off screen
    if (isBrightnessOverride_ || !screenOn) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot boost brightness, isOverride: %{public}d, screenOn: %{public}d",
            isBrightnessOverride_.load(), screenOn);
        return false;
    }
    if (!isBoostBrightness_) {
        uint32_t maxBrightness = DisplayParamHelper::GetInstance().GetMaxBrightness();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Boost brightness, maxBrightness: %{public}d", maxBrightness);
        SaveBeforeBrightness(isBoostBrightness_);
        sharedControl_->UpdateBrightness(maxBrightness, gradualDuration);
    }

    handler_->RemoveEvent(DisplayEventHandler::Event::EVENT_CANCEL_BOOST_BRIGHTNESS);
    handler_->SendEvent(DisplayEventHandler::Event::EVENT_CANCEL_BOOST_BRIGHTNESS, timeoutMs);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "BoostBrightness update timeout");

    return true;
}

bool ScreenController::OverrideController::CancelBoostBrightness()
{
    lock_guard lock(mutexOverride_);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Cancel boost brightness");
    if (!isBoostBrightness_ || isBrightnessOverride_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not boost, no need to restore");
        return false;
    }
    handler_->RemoveEvent(DisplayEventHandler::Event::EVENT_CANCEL_BOOST_BRIGHTNESS);
    return BrightnessBeforeRestore(isBoostBrightness_, SCREEN_BRIGHTNESS_UPDATE_DURATION);
}

bool ScreenController::OverrideController::IsBoostBrightness() const
{
    return isBoostBrightness_;
}

void ScreenController::OverrideController::SaveBeforeBrightness(atomic<bool>& isOverride)
{
    isOverride = true;
    // Disable external calls to brightness adjustment
    sharedControl_->AllowAdjustBrightness(false);
    beforeBrightness_ = sharedControl_->GetBrightness();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Gets the current system brightness, beforeBrightness_=%{public}u",
        beforeBrightness_);
}

bool ScreenController::OverrideController::BrightnessBeforeRestore(atomic<bool>& isOverride,
    uint32_t gradualDuration)
{
    isOverride = false;
    // Allows external calls to brightness adjustment
    sharedControl_->AllowAdjustBrightness(true);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Restore system Brightness. value=%{public}u", beforeBrightness_);
    // After the screen is off, the system brightness is restored
    stateControl_->UpdateBeforeOffBrightness(beforeBrightness_);
    return sharedControl_->UpdateBrightness(beforeBrightness_, gradualDuration);
}

bool ScreenController::DisplayStateController::UpdateState(DisplayState state, uint32_t reason)
{
    lock_guard lock(mutexState_);
    DISPLAY_HILOGI(COMP_SVC, "UpdateState, state=%{public}u, current state=%{public}u",
                   static_cast<uint32_t>(state), static_cast<uint32_t>(state_));
    RETURN_IF_WITH_RET(state == state_, true);

    switch (state) {
        case DisplayState::DISPLAY_ON:
        case DisplayState::DISPLAY_OFF: {
            BeforeScreenOff(state);
            function<void(DisplayState)> callback =
                bind(&ScreenController::DisplayStateController::OnStateChanged, this, placeholders::_1);
            bool ret = sharedControl_->GetAction()->SetDisplayState(state, callback);
            if (!ret) {
                DISPLAY_HILOGW(COMP_SVC, "SetDisplayState failed state=%{public}d", state);
                return ret;
            }
            AfterScreenOn(state);
            break;
        }
        case DisplayState::DISPLAY_DIM:
        case DisplayState::DISPLAY_SUSPEND: {
            bool ret = sharedControl_->GetAction()->SetDisplayPower(state, stateChangeReason_);
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

bool ScreenController::DisplayStateController::IsScreenOn()
{
    lock_guard lock(mutexState_);
    return (state_ == DisplayState::DISPLAY_ON || state_ == DisplayState::DISPLAY_DIM);
}

void ScreenController::DisplayStateController::UpdateBeforeOffBrightness(uint32_t brightness)
{
    lock_guard lock(mutexState_);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "After the screen is off, the system brightness is restored." \
        "brightness: %{public}d", brightness);
    beforeOffBrightness_ = brightness;
}

void ScreenController::DisplayStateController::BeforeScreenOff(DisplayState state)
{
    if (state == DisplayState::DISPLAY_OFF) {
        beforeOffBrightness_ = sharedControl_->GetBrightness();
        beforeOffBrightness_ = (beforeOffBrightness_ <= DISPLAY_OFF_BRIGHTNESS ||
            beforeOffBrightness_ > DISPLAY_FULL_BRIGHTNESS) ?
            DISPLAY_FULL_BRIGHTNESS : beforeOffBrightness_;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Screen brightness before screen off %{public}d",
            beforeOffBrightness_);
    }
}

void ScreenController::DisplayStateController::AfterScreenOn(DisplayState state)
{
    if (state == DisplayState::DISPLAY_ON) {
        bool ret = sharedControl_->UpdateBrightness(beforeOffBrightness_, 0);
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Is SetBrightness %{public}d, \
            Update brightness to %{public}u", ret, beforeOffBrightness_);
    }
}

void ScreenController::DisplayStateController::OnStateChanged(DisplayState state)
{
    DISPLAY_HILOGD(COMP_SVC, "OnStateChanged %{public}u", static_cast<uint32_t>(state));
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGI(COMP_SVC, "OnRemoteDied no service");
        return;
    }

    bool ret = sharedControl_->GetAction()->SetDisplayPower(state, stateChangeReason_);
    if (ret) {
        pms->NotifyStateChangeCallback(sharedControl_->GetAction()->GetDisplayId(), state);
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
