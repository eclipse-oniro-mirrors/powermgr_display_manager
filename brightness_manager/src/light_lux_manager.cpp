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

#include "light_lux_manager.h"

#include <cinttypes>

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
const int LIGHT_LUX_BUFFER_RANGE = 10000;
const int LIGHT_MAX_LUX = 40000;
const int AMBIENT_THRESHOLD_LOWLIGHT = 20;
const int LIGHT_LUX_BUFFER_MIN = 5;
const int LIGHT_LUX_MAX_COLORTEMP = 2000;
const int INVALID_VALUE = -1;
const int LOG_INTERVAL_MS = 2000;
const int LUX_BUFFER_NUM_FOR_LOG = 6;
}

float LightLuxManager::GetLux() const
{
    return mLux;
}

void LightLuxManager::SetLux(float lux)
{
    lux = GetValidLux(lux);
    mLux = lux;
    int64_t currentTime = GetCurrentTimeMillis();
    UpdateLuxBuffer(currentTime, lux);
    IsUpdateLuxSuccess(currentTime);
}

bool LightLuxManager::GetIsFirstLux()
{
    return mIsFirstLux;
}

void LightLuxManager::UpdateLuxBuffer(int64_t timestamp, float lux)
{
    mLuxBuffer.Prune(timestamp - LIGHT_LUX_BUFFER_RANGE);
    mLuxBuffer.Push(timestamp, lux);
}

bool LightLuxManager::IsUpdateLuxSuccess(int64_t timestamp)
{
    float smoothLux = CalcSmoothLux();
    if (smoothLux < 0) {
        return false;
    }

    mSmoothedButNotStabledLux = smoothLux;
    mIsFirstLux = false;
    if (mLuxBuffer.GetSize() == 1) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "fist sensor lux, smoothLux=%{public}f, "
            "timestamp=%{public}" PRId64 ".", smoothLux, timestamp);
        mIsFirstLux = true;
    }
    if (mLuxBuffer.GetSize() < LIGHT_LUX_BUFFER_MIN) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "mLux=%{public}f, smoothLux=%{public}f", mLux, smoothLux);
    }
    mLuxBufferFilter.Prune(timestamp - LIGHT_LUX_BUFFER_RANGE);
    mLuxBufferFilter.Push(timestamp, smoothLux);
    int64_t nextBrightenTime = GetNextBrightenTime(timestamp);
    int64_t nextDarkenTime = GetNextDarkenTime(timestamp);
    PrintCurrentLuxLog(timestamp);

    if (nextBrightenTime <= timestamp || nextDarkenTime <= timestamp || mIsFirstLux) {
        UpdateParam(smoothLux);
        return true;
    }
    return false;
}

float LightLuxManager::CalcSmoothLux() const
{
    auto size = mLuxBuffer.GetSize();
    if (size == 0) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "No ambient light readings available");
        return INVALID_VALUE;
    }
    if (size < LIGHT_LUX_BUFFER_MIN) {
        return mLuxBuffer.GetData(size - 1);
    }

    float sum = 0;
    float luxMin = mLuxBuffer.GetData(size - 1);
    float luxMax = mLuxBuffer.GetData(size - 1);
    for (unsigned int i = size; i >= size - (LIGHT_LUX_BUFFER_MIN - 1); i--) {
        float lux = mLuxBuffer.GetData(i - 1);
        if (luxMin > lux) {
            luxMin = lux;
        }
        if (luxMax < lux) {
            luxMax = lux;
        }
        sum += lux;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "sum - luxMin - luxMax  = %{public}f \n", sum - luxMin - luxMax);
    return (sum - luxMin - luxMax) / 3.0f;
}

int64_t LightLuxManager::GetNextBrightenTime(int64_t timestamp) const
{
    auto size = mLuxBufferFilter.GetSize();
    if (size == 0) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mLuxBufferFilter is empty");
        return timestamp;
    }
    int64_t debounceTime = (size == 1) ? 0 : GetBrightenResponseTime();
    int64_t earliestValidTime = timestamp;
    for (unsigned int i = size; i >= 1; i--) {
        if ((mLuxBufferFilter.GetData(i - 1) - mFilteredLux) < mBrightenDelta) {
            break;
        }
        earliestValidTime = mLuxBufferFilter.GetTime(i - 1);
    }

    return earliestValidTime + debounceTime;
}

int64_t LightLuxManager::GetNextDarkenTime(int64_t timestamp) const
{
    auto size = mLuxBufferFilter.GetSize();
    if (size == 0) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mLuxBufferFilter is empty");
        return timestamp;
    }
    int64_t debounceTime = (size == 1) ? 0 : GetDarkenResponseTime();
    int64_t earliestValidTime = timestamp;
    for (unsigned int i = size; i >= 1; i--) {
        if ((mFilteredLux - mLuxBufferFilter.GetData(i - 1)) < mDarkenDelta) {
            break;
        }
        earliestValidTime = mLuxBufferFilter.GetTime(i - 1);
    }

    return earliestValidTime + debounceTime;
}

void LightLuxManager::UpdateParam(const float lux)
{
    mFilteredLux = lux;
    std::vector<PointXy> brightenLinePointsList = GetBrightenLuxList();
    float delta = CalcDelta(brightenLinePointsList);
    if (delta >= 0) {
        mBrightenDelta = delta;
    }
    std::vector<PointXy> darkenLinePointsList = GetDarkenLuxList();
    delta = CalcDelta(darkenLinePointsList);
    if (delta >= 0) {
        mDarkenDelta = delta;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "mFilteredLux=%{public}f, "
        "bDelta=%{public}f, dDelta=%{public}f", mFilteredLux, mBrightenDelta, mDarkenDelta);
}

