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

#ifndef BRIGHTNESS_BASE_H
#define BRIGHTNESS_BASE_H

#include <ctime>
#include <math.h>
#include <string>

namespace OHOS {
namespace DisplayPowerMgr {

static constexpr uint32_t DEFAULT_DISPLAY_ID = 0;
static constexpr uint32_t OUTTER_SCREEN_DISPLAY_ID = 5;
static const float EPSILON = 0.0000001f;
static const int MSECPERSEC = 1000;
static const int NSECPERMSEC = 1000000;

enum class BrightnessFilterMode {
    MEAN_FILTER = 0,
    WEIGHT_FILTER,
    FITLER_END
};

enum class BrightnessSceneMode {
    MODE_DEFAULT = 0,
    MODE_GAME,
    MODE_VIDEO,
    SCENCE_END
};

enum class BrightnessModeState {
    CAMERA_MODE,
    GAME_MODE,
    DEFAULT_MODE
};

static std::string ScenceLabel[static_cast<int>(BrightnessSceneMode::SCENCE_END)] = {
    "DefaultMode", "GameMode", "VideoMode" };
static std::string FilterLabel[static_cast<int>(BrightnessFilterMode::FITLER_END)] = {
    "meanFilter", "weightFilter" };

static inline int64_t GetCurrentTimeMillis()
{
    struct timespec val {};
    if (clock_gettime(CLOCK_BOOTTIME, &val) == 0) {
        return static_cast<int64_t>(val.tv_sec) * MSECPERSEC + val.tv_nsec / NSECPERMSEC;
    }
    return 0;
}

static bool IsEqualF(float a, float b)
{
    return fabs(a - b) < EPSILON;
}
} // namespace BrightnessPowerMgr
} // namespace OHOS
#endif // BRIGHTNESS_BASE_H