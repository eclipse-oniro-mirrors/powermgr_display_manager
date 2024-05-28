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

#include "calculation_curve.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "brightness_base.h"
#include "config_parser.h"
#include "display_log.h"

using namespace std;

namespace OHOS {
namespace DisplayPowerMgr {

using std::string;
using std::vector;

void BrightnessCalculationCurve::InitParameters()
{
    const ScreenConfig screenConfig = ConfigParse::Get().GetScreenConfig();
    mScreenConfig = screenConfig;
    int displayId = 0;
    const std::unordered_map<int, Config>& brightnessConfig =
        ConfigParse::Get().GetBrightnessConfig();
    std::unordered_map<int, Config>::const_iterator itDisp = brightnessConfig.find(displayId);
    if (itDisp == brightnessConfig.end()) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "[%{public}d]Failed to find config", displayId);
        return;
    }
    mCurveConfig = itDisp->second;
}

float BrightnessCalculationCurve::GetCurrentBrightness(float lux)
{
    std::vector<PointXy> curve = mCurveConfig.calculationConfig.defaultPoints;
    if (curve.size() == 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetCurrentBrightness default=%{public}f", mDefaultBrightness);
        return mDefaultBrightness;
    }
    float brightness = GetBrightnessCurveLevel(curve, lux);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetCurrentBrightness brightness=%{public}f", brightness);
    return brightness;
}

float BrightnessCalculationCurve::GetBrightnessCurveLevel(std::vector<PointXy>& linePointsList, float lux)
{
    float brightnessLevel = mDefaultBrightness;
    for (auto point = linePointsList.begin(), prePoint = point; point != linePointsList.end(); point++) {
        if (lux < point->x) {
            if (point->x < prePoint->x || IsEqualF(point->x, prePoint->x)) {
                brightnessLevel = mDefaultBrightness;
            } else {
                brightnessLevel = ((point->y - prePoint->y) / (point->x - prePoint->x) * (lux - prePoint->x))
                    + prePoint->y;
            }
            break;
        } else {
            prePoint = point;
            brightnessLevel = prePoint->y;
        }
    }
    return brightnessLevel;
}

void BrightnessCalculationCurve::UpdateCurveAmbientLux(float lux)
{
    mCurveAmbientLux = lux;
}

void BrightnessCalculationCurve::UpdateCurrentUserId(int userId)
{
    mCurrentUserId = userId;
}

int BrightnessCalculationCurve::GetDisplayIdWithDisplayMode(int displayMode)
{
    std::unordered_map<int, ScreenData> displayModeMap = mScreenConfig.brightnessConfig.displayModeMap;
    if (displayModeMap.find(displayMode) != displayModeMap.end()) {
        return displayModeMap[displayMode].displayId;
    }
    return DEFAULT_DISPLAY_ID;
}

int BrightnessCalculationCurve::GetSensorIdWithDisplayMode(int displayMode)
{
    std::unordered_map<int, ScreenData> displayModeMap = mScreenConfig.brightnessConfig.displayModeMap;
    if (displayModeMap.find(displayMode) != displayModeMap.end()) {
        return displayModeMap[displayMode].sensorId;
    }
    return DEFAULT_SENSOR_ID;
}

int BrightnessCalculationCurve::GetDisplayIdWithFoldstatus(int foldStatus)
{
    std::unordered_map<int, ScreenData> displayModeMap = mScreenConfig.brightnessConfig.foldStatusModeMap;
    if (displayModeMap.find(foldStatus) != displayModeMap.end()) {
        return displayModeMap[foldStatus].displayId;
    }
    return DEFAULT_DISPLAY_ID;
}

int BrightnessCalculationCurve::GetSensorIdWithFoldstatus(int foldStatus)
{
    std::unordered_map<int, ScreenData> displayModeMap = mScreenConfig.brightnessConfig.foldStatusModeMap;
    if (displayModeMap.find(foldStatus) != displayModeMap.end()) {
        return displayModeMap[foldStatus].sensorId;
    }
    return DEFAULT_SENSOR_ID;
}

} // namespace DisplayPowerMgr
} // namespace OHOS