float LightLuxManager::CalcDelta(const std::vector<PointXy>& pointsList) const
{
    if (pointsList.empty()) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "error! input vector is empty");
        return INVALID_VALUE;
    }
    float delta = 0;
    PointXy temp = pointsList.front();
    std::vector<PointXy>::const_iterator it;
    for (it = pointsList.begin(); it != pointsList.end(); it++) {
        if (mFilteredLux < it->x) {
            if (it->x < temp.x || IsEqualF(it->x, temp.x)) {
                delta = 1;
            } else {
                delta = (it->y - temp.y) / (it->x - temp.x) * (mFilteredLux - temp.x) + temp.y;
                delta = (delta < 1 ? 1 : delta);
            }
            break;
        } else {
            temp = *it;
            delta = it->y;
        }
    }
    return delta;
}

float LightLuxManager::GetValidLux(float lux) const
{
    if (lux > LIGHT_MAX_LUX) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "error lux = %{public}f > LIGHT_MAX_LUX", lux);
        lux = LIGHT_MAX_LUX;
    }
    if (lux < 0) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "error lux = %{public}f < 0", lux);
        lux = 0;
    }
    return lux;
}

void LightLuxManager::InitParameters()
{
    int displayId = 0;
    const std::unordered_map<int, Config>& brightnessConfig =
        ConfigParse::Get().GetBrightnessConfig();
    std::unordered_map<int, Config>::const_iterator itDisp = brightnessConfig.find(displayId);
    if (itDisp == brightnessConfig.end()) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "[%{public}d]Failed to find config", displayId);
        return;
    }
    mBrightnessConfigData = itDisp->second;
}

void LightLuxManager::SetSceneMode(BrightnessSceneMode mode)
{
    mCurrentSceneMode = mode;
}

float LightLuxManager::GetFilteredLux() const
{
    return mSmoothedButNotStabledLux;
}

float LightLuxManager::GetSmoothedLux() const
{
    return static_cast<int>(mFilteredLux);
}

void LightLuxManager::UpdateSmoothedLux(float lux)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateSmoothedLux mFilteredLux =%{public}f, lux = %{public}f", mFilteredLux, lux);
    mFilteredLux = lux;
}

void LightLuxManager::ClearLuxData()
{
    mLuxBuffer.Clear();
    mLuxBufferFilter.Clear();
    DISPLAY_HILOGE(FEAT_BRIGHTNESS, "ClearLuxData, lux=%{public}f", mFilteredLux);
}

int LightLuxManager::GetDarkenResponseTime() const
{
    auto& mode = GetCurrentModeData();
    int DarkenTime = mode.darkenDebounceTime;
    if (DarkenTime == INVALID_VALUE) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "GetDarkenResponseTime Failed!");
        return 0;
    }
    return DarkenTime;
}

int LightLuxManager::GetBrightenResponseTime() const
{
    auto& mode = GetCurrentModeData();
    int brightenTime = mode.brightenDebounceTime;
    if (brightenTime == INVALID_VALUE) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "GetBrightenResponseTime Failed!");
        return 0;
    }
    return brightenTime;
}

const LuxThresholdConfig::Mode& LightLuxManager::GetCurrentModeData() const
{
    return mBrightnessConfigData.luxThresholdConfig.modeArray.at(ScenceLabel[static_cast<int>(mCurrentSceneMode)]);
}

std::vector<PointXy> LightLuxManager::GetBrightenLuxList()
{
    std::vector<PointXy> brightenPointsList;
    auto& mode = GetCurrentModeData();
    brightenPointsList = mode.brightenPoints;

    return brightenPointsList;
}

std::vector<PointXy> LightLuxManager::GetDarkenLuxList()
{
    std::vector<PointXy> darkenPointsList;
    auto& mode = GetCurrentModeData();
    darkenPointsList = mode.darkenPoints;

    return darkenPointsList;
}

int LightLuxManager::GetFilterNum()
{
    std::string filterName = FilterLabel[static_cast<int>(mCurrentFilter)];
    return mBrightnessConfigData.luxFilterConfig[filterName].filterNum;
}

int LightLuxManager::GetNoFilterNum()
{
    std::string filterName = FilterLabel[static_cast<int>(mCurrentFilter)];
    return mBrightnessConfigData.luxFilterConfig[filterName].filterNoFilterNum;
}

bool LightLuxManager::IsNeedUpdateBrightness(float lux)
{
    float validLux = GetValidLux(lux);
    mLux = validLux;
    int64_t currentTime = GetCurrentTimeMillis();
    UpdateLuxBuffer(currentTime, validLux);
    return IsUpdateLuxSuccess(currentTime);
}

void LightLuxManager::PrintCurrentLuxLog(int64_t time)
{
    if (time - mPrintLogTime > LOG_INTERVAL_MS) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "lux=%{public}s, bDelta=%{public}f, dDelta=%{public}f, size=%{public}d",
            mLuxBufferFilter.ToString(LUX_BUFFER_NUM_FOR_LOG).c_str(),
            mBrightenDelta, mDarkenDelta, mLuxBufferFilter.GetSize());
        mPrintLogTime = time;
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS