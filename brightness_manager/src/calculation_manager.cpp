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

#include "calculation_manager.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {

namespace {
constexpr float DEFAULT_NO_OFFSET_LUX = -1.0f;
constexpr float DEFAULT_OFFSET_BRIGHTNESS = 0.0f;
constexpr float DEFAULT_OFFSET_DELTA = 0.0f;
constexpr float SMALL_VALUE = 1e-6f;
constexpr float DEFAULT_OFFSET_RATIO = 1.0f;
constexpr float MIN_OFFSET_RATIO = 0.0f;
[[maybe_unused]] constexpr float DEFAULT_MIN_DELTA = 1.0f;
constexpr float MIN_DEFAULT_BRIGHTNESS = 4.0f;
constexpr float MAX_DEFAULT_BRIGHTNESS = 255.0f;

constexpr float AMBIENT_VALID_MAX_LUX = 40000;
constexpr float AMBIENT_VALID_MIN_LUX = 0.0f;
[[maybe_unused]] constexpr float DEFAULT_BRIGHTNESS = 100.0f;
} // namespace

void BrightnessCalculationManager::InitParameters()
{
    mBrightnessCalculationCurve.InitParameters();
}

float BrightnessCalculationManager::GetInterpolatedValue(float lux)
{
    float valueInterp = GetInterpolatedBrightenssLevel(mPosBrightness, lux) / MAX_DEFAULT_BRIGHTNESS;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetInterpolatedValue lux=%{public}f, valueInterp=%{public}f, "\
        " mPosBrightness=%{public}f", lux, valueInterp, mPosBrightness);
    return valueInterp;
}

float BrightnessCalculationManager::GetInterpolatedBrightenssLevel(float positionBrightness, float lux)
{
    float posBrightness = positionBrightness;
    UpdateCurveAmbientLux(lux);
    UpdateDefaultBrightness(lux);

    if (mLastLuxDefaultBrightness <= DEFAULT_OFFSET_BRIGHTNESS && mPosBrightness != DEFAULT_OFFSET_BRIGHTNESS) {
        posBrightness = DEFAULT_OFFSET_BRIGHTNESS;
        ResetDefaultBrightnessOffset();
    }

    float offsetBrightness = mDefaultBrightnessFromLux;
    if (IsDefaultBrightnessMode(posBrightness)) {
        offsetBrightness = mDefaultBrightnessFromLux;
    } else {
        offsetBrightness = GetOffsetLevel(mStartLuxDefaultBrightness,
            mDefaultBrightnessFromLux, posBrightness, mDelta);
    }

    mLastLuxDefaultBrightness = mDefaultBrightnessFromLux;
    mOffsetBrightnessLast = offsetBrightness;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetLevel lux=%{public}f, offsetBrightness=%{public}f, default=%{public}f",
        lux, offsetBrightness, mDefaultBrightnessFromLux);
    return offsetBrightness;
}

void BrightnessCalculationManager::UpdateDefaultBrightness(float lux)
{
    float defaultBrightness = GetCurrentBrightness(lux);
    mDefaultBrightnessFromLux = defaultBrightness;
}

float BrightnessCalculationManager::GetCurrentBrightness(float lux)
{
    return mBrightnessCalculationCurve.GetCurrentBrightness(lux);
}

void BrightnessCalculationManager::UpdateParaFromReboot()
{
    mLastLuxDefaultBrightness = mDefaultBrightnessFromLux;
    mStartLuxDefaultBrightness = mDefaultBrightnessFromLux;
    mOffsetBrightnessLast = mDefaultBrightnessFromLux;
    mIsReboot = false;
}

bool BrightnessCalculationManager::IsDefaultBrightnessMode(float positionBrightness)
{
    if (positionBrightness == DEFAULT_OFFSET_DELTA) {
        return true;
    }
    return false;
}

