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
#include "watchdog.h"

namespace OHOS {
namespace DisplayPowerMgr {
GradualAnimator::GradualAnimator(const std::string& name,
    const std::shared_ptr<AnimateCallback>& callback)
{
    name_ = name;
    callback_ = callback;
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
        DISPLAY_HILOGD(MODULE_SERVICE, "animation is running, no need to start again");
        return;
    }
    DISPLAY_HILOGD(MODULE_SERVICE, "animation from=%{public}u, to=%{public}u, duration=%{public}u",
        from, to, duration);
    if (callback_.lock() == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "callback_ is nullptr");
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
    stride_ = (static_cast<int32_t>(toBrightness_) - static_cast<int32_t>(fromBrightness_)) /
              static_cast<int32_t>(totalSteps_);
    currentStep_ = 0;
    if (handler_ == nullptr) {
        eventRunner_ = AppExecFwk::EventRunner::Create(name_);
        if (eventRunner_ == nullptr) {
            DISPLAY_HILOGW(MODULE_SERVICE, "GradualAnimator failed due to create EventRunner");
            return;
        }
        handler_ = std::make_shared<AnimatorHandler>(eventRunner_, shared_from_this());
        const int32_t WATCH_DOG_DELAY_MS = 10000;
        HiviewDFX::Watchdog::GetInstance().AddThread(name_, handler_, WATCH_DOG_DELAY_MS);
    }
    animating_ = true;
    handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    DISPLAY_HILOGD(MODULE_SERVICE, "animation started");
}

void GradualAnimator::StopAnimation()
{
    animating_ = false;
    handler_->RemoveEvent(EVENT_STEP);
    if (callback_.lock() == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "Callback is nullptr");
        return;
    }
    std::shared_ptr<AnimateCallback> callback = callback_.lock();
    callback->OnEnd();
    DISPLAY_HILOGD(MODULE_SERVICE, "animation stopped");
}

bool GradualAnimator::IsAnimating() const
{
    return animating_;
}

void GradualAnimator::NextStep()
{
    if (!animating_) {
        DISPLAY_HILOGW(MODULE_SERVICE, "is not animating, return");
        return;
    }
    if (callback_.lock() == nullptr) {
        DISPLAY_HILOGW(MODULE_SERVICE, "Callback is nullptr");
        return;
    }
    std::shared_ptr<AnimateCallback> callback = callback_.lock();
    currentStep_++;
    if (currentStep_ == 1) {
        callback->OnStart();
    }
    if (currentStep_ < totalSteps_) {
        currentBrightness_ = currentBrightness_ + stride_;
        callback->OnChanged(currentBrightness_);
        handler_->SendEvent(EVENT_STEP, 0, updateTime_);
    } else {
        currentBrightness_ = toBrightness_;
        callback->OnChanged(currentBrightness_);
        callback->OnEnd();
        animating_ = false;
    }
    DISPLAY_HILOGD(MODULE_SERVICE, "animating next step, step=%{public}u, brightness=%{public}u, stride=%{public}d",
                   currentStep_, currentBrightness_, stride_);
}

GradualAnimator::AnimatorHandler::AnimatorHandler(
    const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const std::shared_ptr<GradualAnimator>& owner)
    : AppExecFwk::EventHandler(runner), owner_(owner)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "AnimatorHandler is created");
}

void GradualAnimator::AnimatorHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "process animator event ,eventId = %{public}d", event->GetInnerEventId());
    std::shared_ptr<GradualAnimator> animator = owner_.lock();
    if (animator == nullptr) {
        DISPLAY_HILOGD(MODULE_SERVICE, "animator is nullptr");
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
