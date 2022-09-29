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

#ifndef DISPLAY_AUTO_BRIGHTNESS_H
#define DISPLAY_AUTO_BRIGHTNESS_H

#include <cinttypes>

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayAutoBrightness {
public:
    static uint32_t CalculateAutoBrightness(float scalar);

private:
    enum TransformNodeType {
        TRANSFORM_NODE_TYPE_START = 0,
        TRANSFORM_NODE_TYPE_FIRST,
        TRANSFORM_NODE_TYPE_SECOND,
        TRANSFORM_NODE_TYPE_LAST,
        TRANSFORM_NODE_TYPE_END
    };

    struct TransformNode {
        float lux;
        uint32_t brightness;
    };

    static const float LUX_MIN;
    static const float LUX_MAX;
    static const uint32_t BRIGHTNESS_MIN;
    static const uint32_t BRIGHTNESS_MAX;
    static const float TRANSFORM_NODE_LUX_START;
    static const float TRANSFORM_NODE_LUX_FIRST;
    static const float TRANSFORM_NODE_LUX_SECOND;
    static const float TRANSFORM_NODE_BRIGHTNESS_FIRST_PER;
    static const float TRANSFORM_NODE_BRIGHTNESS_SECOND_PER;
    static const uint32_t TRANSFORM_NODE_BRIGHTNESS_FIRST;
    static const uint32_t TRANSFORM_NODE_BRIGHTNESS_SECOND;
    static const TransformNode TRANSFORM_NODE_TABLE[TRANSFORM_NODE_TYPE_END];
};
} // namespace PowerMgr
} // namespace OHOS
#endif // DISPLAY_AUTO_BRIGHTNESS_H