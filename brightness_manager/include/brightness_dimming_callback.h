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

#ifndef BRRIGHTNESS_DIMMING_CALLBACK_H
#define BRRIGHTNESS_DIMMING_CALLBACK_H

#include <cstdint>

namespace OHOS {
namespace DisplayPowerMgr {
class BrightnessDimmingCallback {
public:
    BrightnessDimmingCallback() = default;
    virtual ~BrightnessDimmingCallback() = default;
    BrightnessDimmingCallback(const BrightnessDimmingCallback&) = delete;
    BrightnessDimmingCallback& operator=(const BrightnessDimmingCallback&) = delete;
    BrightnessDimmingCallback(BrightnessDimmingCallback&&) = delete;
    BrightnessDimmingCallback& operator=(BrightnessDimmingCallback&&) = delete;
    
    virtual void OnStart() = 0;
    virtual void OnChanged(uint32_t currentValue) = 0;
    virtual void OnEnd() = 0;
    virtual void DiscountBrightness(double discount) = 0;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // BRRIGHTNESS_DIMMING_CALLBACK_H