float BrightnessCalculationManager::GetOffsetLevel(float brightnessStartOrig, float brightnessEndOrig,
    float brightnessStartNew, float delta)
{
    float brightenRatio = 1.0f;
    float darkenRatio = 1.0f;
    float deltaStart = delta;
    if (brightnessStartOrig < brightnessEndOrig) {
        if (deltaStart > DEFAULT_OFFSET_DELTA) {
            darkenRatio = GetDefaultBrightenOffsetBrightenRaio(brightnessStartOrig, brightnessEndOrig,
                brightnessStartNew, deltaStart);
        }
        if (deltaStart < DEFAULT_OFFSET_DELTA) {
            brightenRatio = GetDefaultDarkenOffsetBrightenRatio(brightnessStartOrig, brightnessEndOrig,
                brightnessStartNew, deltaStart);
        }
    }
    if (brightnessStartOrig > brightnessEndOrig) {
        if (deltaStart < DEFAULT_OFFSET_DELTA) {
            darkenRatio = GetDefaultDarkenOffsetDarkenRatio(brightnessStartOrig, brightnessEndOrig,
                brightnessStartNew, deltaStart);
        }
        if (deltaStart > DEFAULT_OFFSET_DELTA) {
            brightenRatio = GetDefaultBrightenOffsetDarkenRatio(brightnessStartOrig, brightnessEndOrig,
                brightnessStartNew);
        }
    }
    float tempDeltaNew = deltaStart * brightenRatio * darkenRatio;

    float brightnessAndDelta = brightnessEndOrig + tempDeltaNew;
    float offsetBrightnessTemp = (brightnessAndDelta > MIN_DEFAULT_BRIGHTNESS
        ? brightnessAndDelta : MIN_DEFAULT_BRIGHTNESS);
    float offsetBrightness = (offsetBrightnessTemp < MAX_DEFAULT_BRIGHTNESS
        ? offsetBrightnessTemp : MAX_DEFAULT_BRIGHTNESS);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetOffsetLevel tempDeltaNew=%{public}f, deltaStart=%{public}f, "\
        " brightenRatio=%{public}f, darkenRatio=%{public}f", tempDeltaNew, deltaStart, brightenRatio, darkenRatio);
    return offsetBrightness;
}

float BrightnessCalculationManager::GetDefaultBrightenOffsetBrightenRaio(float brightnessStartOrig,
    float brightnessEndOrig, float brightnessStartNew, float deltaStart)
{
    float brightenRatio;
    if (std::abs(MAX_DEFAULT_BRIGHTNESS - brightnessStartOrig) < SMALL_VALUE) {
        brightenRatio = DEFAULT_OFFSET_RATIO;
    } else {
        brightenRatio = (MAX_DEFAULT_BRIGHTNESS - brightnessEndOrig)
            / (MAX_DEFAULT_BRIGHTNESS - brightnessStartOrig);
    }

    float offsetBrightnessTmp = ((1 - offsetBrightenAlphaRight)
        * std::max(brightnessEndOrig, brightnessStartNew))
        + (offsetBrightenAlphaRight * ((deltaStart * brightenRatio) + brightnessEndOrig));
    if (std::abs(deltaStart) < SMALL_VALUE) {
        brightenRatio = DEFAULT_OFFSET_RATIO;
    } else {
        brightenRatio = (offsetBrightnessTmp - brightnessEndOrig) / deltaStart;
    }
    if (brightenRatio < MIN_OFFSET_RATIO) {
        brightenRatio = MIN_OFFSET_RATIO;
    }
    return brightenRatio;
}


float BrightnessCalculationManager::GetAmbientOffsetLux()
{
    float lux = static_cast<int>(mOffsetLux);
    return lux;
}

float BrightnessCalculationManager::GetBrightenOffsetNoValidBrightenLux(float lux)
{
    float noValidBrightenLuxTh = AMBIENT_VALID_MAX_LUX;
    return noValidBrightenLuxTh;
}

float BrightnessCalculationManager::GetDefaultDarkenOffsetBrightenRatio(float brightnessStartOrig,
    float brightnessEndOrig, float brightnessStartNew, float deltaStart)
{
    float brightenRatio;
    if (std::abs(MAX_DEFAULT_BRIGHTNESS - brightnessStartOrig) < SMALL_VALUE) {
        brightenRatio = DEFAULT_OFFSET_RATIO;
    } else {
        brightenRatio = (MAX_DEFAULT_BRIGHTNESS - brightnessEndOrig)
            / (MAX_DEFAULT_BRIGHTNESS - brightnessStartOrig);
    }

    if (brightenRatio < MIN_OFFSET_RATIO) {
        brightenRatio = MIN_OFFSET_RATIO;
    }
    return brightenRatio;
}

float BrightnessCalculationManager::GetDefaultDarkenOffsetDarkenRatio(float brightnessStartOrig,
    float brightnessEndOrig, float brightnessStartNew, float deltaStart)
{
    float darkenRatio;
    if (std::abs(brightnessStartOrig - MIN_DEFAULT_BRIGHTNESS) < SMALL_VALUE) {
        darkenRatio = DEFAULT_OFFSET_RATIO;
    } else {
        darkenRatio = (brightnessEndOrig - MIN_DEFAULT_BRIGHTNESS)
            / (brightnessStartOrig - MIN_DEFAULT_BRIGHTNESS);
    }
    float offsetBrightnessTmp = ((1 - offsetDarkenAlphaLeft) * std::min(brightnessEndOrig,
        brightnessStartNew)) + (offsetDarkenAlphaLeft * ((deltaStart * darkenRatio) + brightnessEndOrig));
    if (std::abs(deltaStart) < SMALL_VALUE) {
        darkenRatio = DEFAULT_OFFSET_RATIO;
    } else {
        darkenRatio = (offsetBrightnessTmp - brightnessEndOrig) / deltaStart;
    }
    if (darkenRatio < MIN_OFFSET_RATIO) {
        darkenRatio = MIN_OFFSET_RATIO;
    }
    return darkenRatio;
}

