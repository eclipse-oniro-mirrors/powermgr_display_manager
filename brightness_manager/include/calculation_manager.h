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

#ifndef BRIGHTNESS_CALCULATION_MANAGER_H
#define BRIGHTNESS_CALCULATION_MANAGER_H

#include <memory>
#include <mutex>
#include <vector>

#include "calculation_curve.h"

namespace OHOS {
namespace DisplayPowerMgr {

class BrightnessCalculationManager {
public:
    BrightnessCalculationManager() = default;
    virtual ~BrightnessCalculationManager() = default;;
    BrightnessCalculationManager(const BrightnessCalculationManager&) = delete;
    BrightnessCalculationManager& operator=(const BrightnessCalculationManager&) = delete;
    BrightnessCalculationManager(BrightnessCalculationManager&&) = delete;
    BrightnessCalculationManager& operator=(BrightnessCalculationManager&&) = delete;
    void InitParameters();
    float GetInterpolatedValue(float lux);
    void UpdateCurrentUserId(int userId);
    void UpdateBrightnessOffset(float posBrightness, float lux);
    void SetGameModeEnable(bool isGameCurveEnable);
    void SetCameraModeEnable(bool isCameraCurveEnable);
    void ResetOffsetFromHumanFactor(bool isOffsetResetEnable, int minOffsetBrightness, int maxOffsetBrightness);

private:
    void UpdateParaFromReboot();
    void UpdateCurveAmbientLux(float lux);
    float GetInterpolatedBrightenssLevel(float positionBrightness, float lux);
    void UpdateDefaultBrightness(float lux);
    float GetCurrentBrightness(float lux);

    bool IsDefaultBrightnessMode(float positionBrightness);
    float GetAmbientOffsetLux();
    float GetBrightenOffsetNoValidBrightenLux(float lux);

    float GetOffsetLevel(float brightnessStartOrig, float brightnessEndOrig,
        float brightnessStartNew, float delta);
    float GetDefaultBrightenOffsetBrightenRaio(float brightnessStartOrig, float brightnessEndOrig,
        float brightnessStartNew, float deltaStart);
    float GetDefaultDarkenOffsetBrightenRatio(float brightnessStartOrig, float brightnessEndOrig,
        float brightnessStartNew, float deltaStart);
    float GetDefaultDarkenOffsetDarkenRatio(float brightnessStartOrig, float brightnessEndOrig,
        float brightnessStartNew, float deltaStart);
    float GetDefaultBrightenOffsetDarkenRatio(float brightnessStartOrig, float brightnessEndOrig,
        float brightnessStartNew);
    void ResetDefaultBrightnessOffset();

    BrightnessCalculationCurve mBrightnessCalculationCurve{};
    uint32_t mDisplayId;
    int mCurrentUserId;
    bool mIsCameraCurveEnable{false};
    bool mIsGameCurveEnable{false};
    float mDefaultBrightness{false};
    float mDelta{0.0f};
    float mOffsetLux{0.0f};
    float mPosBrightness{0.0f};
    bool mIsReboot{false};
    float mOffsetBrightnessLast{0.0f};
    float mLastLuxDefaultBrightness{0.0f};
    float mStartLuxDefaultBrightness{0.0f};
    float mCurveLux{0.0f};
    float mDefaultBrightnessFromLux{0.0f};
    float offsetBrightenAlphaRight{0.0f};
    float offsetDarkenAlphaLeft{0.0f};
    float offsetBrightenAlphaLeft{0.0f};
    float offsetBrightenRatioLeft{0.0f};
};

} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // BRIGHTNESS_CALCULATION_MANAGER_H
