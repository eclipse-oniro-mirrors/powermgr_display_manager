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

#ifndef BRIGHTNESS_CALCULATION_CURVE_H
#define BRIGHTNESS_CALCULATION_CURVE_H

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "brightness_base.h"
#include "config_parser.h"

namespace OHOS {
namespace DisplayPowerMgr {
class BrightnessCalculationCurve {
public:
    BrightnessCalculationCurve() = default;
    BrightnessCalculationCurve(const BrightnessCalculationCurve&) = delete;
    BrightnessCalculationCurve& operator=(const BrightnessCalculationCurve&) = delete;
    BrightnessCalculationCurve(BrightnessCalculationCurve&&) = delete;
    BrightnessCalculationCurve& operator=(BrightnessCalculationCurve&&) = delete;

    void InitParameters();
    float GetCurrentBrightness(float lux);
    void UpdateCurveAmbientLux(float lux);
    void UpdateCurrentUserId(int userId);
private:
    float GetBrightnessCurveLevel(std::vector<PointXy>& linePointsList, float lux);

    Config mCurveConfig{};
    uint32_t mDisplayId {0};
    float mDefaultBrightness {100.0f};
    float mCurveAmbientLux {0.0f};
    int mCurrentUserId {0};
};

} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // BRIGHTNESS_CALCULATION_CURVE_H