float BrightnessCalculationManager::GetDefaultBrightenOffsetDarkenRatio(float brightnessStartOrig,
    float brightnessEndOrig, float brightnessStartNew)
{
    float darkenRatio;
    if (std::abs(brightnessStartOrig) < SMALL_VALUE) {
        darkenRatio = DEFAULT_OFFSET_RATIO;
    } else {
        float darkenRatioTmp = static_cast<float>(pow(brightnessEndOrig / brightnessStartOrig,
            offsetBrightenRatioLeft));
        darkenRatio = (offsetBrightenAlphaLeft * brightnessEndOrig / brightnessStartOrig)
            + ((1 - offsetBrightenAlphaLeft) * darkenRatioTmp);
    }
    return darkenRatio;
}

void BrightnessCalculationManager::UpdateBrightnessOffset(float posBrightness, float lux)
{
    if (lux < AMBIENT_VALID_MIN_LUX || lux > AMBIENT_VALID_MAX_LUX) {
        return;
    }
    if (std::abs(posBrightness) < SMALL_VALUE) {
        ResetDefaultBrightnessOffset();
        return;
    }
    mOffsetLux = lux;
    mStartLuxDefaultBrightness = GetCurrentBrightness(lux);
    mPosBrightness = posBrightness;
    mDelta = mPosBrightness - mStartLuxDefaultBrightness;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightnessOffset lux=%{public}f, mDelta=%{public}f, "\
     "mPosBrightness=%{public}f", lux, mDelta, mPosBrightness);
}

void BrightnessCalculationManager::ResetDefaultBrightnessOffset()
{
    mOffsetLux = DEFAULT_NO_OFFSET_LUX;
    mDelta = DEFAULT_OFFSET_DELTA;

    mOffsetBrightnessLast = DEFAULT_OFFSET_BRIGHTNESS;
    mLastLuxDefaultBrightness = DEFAULT_OFFSET_BRIGHTNESS;
    mStartLuxDefaultBrightness = DEFAULT_OFFSET_BRIGHTNESS;
    mPosBrightness = DEFAULT_OFFSET_BRIGHTNESS;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ResetDefaultBrightnessOffset");
}

void BrightnessCalculationManager::UpdateCurveAmbientLux(float lux)
{
    if (mCurveLux != lux) {
        mBrightnessCalculationCurve.UpdateCurveAmbientLux(lux);
        mCurveLux = lux;
    }
}

void BrightnessCalculationManager::SetGameModeEnable(bool isGameCurveEnable)
{
    mIsGameCurveEnable = isGameCurveEnable;
}

void BrightnessCalculationManager::SetCameraModeEnable(bool isCameraCurveEnable)
{
    mIsCameraCurveEnable = isCameraCurveEnable;
}

void BrightnessCalculationManager::UpdateCurrentUserId(int userId)
{
    mCurrentUserId = userId;
    mBrightnessCalculationCurve.UpdateCurrentUserId(userId);
}

void BrightnessCalculationManager::ResetOffsetFromHumanFactor(bool isOffsetResetEnable, int minOffsetBrightness,
    int maxOffsetBrightness)
{
    if (isOffsetResetEnable && std::abs(mPosBrightness) > SMALL_VALUE) {
        if (mPosBrightness < minOffsetBrightness) {
            mPosBrightness = minOffsetBrightness;
            mOffsetBrightnessLast = minOffsetBrightness;
            mDelta = mPosBrightness - mStartLuxDefaultBrightness;
        }
        if (mPosBrightness > maxOffsetBrightness) {
            mPosBrightness = maxOffsetBrightness;
            mOffsetBrightnessLast = maxOffsetBrightness;
            mDelta = mPosBrightness - mStartLuxDefaultBrightness;
        }
    }
}

int BrightnessCalculationManager::GetDisplayIdWithDisplayMode(int displayMode)
{
    return mBrightnessCalculationCurve.GetDisplayIdWithDisplayMode(displayMode);
}

int BrightnessCalculationManager::GetSensorIdWithDisplayMode(int displayMode)
{
    return mBrightnessCalculationCurve.GetSensorIdWithDisplayMode(displayMode);
}

int BrightnessCalculationManager::GetDisplayIdWithFoldstatus(int foldStatus)
{
    return mBrightnessCalculationCurve.GetDisplayIdWithFoldstatus(foldStatus);
}

int BrightnessCalculationManager::GetSensorIdWithFoldstatus(int foldStatus)
{
    return mBrightnessCalculationCurve.GetSensorIdWithFoldstatus(foldStatus);
}
} // namespace DisplayPowerMgr
} // namespace OHOS