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

#include "gradual_animator.h"

#include "display_log.h"
#include "xcollie/watchdog.h"

namespace OHOS {
namespace DisplayPowerMgr {
GradualAnimator::GradualAnimator(const std::string& name, std::shared_ptr<AnimateCallback>& callback)
    : name_(name), callback_(callback)
{
    fromBrightness_ = 0;
    toBrightness_ = 0;
    currentBrightness_ = 0;
    duration_ = 0;
    totalSteps_ = 0;
    stride_ = 0;
    updateTime_ = DEFAULT_UPDATE_TIME;
    handler_ = nullptr;
}

void GradualAnimator::StartAnimation(uint32_t from, uint32_t to, uint32_t duration)
{
    if (animating_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animation is running, no need to start again");
        return;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animation from=%{public}u, to=%{public}u, duration=%{public}u",
        from, to, duration);
    if (callback_ == nullptr) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "callback_ is nullptr");
        return;
    }
    fromBrightness_ = from;
    toBrightness_ = to;
    currentBrightness_ = fromBrightness_;
    duration_ = duration;
    totalSteps_ = duration_ / updateTime_;
    if (totalSteps_ < 1) {
        totalSteps_ = 1;
    }
    int32_t changeBrightness = static_cast<int32_t>(toBrightness_) - static_cast<int32_t>(fromBrightness_);
    stride_ = changeBrightness / static_cast<int32_t>(totalSteps_);
    if (abs(stride_) < STRIDE_ABSOLUTE_MIN) {
        stride_ = (changeBrightness / abs(changeBrightness)) * STRIDE_ABSOLUTE_MIN;
    }
    currentStep_ = 0;
    if (handler_ == nullptr) {
        eventRunner_ = AppExecFwk::EventRunner::Create(name_);
        if (eventRunner_ == nullptr) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "GradualAnimator failed due to create EventRunner");
            return;
        }
        handler_ = std::make_shared<AnimatorHandler>(eventRunner_, shared_from_this());
        HiviewDFX::Watchdog::GetInstance().AddThread(name_, handler_);
    }
    animating_ = true;
    handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animation started");
}

void GradualAnimator::StopAnimation()
{
    animating_ = false;
    handler_->RemoveEvent(EVENT_STEP);
    if (callback_ == nullptr) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Callback is nullptr");
        return;
    }
    callback_->OnEnd();
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animation stopped");
}

bool GradualAnimator::IsAnimating() const
{
    return animating_;
}

uint32_t GradualAnimator::GetAnimationUpdateTime() const
{
    return updateTime_;
}

void GradualAnimator::NextStep()
{
    if (!animating_) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "is not animating, return");
        return;
    }
    if (callback_ == nullptr) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Callback is nullptr");
        return;
    }
    currentStep_++;
    if (currentStep_ == 1) {
        callback_->OnStart();
    }
    if (currentStep_ < totalSteps_) {
        uint32_t nextBrightness = currentBrightness_ + stride_;
        bool isOutRange = stride_ > 0 ? (nextBrightness >= toBrightness_) : (nextBrightness <= toBrightness_);
        if (isOutRange) {
            currentBrightness_ = toBrightness_;
            callback_->OnChanged(currentBrightness_);
            callback_->OnEnd();
            animating_ = false;
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "next step brightness is out range, brightness=%{public}u", nextBrightness);
        } else {
            currentBrightness_ = nextBrightness;
            callback_->OnChanged(currentBrightness_);
            handler_->SendEvent(EVENT_STEP, 0, updateTime_);
        }
    } else {
        currentBrightness_ = toBrightness_;
        callback_->OnChanged(currentBrightness_);
        callback_->OnEnd();
        animating_ = false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animating next step, step=%{public}u, brightness=%{public}u, stride=%{public}d",
                   currentStep_, currentBrightness_, stride_);
}

GradualAnimator::AnimatorHandler::AnimatorHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const std::shared_ptr<GradualAnimator>& owner)
    : AppExecFwk::EventHandler(runner), owner_(owner)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AnimatorHandler is created");
}

void GradualAnimator::AnimatorHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "process animator event ,eventId = %{public}d", event->GetInnerEventId());
    std::shared_ptr<GradualAnimator> animator = owner_.lock();
    if (animator == nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animator is nullptr");
        return;
    }
    if (event->GetInnerEventId() == EVENT_STEP) {
        animator->NextStep();
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
