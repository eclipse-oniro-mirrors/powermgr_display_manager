/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "display_auto_brightness.h"

#include "display_param_helper.h"

namespace OHOS {
namespace DisplayPowerMgr {
const float DisplayAutoBrightness::LUX_MIN = 0.0;
const float DisplayAutoBrightness::LUX_MAX = 10000.0;
const uint32_t DisplayAutoBrightness::BRIGHTNESS_MIN = DisplayParamHelper::GetMinBrightness();
const uint32_t DisplayAutoBrightness::BRIGHTNESS_MAX = DisplayParamHelper::GetMaxBrightness();
const float DisplayAutoBrightness::TRANSFORM_NODE_LUX_START = 0.0;
const float DisplayAutoBrightness::TRANSFORM_NODE_LUX_FIRST = 100.0;
const float DisplayAutoBrightness::TRANSFORM_NODE_LUX_SECOND = 500.0;
// 0.2 is the ratio of brightness value 51 to 255
const float DisplayAutoBrightness::TRANSFORM_NODE_BRIGHTNESS_FIRST_PER = 0.2;
// 0.47 is the ratio of brightness value 120 to 255
const float DisplayAutoBrightness::TRANSFORM_NODE_BRIGHTNESS_SECOND_PER = 0.47;
const uint32_t DisplayAutoBrightness::TRANSFORM_NODE_BRIGHTNESS_FIRST = static_cast<uint32_t>(BRIGHTNESS_MIN +
    TRANSFORM_NODE_BRIGHTNESS_FIRST_PER * (BRIGHTNESS_MAX - BRIGHTNESS_MIN));
const uint32_t DisplayAutoBrightness::TRANSFORM_NODE_BRIGHTNESS_SECOND = static_cast<uint32_t>(BRIGHTNESS_MIN +
    TRANSFORM_NODE_BRIGHTNESS_SECOND_PER * (BRIGHTNESS_MAX - BRIGHTNESS_MIN));

const DisplayAutoBrightness::TransformNode DisplayAutoBrightness::TRANSFORM_NODE_TABLE[TRANSFORM_NODE_TYPE_END] = {
    {TRANSFORM_NODE_LUX_START, BRIGHTNESS_MIN},
    {TRANSFORM_NODE_LUX_FIRST, TRANSFORM_NODE_BRIGHTNESS_FIRST},
    {TRANSFORM_NODE_LUX_SECOND, TRANSFORM_NODE_BRIGHTNESS_SECOND},
    {LUX_MAX, BRIGHTNESS_MAX},
};

uint32_t DisplayAutoBrightness::CalculateAutoBrightness(float scalar)
{
    uint32_t brightness = BRIGHTNESS_MAX;
    if (scalar >= LUX_MAX) {
        return brightness;
    }
    for (uint8_t count = 1; count < TRANSFORM_NODE_TYPE_END; count++) {
        auto lastNode = TRANSFORM_NODE_TABLE[count - 1];
        auto curNode = TRANSFORM_NODE_TABLE[count];
        if (lastNode.lux <= scalar && scalar < curNode.lux) {
            brightness = static_cast<uint32_t>(lastNode.brightness + (scalar - lastNode.lux) *
                ((curNode.brightness - lastNode.brightness) / (curNode.lux - lastNode.lux)));
            return brightness;
        }
    }
    return brightness;
}
} // namespace PowerMgr
} // namespace OHOS