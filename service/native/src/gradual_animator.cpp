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

#include "gradual_animator.h"

#include "display_common.h"
#include "watchdog.h"

namespace OHOS {
namespace DisplayPowerMgr {
GradualAnimator::GradualAnimator(const std::string& name,
    std::shared_ptr<AnimateCallback> callback)
{
    DISPLAY_HILOGD(COMP_SVC, "GradualAnimator construct start");
    name_ = name;
    callback_ = callback;
    from_ = 0;
    to_ = 0;
    current_ = 0;
    duration_ = 0;
    steps_ = 0;
    stride_ = 0;
    updateTime_ = DEFAULT_UPDATE_TIME;
    handler_ = nullptr;
    DISPLAY_HILOGD(COMP_SVC, "GradualAnimator construct end");
}

void GradualAnimator::StartAnimation(uint32_t from, uint32_t to, uint32_t duration)
{
    if (animating_) {
        DISPLAY_HILOGD(COMP_SVC, "StartAnimation is running...");
        return;
    }
    DISPLAY_HILOGD(COMP_SVC,
        "StartAnimation from=%{public}u, to=%{public}u, duration=%{public}u",
        from, to, duration);
    if (callback_.lock() == nullptr) {
        DISPLAY_HILOGW(COMP_SVC, "Callback is NULL");
        return;
    }
    from_ = from;
    to_ = to;
    current_ = from_;
    duration_ = duration;
    steps_ = duration_ / updateTime_;
    if (steps_ < 1) {
        steps_ = 1;
    }
    stride_ = (to_ - from_) / static_cast<int32_t>(steps_);
    currentStep_ = 0;
    if (handler_ == nullptr) {
        eventRunner_ = AppExecFwk::EventRunner::Create(name_);
        if (eventRunner_ == nullptr) {
            DISPLAY_HILOGW(COMP_SVC, "GradualAnimator failed due to create EventRunner");
            return;
        }
        handler_ = std::make_shared<AnimatorHandler>(eventRunner_, shared_from_this());
        const int32_t WATCH_DOG_DELAY_MS = 10000;
        HiviewDFX::Watchdog::GetInstance().AddThread(name_, handler_, WATCH_DOG_DELAY_MS);
    }
    animating_ = true;
    handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    DISPLAY_HILOGD(COMP_SVC, "StartAnimation end");
}

void GradualAnimator::StopAnimation()
{
    DISPLAY_HILOGD(COMP_SVC, "GradualAnimator StopAnimation start");
    animating_ = false;
    handler_->RemoveEvent(EVENT_STEP);
    if (callback_.lock() == nullptr) {
        DISPLAY_HILOGW(COMP_SVC, "Callback is NULL");
        return;
    }
    std::shared_ptr<AnimateCallback> callback = callback_.lock();
    callback->OnEnd();
    DISPLAY_HILOGD(COMP_SVC, "GradualAnimator StopAnimation end");
}

bool GradualAnimator::IsAnimating()
{
    return animating_;
}

void GradualAnimator::NextStep()
{
    if (!animating_) {
        DISPLAY_HILOGW(COMP_SVC, "NextStep, not animating");
        return;
    }
    if (callback_.lock() == nullptr) {
        DISPLAY_HILOGW(COMP_SVC, "Callback is NULL");
        return;
    }
    std::shared_ptr<AnimateCallback> callback = callback_.lock();
    currentStep_++;
    if (currentStep_ == 1) {
        callback->OnStart();
    }
    if (currentStep_ < steps_) {
        current_ = current_ + stride_;
        callback->OnChanged(current_);
        handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    } else {
        current_ = to_;
        callback->OnChanged(current_);
        callback->OnEnd();
        animating_ = false;
    }
    DISPLAY_HILOGD(COMP_SVC, "NextStep: Step=%{public}d, current=%{public}d, stride=%{public}d",
        currentStep_, current_, stride_);
}

GradualAnimator::AnimatorHandler::AnimatorHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    std::shared_ptr<GradualAnimator> owner)
    : AppExecFwk::EventHandler(runner), owner_(owner)
{
    DISPLAY_HILOGD(COMP_SVC, "AnimatorHandler is created");
}

void GradualAnimator::AnimatorHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    DISPLAY_HILOGD(COMP_SVC, "AnimatorHandler::%{public}s ,eventid = %d", __func__,
        event->GetInnerEventId());
    std::shared_ptr<GradualAnimator> animator = owner_.lock();
    if (animator == nullptr) {
        DISPLAY_HILOGD(COMP_SVC, "AnimatorHandler no object");
        return;
    }
    switch (event->GetInnerEventId()) {
        case EVENT_STEP: {
            animator->NextStep();
            break;
        }
        default:
            break;
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
