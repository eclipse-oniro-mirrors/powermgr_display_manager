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

#include "ffrt_utils.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
using namespace PowerMgr;
namespace {
FFRTQueue g_animatorQueue("display_animator_queue");
FFRTHandle g_animatorTaskHandle;
}
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
    currentBrightness_ = fromBrightness_.load();
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
    animating_ = true;
    FFRTTask task = std::bind(&GradualAnimator::NextStep, this);
    g_animatorTaskHandle = FFRTUtils::SubmitDelayTask(task, updateTime_, g_animatorQueue);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animation started");
}

void GradualAnimator::StopAnimation()
{
    animating_ = false;
    FFRTUtils::CancelTask(g_animatorTaskHandle, g_animatorQueue);
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
            currentBrightness_ = toBrightness_.load();
            callback_->OnChanged(currentBrightness_);
            callback_->OnEnd();
            animating_ = false;
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "next step brightness is out range, brightness=%{public}u", nextBrightness);
        } else {
            currentBrightness_ = nextBrightness;
            callback_->OnChanged(currentBrightness_);
            FFRTTask task = std::bind(&GradualAnimator::NextStep, this);
            g_animatorTaskHandle = FFRTUtils::SubmitDelayTask(task, updateTime_, g_animatorQueue);
        }
    } else {
        currentBrightness_ = toBrightness_.load();
        callback_->OnChanged(currentBrightness_);
        callback_->OnEnd();
        animating_ = false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animating next step, step=%{public}u, brightness=%{public}u, stride=%{public}d",
                   currentStep_.load(), currentBrightness_.load(), stride_.load());
}
} // namespace DisplayPowerMgr
} // namespace OHOS
