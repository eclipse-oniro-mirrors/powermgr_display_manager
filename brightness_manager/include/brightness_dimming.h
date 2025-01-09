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

#ifndef BRRIGHTNESS_DIMMING_H
#define BRRIGHTNESS_DIMMING_H

#include <atomic>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "brightness_dimming_callback.h"
#include "brightness_ffrt.h"

namespace OHOS {
namespace DisplayPowerMgr {
class BrightnessDimming : public std::enable_shared_from_this<BrightnessDimming> {
public:
    BrightnessDimming(const std::string& name, std::shared_ptr<BrightnessDimmingCallback>& callback);
    virtual ~BrightnessDimming() = default;
    BrightnessDimming(const BrightnessDimming&) = delete;
    BrightnessDimming& operator=(const BrightnessDimming&) = delete;
    BrightnessDimming(BrightnessDimming&&) = delete;
    BrightnessDimming& operator=(BrightnessDimming&&) = delete;

    void StartDimming(uint32_t from, uint32_t to, uint32_t duration);
    void StopDimming();
    bool IsDimming() const;
    uint32_t GetDimmingUpdateTime() const;
    bool Init();
    void Reset();
private:
    static const uint32_t DEFAULT_UPDATE_TIME = 32;
    static const int32_t STRIDE_ABSOLUTE_MIN = 1;

    void NextStep();

    std::string mName{};
    std::shared_ptr<BrightnessDimmingCallback> mCallback{};
    std::atomic_bool mDimming{};
    std::atomic_uint32_t mFromBrightness{};
    std::atomic_uint32_t mToBrightness{};
    std::atomic_uint32_t mDuration{};
    std::atomic_uint32_t mUpdateTime{};
    std::atomic_uint32_t mTotalSteps{};
    std::atomic_int32_t mStride{};
    std::atomic_uint32_t mCurrentBrightness{};
    std::atomic_uint32_t mCurrentStep{};
    std::shared_ptr<PowerMgr::FFRTQueue> mQueue;
    std::mutex mAnimatorHandleLock{};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // BRRIGHTNESS_DIMMING_H