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

#ifndef LUX_FILTER_CONFIG_PARSER_H
#define LUX_FILTER_CONFIG_PARSER_H

#include <string>
#include <unordered_map>
#include <cJSON.h>

namespace OHOS {
namespace DisplayPowerMgr {
namespace LuxFilterConfig {
struct Data {
    int filterNoFilterNum{-1};
    int filterNum{-1};
    int filterMaxFuncLuxNum{-1};
    float filterAlpha{-1.0f};
    int filterLuxTh{-1};
};
}
class LuxFilterConfigParser {
public:
    LuxFilterConfigParser() = default;
    virtual ~LuxFilterConfigParser() = default;
    LuxFilterConfigParser(const LuxFilterConfigParser&) = delete;
    LuxFilterConfigParser& operator=(const LuxFilterConfigParser&) = delete;
    LuxFilterConfigParser(LuxFilterConfigParser&&) = delete;
    LuxFilterConfigParser& operator=(LuxFilterConfigParser&&) = delete;

    static void LuxFilterParseConfigParams(
        cJSON* item, std::unordered_map<std::string, LuxFilterConfig::Data>& data);
    static bool ParseConfig(
        int displayId, std::unordered_map<std::string, LuxFilterConfig::Data>& data);
    static bool ParseConfigJsonRoot(
        const std::string& fileContent, std::unordered_map<std::string, LuxFilterConfig::Data>& data);
    static void PrintConfig(
        int displayId, const std::unordered_map<std::string, LuxFilterConfig::Data>& data);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // LUX_FILTER_CONFIG_PARSER_H
