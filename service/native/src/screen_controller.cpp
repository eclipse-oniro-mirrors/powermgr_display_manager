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

#include "delayed_sp_singleton.h"
#include "display_common.h"
#include "display_log.h"
#include "display_param_helper.h"
#include "display_power_mgr_service.h"
#include "display_setting_helper.h"
#include "ffrt_utils.h"
#include "refbase.h"
#include "setting_provider.h"
#include "system_ability_definition.h"

using namespace std;
using namespace OHOS::PowerMgr;

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
FFRTQueue g_queue("screen_controller");
FFRTHandle g_cancelBoostTaskHandle;
}

ScreenController::ScreenController(uint32_t displayId)
{
    DISPLAY_HILOGI(COMP_SVC, "ScreenController created for displayId=%{public}u", displayId);
    action_ = make_shared<ScreenAction>(displayId);
    state_ = action_->GetDisplayState();

    string name = "BrightnessController_" + to_string(displayId);
    if (animateCallback_ == nullptr) {
        animateCallback_ = make_shared<AnimateCallbackImpl>(action_, [this](uint32_t brightness) {
            SetSettingBrightness(brightness);
        });
    }
    animator_ = make_shared<GradualAnimator>(name, animateCallback_);
}

ScreenController::AnimateCallbackImpl::AnimateCallbackImpl(const std::shared_ptr<ScreenAction>& action,
    std::function<void(uint32_t)> callback)
    : action_(action), callback_(callback)
{
}

void ScreenController::AnimateCallbackImpl::OnStart()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ScreenAnimatorCallback onStart");
}

void ScreenController::AnimateCallbackImpl::OnChanged(uint32_t currentValue)
{
    auto brightness = DisplayPowerMgrService::GetSafeBrightness(static_cast<uint32_t>(currentValue * discount_));
    bool isSucc = action_->SetBrightness(brightness);
    if (isSucc) {
        FFRTTask task = std::bind([this](uint32_t value) { callback_(value); }, currentValue);
        FFRTUtils::SubmitTask(task);
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Update brightness, brightness=%{public}u", currentValue);
    } else {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Update brightness failed, brightness=%{public}d", currentValue);
    }
}

void ScreenController::AnimateCallbackImpl::OnEnd()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ScreenAnimatorCallback OnEnd");
}

void ScreenController::AnimateCallbackImpl::DiscountBrightness(double discount)
{
    discount_ = discount;
}

DisplayState ScreenController::GetState()
{
    return state_;
}

DisplayState ScreenController::SetDelayOffState()
{
    DISPLAY_HILOGI(COMP_SVC, "Set the dispaly state is DELAY OFF when overriding display off delay");
    state_ = DisplayState::DISPLAY_DELAY_OFF;
    return state_;
}

DisplayState ScreenController::SetOnState()
{
    DISPLAY_HILOGI(COMP_SVC, "Set the dispaly state is ON after overriding display on delay");
    state_ = DisplayState::DISPLAY_ON;
    return state_;
}

bool ScreenController::UpdateState(DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(FEAT_STATE, "UpdateState, state=%{public}u, current state=%{public}u",
                   static_cast<uint32_t>(state), static_cast<uint32_t>(state_));
    RETURN_IF_WITH_RET(state == state_, true);

    switch (state) {
        case DisplayState::DISPLAY_ON:
        case DisplayState::DISPLAY_OFF: {
            function<void(DisplayState)> callback = bind(&ScreenController::OnStateChanged, this, placeholders::_1);
            bool ret = action_->SetDisplayState(state, callback);
            if (!ret) {
                DISPLAY_HILOGW(FEAT_STATE, "SetDisplayState failed state=%{public}d", state);
                return ret;
            }
            break;
        }
        case DisplayState::DISPLAY_DIM:
        case DisplayState::DISPLAY_SUSPEND: {
            bool ret = action_->SetDisplayPower(state, stateChangeReason_);
            if (!ret) {
                DISPLAY_HILOGW(FEAT_STATE, "SetDisplayPower failed state=%{public}d", state);
                return ret;
            }
            break;
        }
        default:
            break;
    }

    lock_guard lock(mutexState_);
    state_ = state;
    stateChangeReason_ = reason;

    DISPLAY_HILOGI(FEAT_STATE, "Update screen state to %{public}u", state);
    return true;
}

