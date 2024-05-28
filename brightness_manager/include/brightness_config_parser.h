/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef BRIGHTNESS_CONFIG_PARSER_H
#define BRIGHTNESS_CONFIG_PARSER_H

#include <unordered_map>

namespace OHOS {
namespace DisplayPowerMgr {
struct ScreenData {
    int displayId{0};
    int sensorId{0};  // SENSOR_TYPE_ID_AMBIENT_LIGHT = 5, SENSOR_TYPE_ID_AMBIENT_LIGHT1 = 16
};

namespace BrightnessConfig {
struct Data {
    std::unordered_map<int, ScreenData> displayModeMap { // UNKNOWN = 0, FULL = 1, MAIN = 2, SUB = 3, COORDINATION = 4
        { 0, {0, 16}},
        { 1, {0, 16}},
        { 2, {5, 5}}
    };
    std::unordered_map<int, ScreenData> foldStatusModeMap { // UNKNOWN = 0, EXPAND = 1,  FOLDED = 2,  HALF_FOLD = 3;
        { 0, { 0, 16 } },
        { 1, { 0, 16 } },
        { 2, { 5, 5 } },
        { 3, { 0, 16 } }
    };
};
}
class BrightnessConfigParser {
public:
    BrightnessConfigParser() = default;
    virtual ~BrightnessConfigParser() = default;
    BrightnessConfigParser(const BrightnessConfigParser&) = delete;
    BrightnessConfigParser& operator=(const BrightnessConfigParser&) = delete;
    BrightnessConfigParser(BrightnessConfigParser&&) = delete;
    BrightnessConfigParser& operator=(BrightnessConfigParser&&) = delete;

    static bool ParseConfig(BrightnessConfig::Data& data);
    static void PrintConfig(const BrightnessConfig::Data& data);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // BRIGHTNESS_CONFIG_PARSER_H
