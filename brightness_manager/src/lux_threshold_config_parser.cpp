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

#include "lux_threshold_config_parser.h"

#include <json/json.h>
#include <unistd.h>

#include "config_parser_base.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
const std::string CONFIG_NAME = "brightness_lux_threshold_config";
void SetDefault(LuxThresholdConfig::Data& data)
{
    // default value
    LuxThresholdConfig::Mode config{
        1200, 1500,
        {
            { 0.0f, 5.0f },       { 3.0f, 5.0f },       { 5.0f, 10.0f },        { 10.0f, 20.0f },
            { 20.0f, 50.0f },     { 30.0f, 100.0f },    { 100.0f, 300.0f },     { 500.0f, 500.0f },
            { 1000.0f, 1000.0f }, { 2000.0f, 2000.0f }, { 10000.0f, 10000.0f }, { 70000.0f, 10000.0f }
        },
        {
            { 0.0f, 1.0f },        { 1.0f, 1.0f },      { 3.0f, 2.0f },       { 10.0f, 7.0f },
            { 20.0f, 15.0f },      { 50.0f, 35.0f },    { 100.0f, 80.0f },    { 200.0f, 170.0f },
            { 500.0f, 350.0f },    { 1000.0f, 600.0f }, { 2000.0f, 1200.0f }, { 10000.0f, 8000.0f },
            { 70000.0f, 60000.0f }
        }
    };
    data.modeArray.insert(std::make_pair("DefaultMode", config));
    data.isLevelEnable = true;
    data.brightenPointsForLevel = {
        { 4.0f,   1.0f },
        { 10.0f,  5.0f },
        { 20.0f,  10.0f },
        { 60.0f,  30.0f },
        { 100.0f, 100.0f },
        { 225.0f, 200.0f },
        { 255.0f, 300.0f }
    };
    data.darkenPointsForLevel = {
        { 4.0f,   1.0f },
        { 10.0f,  5.0f },
        { 20.0f,  5.0f },
        { 60.0f,  15.0f },
        { 100.0f, 50.0f },
        { 225.0f, 50.0f },
        { 255.0f, 100.0f }
    };
}
} // namespace

using namespace OHOS::DisplayPowerMgr;

bool LuxThresholdConfigParser::ParseConfig(int displayId, LuxThresholdConfig::Data& data)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse LuxThresholdConfigParser start!", displayId);
    const Json::Value root = ConfigParserBase::Get().LoadConfigRoot(displayId, CONFIG_NAME);
    if (root.isNull()) {
        SetDefault(data);
        return false;
    }
    if (root["isLevelEnable"].isBool()) {
        data.isLevelEnable = root["isLevelEnable"].asBool();
    }
    ConfigParserBase::Get().ParsePointXy(
        root, "brightenPointsForLevel", data.brightenPointsForLevel);
    ConfigParserBase::Get().ParsePointXy(
        root, "darkenPointsForLevel", data.darkenPointsForLevel);
    if (!root["thresholdMode"].isArray()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "root <%{public}s> is not Array!", CONFIG_NAME.c_str());
        return false;
    }
    for (auto value : root["thresholdMode"]) {
        LuxThresholdConfig::Mode config{};
        if (!value["modeName"].isString() || value["modeName"].asString().empty()) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "<%{public}s> modeName is not find!", CONFIG_NAME.c_str());
            continue;
        }
        std::string name = value["modeName"].asString();
        if (value["brightenDebounceTime"].isInt()) {
            config.brightenDebounceTime = value["brightenDebounceTime"].asInt();
        }
        if (value["darkenDebounceTime"].isInt()) {
            config.darkenDebounceTime = value["darkenDebounceTime"].asInt();
        }
        ConfigParserBase::Get().ParsePointXy(value, "brightenPoints", config.brightenPoints);
        ConfigParserBase::Get().ParsePointXy(value, "darkenPoints", config.darkenPoints);
        data.modeArray.insert(std::make_pair(name, config));
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "<%{public}s> is insert!", name.c_str());
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse LuxThresholdConfigParser over!", displayId);
    return true;
}

void LuxThresholdConfigParser::PrintConfig(
    int displayId, const LuxThresholdConfig::Data& data)
{
    std::string text = std::to_string(displayId).append(" ");
    for (auto [key, value] : data.modeArray) {
        text.append("[modeName: ");
        text.append(key).append(", ");
        text.append("brightenDebounceTime: ");
        text.append(std::to_string(value.brightenDebounceTime)).append(", ");
        text.append("darkenDebounceTime: ");
        text.append(std::to_string(value.darkenDebounceTime)).append(", ");
        text.append(ConfigParserBase::Get().PointXyToString("brightenPoints", value.brightenPoints));
        text.append(ConfigParserBase::Get().PointXyToString("darkenPoints", value.brightenPoints));
        text.append("over]");
    }
    text.append(" isLevelEnable: ");
    text.append(data.isLevelEnable ? "true" : "false").append(", ");
    text.append(ConfigParserBase::Get().PointXyToString(
        "brightenPointsForLevel", data.brightenPointsForLevel));
    text.append(ConfigParserBase::Get().PointXyToString("darkenPointsForLevel", data.brightenPointsForLevel));
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "%{public}s", text.c_str());
}
} // namespace DisplayPowerMgr
} // namespace OHOS
