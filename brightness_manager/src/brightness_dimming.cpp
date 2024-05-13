/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "brightness_dimming.h"

#include "brightness_dimming_callback.h"
#include "display_log.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace DisplayPowerMgr {
using namespace PowerMgr;
namespace {
FFRTHandle g_animatorTaskHandle;
}

BrightnessDimming::BrightnessDimming(const std::string& name, std::shared_ptr<BrightnessDimmingCallback>& callback)
    : mName(name), mCallback(callback)
{
    mFromBrightness = 0;
    mToBrightness = 0;
    mCurrentBrightness = 0;
    mDuration = 0;
    mTotalSteps = 0;
    mStride = 0;
    mUpdateTime = DEFAULT_UPDATE_TIME;
}

bool BrightnessDimming::Init()
{
    mQueue = std::make_shared<FFRTQueue> ("brightness_animator_queue");
    if (mQueue == nullptr) {
        return false;
    }
    return true;
}

void BrightnessDimming::Reset()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "start dimming queue");
    if (mQueue) {
        mQueue.reset();
        g_animatorTaskHandle = nullptr;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "destruct dimming_queue");
    }
}

void BrightnessDimming::StartDimming(uint32_t from, uint32_t to, uint32_t duration)
{
    if (mDimming) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "animation is running, no need to start again, from=%{public}u, "\
            " to=%{public}u, duration=%{public}u", from, to, duration);
        return;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "animation from=%{public}u, to=%{public}u, duration=%{public}u",
        from, to, duration);
    if (mCallback == nullptr) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "mCallback is nullptr");
        return;
    }
    mFromBrightness = from;
    mToBrightness = to;
    mCurrentBrightness = mFromBrightness.load();
    mDuration = duration;
    mTotalSteps = mDuration / mUpdateTime;
    if (mTotalSteps < 1) {
        mTotalSteps = 1;
    }
    int32_t changeBrightness = static_cast<int32_t>(mToBrightness) - static_cast<int32_t>(mFromBrightness);
    if (changeBrightness == 0) {
        return;
    }
    mStride = changeBrightness / static_cast<int32_t>(mTotalSteps);
    if (abs(mStride) < STRIDE_ABSOLUTE_MIN) {
        mStride = (changeBrightness / abs(changeBrightness)) * STRIDE_ABSOLUTE_MIN;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "animation orig mTotalSteps=%{public}d", static_cast<int32_t>(mTotalSteps));
    if (mTotalSteps > 1 && abs(mStride) >= 1) {
        mTotalSteps = static_cast<uint32_t>((abs(changeBrightness) - abs(mStride) *
            mTotalSteps)) / abs(mStride) + mTotalSteps;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "animation update mTotalSteps=%{public}d", static_cast<int32_t>(mTotalSteps));
    }
    mCurrentStep = 0;
    mDimming = true;
    FFRTTask task = std::bind(&BrightnessDimming::NextStep, this);
    g_animatorTaskHandle = FFRTUtils::SubmitDelayTask(task, mUpdateTime, mQueue);
}

void BrightnessDimming::StopDimming()
{
    mDimming = false;
    FFRTUtils::CancelTask(g_animatorTaskHandle, mQueue);
    if (mCallback == nullptr) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Callback is nullptr");
        return;
    }
    mCallback->OnEnd();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "animation stopped");
}

bool BrightnessDimming::IsDimming() const
{
    return mDimming;
}

uint32_t BrightnessDimming::GetDimmingUpdateTime() const
{
    return mUpdateTime;
}

void BrightnessDimming::NextStep()
{
    if (!mDimming) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "is not animating, return");
        return;
    }
    if (mCallback == nullptr) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Callback is nullptr");
        return;
    }
    mCurrentStep++;
    if (mCurrentStep == 1) {
        mCallback->OnStart();
    }
    if (mCurrentStep <= mTotalSteps) {
        uint32_t nextBrightness = mCurrentBrightness + static_cast<uint32_t>(mStride);
        bool isOutRange = (mStride > 0 ? (nextBrightness >= mToBrightness) : (nextBrightness <= mToBrightness));
        if (isOutRange) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "next step brightness is out range, brightness=%{public}u",
                nextBrightness);
            mCurrentBrightness = mToBrightness.load();
            mCallback->OnChanged(mCurrentBrightness);
            mCallback->OnEnd();
            mDimming = false;
        } else {
            DISPLAY_HILOGD(FEAT_BRIGHTNESS, "next step last mCurrentBrightness=%{public}u, next=%{public}d",
                mCurrentBrightness.load(), nextBrightness);
            mCurrentBrightness = nextBrightness;
            mCallback->OnChanged(mCurrentBrightness);
            FFRTTask task = std::bind(&BrightnessDimming::NextStep, this);
            g_animatorTaskHandle = FFRTUtils::SubmitDelayTask(task, mUpdateTime, mQueue);
        }
    } else {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "next step last mCurrentBrightness=%{public}u, mToBrightness=%{public}u",
            mCurrentBrightness.load(), mToBrightness.load());
        mCurrentBrightness = mToBrightness.load();
        mCallback->OnChanged(mCurrentBrightness);
        mCallback->OnEnd();
        mDimming = false;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "animating next step, step=%{public}u, brightness=%{public}u, stride=%{public}d",
        mCurrentStep.load(), mCurrentBrightness.load(), mStride.load());
}
} // namespace DisplayPowerMgr
} // namespace OHOS