bool ScreenController::IsScreenOn()
{
    lock_guard lock(mutexState_);
    return (state_ == DisplayState::DISPLAY_ON || state_ == DisplayState::DISPLAY_DIM);
}

bool ScreenController::SetBrightness(uint32_t value, uint32_t gradualDuration)
{
    if (!CanSetBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot set brightness, ignore the change,"\
            "cachedSettingBrightness_ %{public}u -> %{public}u", cachedSettingBrightness_.load(), value);
        cachedSettingBrightness_ = value;
        return false;
    }
    return UpdateBrightness(value, gradualDuration, true);
}

uint32_t ScreenController::GetBrightness()
{
    return GetSettingBrightness();
}

uint32_t ScreenController::GetDeviceBrightness()
{
    return action_->GetBrightness();
}

uint32_t ScreenController::GetCachedSettingBrightness() const
{
    return cachedSettingBrightness_;
}

bool ScreenController::DiscountBrightness(double discount, uint32_t gradualDuration)
{
    if (!CanDiscountBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot discount brightness, ignore the change");
        return false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Discount brightness, discount=%{public}lf", discount);
    discount_ = discount;
    if (animateCallback_) {
        animateCallback_->DiscountBrightness(discount);
    }
    uint32_t screenOnBrightness = GetScreenOnBrightness();
    return UpdateBrightness(screenOnBrightness, gradualDuration);
}

bool ScreenController::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    if (!CanOverrideBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot override brightness, ignore the change");
        return false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Override brightness, value=%{public}u", value);
    if (!isBrightnessOverridden_) {
        isBrightnessOverridden_ = true;
    }
    overriddenBrightness_ = value;
    return UpdateBrightness(value, gradualDuration);
}

bool ScreenController::RestoreBrightness(uint32_t gradualDuration)
{
    if (!IsBrightnessOverridden()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not override, no need to restore");
        return false;
    }
    isBrightnessOverridden_ = false;
    return UpdateBrightness(cachedSettingBrightness_, gradualDuration, true);
}

bool ScreenController::IsBrightnessOverridden() const
{
    return isBrightnessOverridden_;
}

bool ScreenController::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
    if (!CanBoostBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot boost brightness, ignore the change");
        return false;
    }
    bool ret = true;
    if (!isBrightnessBoosted_) {
        uint32_t maxBrightness = DisplayParamHelper::GetMaxBrightness();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Boost brightness, maxBrightness: %{public}d", maxBrightness);
        isBrightnessBoosted_ = true;
        ret = UpdateBrightness(maxBrightness, gradualDuration);
    }

    // If boost multi-times, we will resend the cancel boost event.
    FFRTUtils::CancelTask(g_cancelBoostTaskHandle, g_queue);
    FFRTTask task = std::bind(&ScreenController::CancelBoostBrightness, this, gradualDuration);
    g_cancelBoostTaskHandle = FFRTUtils::SubmitDelayTask(task, timeoutMs, g_queue);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "BoostBrightness update timeout=%{public}u, ret=%{public}d", timeoutMs, ret);
    return ret;
}

bool ScreenController::CancelBoostBrightness(uint32_t gradualDuration)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Cancel boost brightness");
    if (!IsBrightnessBoosted()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not boost, no need to restore");
        return false;
    }
    FFRTUtils::CancelTask(g_cancelBoostTaskHandle, g_queue);
    isBrightnessBoosted_ = false;
    return UpdateBrightness(cachedSettingBrightness_, gradualDuration, true);
}

bool ScreenController::IsBrightnessBoosted() const
{
    return isBrightnessBoosted_;
}

void ScreenController::OnStateChanged(DisplayState state)
{
    auto pms = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    if (pms == nullptr) {
        DISPLAY_HILOGW(FEAT_STATE, "pms is nullptr");
        return;
    }

    bool ret = action_->SetDisplayPower(state, stateChangeReason_);
    if (state == DisplayState::DISPLAY_ON) {
        // Restore the brightness before screen off
        uint32_t screenOnBrightness = GetScreenOnBrightness();
        UpdateBrightness(screenOnBrightness);
    }

    if (ret) {
        pms->NotifyStateChangeCallback(action_->GetDisplayId(), state);
    }
}

