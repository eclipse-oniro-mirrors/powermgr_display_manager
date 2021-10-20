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

#include "gradual_animator.h"

#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
GradualAnimator::GradualAnimator(const std::string& name,
    std::shared_ptr<AnimateCallback> callback)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "GradualAnimator construct start");
    name_ = name;
    callback_ = callback;
    eventRunner_ = AppExecFwk::EventRunner::Create(name_);
    if (eventRunner_ == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "GradualAnimator failed due to create EventRunner");
    }
    from_ = 0;
    to_ = 0;
    current_ = 0;
    duration_ = 0;
    steps_ = 0;
    stride_ = 0;
    updateTime_ = DEFAULT_UPDATE_TIME;
    handler_ = nullptr;
    DISPLAY_HILOGD(MODULE_SERVICE, "GradualAnimator construct end");
}

void GradualAnimator::StartAnimation(int32_t from, int32_t to, uint32_t duration)
{
    DISPLAY_HILOGD(MODULE_SERVICE,
        "StartAnimation from=%{public}d, to=%{public}d, duration=%{public}d",
        from, to, duration);
    if (callback_ == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "Callback is NULL");
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
    stride_ = (to_ - from_) / steps_;
    currentStep_ = 0;
    if (handler_ == nullptr) {
        handler_ = std::make_shared<AnimatorHandler>(eventRunner_, shared_from_this());
    }
    animating_ = true;
    handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    DISPLAY_HILOGD(MODULE_SERVICE, "StartAnimation end");
}

void GradualAnimator::StopAnimation()
{
    DISPLAY_HILOGD(MODULE_SERVICE, "GradualAnimator StopAnimation start");
    animating_ = false;
    handler_->RemoveEvent(EVENT_STEP);
    if (callback_ == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "Callback is NULL");
        return;
    }
    callback_->onEnd();
    DISPLAY_HILOGD(MODULE_SERVICE, "GradualAnimator StopAnimation end");
}

bool GradualAnimator::IsAnimating()
{
    return animating_;
}

void GradualAnimator::NextStep()
{
    if (!animating_) {
        DISPLAY_HILOGW(MODULE_SERVICE, "NextStep, not animating");
        return;
    }
    if (callback_ == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "Callback is NULL");
        return;
    }
    currentStep_++;
    if (currentStep_ == 1) {
        callback_->onStart();
    }
    if (currentStep_ < steps_) {
        current_ = current_ + stride_;
        callback_->onChanged(current_);
        handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    } else {
        current_ = to_;
        callback_->onChanged(current_);
        callback_->onEnd();
        animating_ = false;
    }
}

GradualAnimator::AnimatorHandler::AnimatorHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    std::shared_ptr<GradualAnimator> owner)
    : AppExecFwk::EventHandler(runner), owner_(owner)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "AnimatorHandler is created");
}

void GradualAnimator::AnimatorHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "AnimatorHandler::%{public}s ,eventid = %d", __func__,
        event->GetInnerEventId());
    std::shared_ptr<GradualAnimator> animator = owner_.lock();
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
