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

#ifndef CALCULATION_CONFIG_PARSER_H
#define CALCULATION_CONFIG_PARSER_H

#include <vector>

namespace OHOS {
namespace DisplayPowerMgr {
struct PointXy {
    float x{-1.0f};
    float y{-1.0f};
};
struct PointXyz {
    float x{-1.0f};
    float y{-1.0f};
    float z{-1.0f};
};
namespace CalculationConfig {
struct Data {
    // default value
    float defaultBrightness{35.0f};
    std::vector<PointXy> defaultPoints{
        { 0.0f, 5.0f },     { 5.0f, 17.0f },     { 20.0f, 30.0f },
        { 50.0f, 36.0f },   { 200.0f, 45.0f },   { 400.0f, 54.0f },
        { 1000.0f, 81.0f }, { 2000.0f, 123.0f }, { 7000.0f, 224.0f }
    };
};
}
class CalculationConfigParser {
public:
    CalculationConfigParser() = default;
    virtual ~CalculationConfigParser() = default;
    CalculationConfigParser(const CalculationConfigParser&) = delete;
    CalculationConfigParser& operator=(const CalculationConfigParser&) = delete;
    CalculationConfigParser(CalculationConfigParser&&) = delete;
    CalculationConfigParser& operator=(CalculationConfigParser&&) = delete;

    static bool ParseConfig(int displayId, CalculationConfig::Data& data);
    static void PrintConfig(int displayId, const CalculationConfig::Data& data);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // CALCULATION_CONFIG_PARSER_H