bool ScreenController::CanSetBrightness()
{
    bool isScreenOn = IsScreenOn();
    bool isOverridden = IsBrightnessOverridden();
    bool isBoosted = IsBrightnessBoosted();
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "isScreenOn: %{public}d, isOverridden: %{public}d, isBoosted: %{public}d",
        isScreenOn, isOverridden, isBoosted);
    return isScreenOn && !isOverridden && !isBoosted;
}

bool ScreenController::CanDiscountBrightness()
{
    return IsScreenOn();
}

bool ScreenController::CanOverrideBrightness()
{
    return IsScreenOn() && !IsBrightnessBoosted();
}

bool ScreenController::CanBoostBrightness()
{
    return IsScreenOn() && !IsBrightnessOverridden();
}

bool ScreenController::UpdateBrightness(uint32_t value, uint32_t gradualDuration, bool updateSetting)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Update brightness, value=%{public}u, discount=%{public}lf,"\
                   " duration=%{public}u, updateSetting=%{public}d", value, discount_, gradualDuration, updateSetting);

    if (animator_->IsAnimating()) {
        animator_->StopAnimation();
    }
    if (gradualDuration > 0) {
        animator_->StartAnimation(GetSettingBrightness(), value, gradualDuration);
        return true;
    }
    auto brightness = DisplayPowerMgrService::GetSafeBrightness(static_cast<uint32_t>(value * discount_));
    bool isSucc = action_->SetBrightness(brightness);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Updated brightness is %{public}s, brightness: %{public}u",
                   isSucc ? "succ" : "failed", brightness);
    if (isSucc && updateSetting) {
        FFRTUtils::SubmitTask(std::bind(&ScreenController::SetSettingBrightness, this, value));
    }
    return isSucc;
}

uint32_t ScreenController::GetSettingBrightness(const std::string& key) const
{
    uint32_t settingBrightness;
    auto ret = DisplaySettingHelper::GetSettingBrightness(settingBrightness, key);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS,
            "get setting brightness failed, return cachedBrightness_=%{public}u, key=%{public}s, ret=%{public}d",
            cachedSettingBrightness_.load(), key.c_str(), ret);
        return cachedSettingBrightness_;
    }
    return settingBrightness;
}

void ScreenController::SetSettingBrightness(uint32_t value)
{
    DisplaySettingHelper::SetSettingBrightness(value);
    cachedSettingBrightness_ = value;
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "set setting brightness=%{public}u", value);
}

uint32_t ScreenController::GetScreenOnBrightness() const
{
    if (IsBrightnessBoosted()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is boosted, return max brightness");
        return DisplayParamHelper::GetMaxBrightness();
    } else if (IsBrightnessOverridden()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is overridden, return overridden brightness=%{public}u",
                       overriddenBrightness_);
        return overriddenBrightness_;
    } else {
        return GetSettingBrightness();
    }
}

void ScreenController::RegisterSettingBrightnessObserver()
{
    SettingObserver::UpdateFunc updateFunc = [&](const std::string& key) { BrightnessSettingUpdateFunc(key); };
    DisplaySettingHelper::RegisterSettingBrightnessObserver(updateFunc);
}

void ScreenController::BrightnessSettingUpdateFunc(const string& key)
{
    if (animator_->IsAnimating() || !CanSetBrightness()) {
        return;
    }
    uint32_t settingBrightness = GetSettingBrightness(key);
    if (cachedSettingBrightness_ == settingBrightness) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "no need to set setting brightness");
        return;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "setting brightness updated, brightness %{public}u -> %{public}u",
                   cachedSettingBrightness_.load(), settingBrightness);
    cachedSettingBrightness_ = settingBrightness;
    UpdateBrightness(settingBrightness);
}

void ScreenController::UnregisterSettingBrightnessObserver()
{
    DisplaySettingHelper::UnregisterSettingBrightnessObserver();
}

double ScreenController::GetDiscount() const
{
    return discount_;
}

uint32_t ScreenController::GetAnimationUpdateTime() const
{
    return animator_->GetAnimationUpdateTime();
}
} // namespace DisplayPowerMgr
} // namespace OHOS
