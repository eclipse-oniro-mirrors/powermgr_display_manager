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

#include "config_parser.h"

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr int DISPLAY_ID_MAX = 5;
const std::string CONFIG_NAME = "brightness_lux_threshold_config";
} // namespace

using namespace OHOS::DisplayPowerMgr;

ConfigParse& ConfigParse::Get()
{
    static ConfigParse ConfigParse;
    return ConfigParse;
}

void ConfigParse::Initialize()
{
    std::lock_guard<std::mutex> lock(mLock);
    if (mIsInitialized.load()) [[unlikely]] {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Already init!");
        return;
    }
    for (int displayId = 0; displayId < DISPLAY_ID_MAX; displayId++) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] Already init!", displayId);
        Config brightnessConfig{};
        ParseConfig(displayId, brightnessConfig);
        PrintConfig(displayId, brightnessConfig);
        mConfig[displayId] = brightnessConfig;
    }
    mIsInitialized = true;
}

const std::unordered_map<int, Config>& ConfigParse::GetBrightnessConfig() const
{
    return mConfig;
}

bool ConfigParse::ParseConfig(int displayId, Config& data) const
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse Config start!", displayId);
    CalculationConfigParser::ParseConfig(displayId, data.calculationConfig);
    LuxFilterConfigParser::ParseConfig(displayId, data.luxFilterConfig);
    LuxThresholdConfigParser::ParseConfig(displayId, data.luxThresholdConfig);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse Config over!", displayId);
    return true;
}

void ConfigParse::PrintConfig(int displayId, const Config& data) const
{
    CalculationConfigParser::PrintConfig(displayId, data.calculationConfig);
    LuxFilterConfigParser::PrintConfig(displayId, data.luxFilterConfig);
    LuxThresholdConfigParser::PrintConfig(displayId, data.luxThresholdConfig);
}
} // namespace DisplayPowerMgr
} // namespace OHOS
