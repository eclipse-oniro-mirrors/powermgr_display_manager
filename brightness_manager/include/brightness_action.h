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

#ifndef BRIGHTNESS_ACTION_H
#define BRIGHTNESS_ACTION_H

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "brightness_base.h"
#include "display_power_info.h"

namespace OHOS {
namespace DisplayPowerMgr {
class BrightnessAction {
public:
    BrightnessAction(uint32_t displayId);
    ~BrightnessAction() = default;
    BrightnessAction(const BrightnessAction&) = delete;
    BrightnessAction& operator=(const BrightnessAction&) = delete;
    BrightnessAction(BrightnessAction&&) = delete;
    BrightnessAction& operator=(BrightnessAction&&) = delete;

    static uint32_t GetDefaultDisplayId();
    static std::vector<uint32_t> GetAllDisplayId();

    uint32_t GetDisplayId();
    void SetDisplayId(uint32_t displayId);
    DisplayState GetDisplayState();
    bool SetDisplayState(DisplayState state, const std::function<void(DisplayState)>& callback);
    bool SetDisplayPower(DisplayState state, uint32_t reason);
    uint32_t GetBrightness();
    bool SetBrightness(uint32_t value);
    bool SetBrightness(uint32_t displayId, uint32_t value);

private:
    std::mutex mMutexBrightness;
    uint32_t mBrightness {102};
    uint32_t mDisplayId {DEFAULT_DISPLAY_ID};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // BRIGHTNESS_ACTION_H
