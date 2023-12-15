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

#ifndef LUX_HTRESHOLD_CONFIG_PARSER_H
#define LUX_HTRESHOLD_CONFIG_PARSER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "calculation_config_parser.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace LuxThresholdConfig {
struct Mode {
    int brightenDebounceTime{-1};
    int darkenDebounceTime{-1};
    std::vector<PointXy> brightenPoints{};
    std::vector<PointXy> darkenPoints{};
};
struct Data {
    std::unordered_map<std::string, LuxThresholdConfig::Mode> modeArray{};
    bool isLevelEnable{false};
    std::vector<PointXy> brightenPointsForLevel{};
    std::vector<PointXy> darkenPointsForLevel{};
};
}
class LuxThresholdConfigParser {
public:
    LuxThresholdConfigParser() = default;
    virtual ~LuxThresholdConfigParser() = default;
    LuxThresholdConfigParser(const LuxThresholdConfigParser&) = delete;
    LuxThresholdConfigParser& operator=(const LuxThresholdConfigParser&) = delete;
    LuxThresholdConfigParser(LuxThresholdConfigParser&&) = delete;
    LuxThresholdConfigParser& operator=(LuxThresholdConfigParser&&) = delete;

    static bool ParseConfig(int displayId, LuxThresholdConfig::Data& data);
    static void PrintConfig(int displayId, const LuxThresholdConfig::Data& data);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // LUX_HTRESHOLD_CONFIG_PARSER_H
