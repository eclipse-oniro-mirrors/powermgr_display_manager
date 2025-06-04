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

void LuxThresholdConfigParser::LuxThresholdParseConfigParams(cJSON* item, LuxThresholdConfig::Data& data)
{
    LuxThresholdConfig::Mode config{};
    std::string name;

    const cJSON* modeNameNode = cJSON_GetObjectItemCaseSensitive(item, "modeName");
    if (modeNameNode && cJSON_IsString(modeNameNode) && modeNameNode->valuestring &&
        strlen(modeNameNode->valuestring) > 0) {
        name = modeNameNode->valuestring;
    } else {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "<%{public}s> modeName is not found!", CONFIG_NAME.c_str());
        return;
    }

    const cJSON* brightenDebounceTimeNode = cJSON_GetObjectItemCaseSensitive(item, "brightenDebounceTime");
    if (brightenDebounceTimeNode && cJSON_IsNumber(brightenDebounceTimeNode)) {
        config.brightenDebounceTime = brightenDebounceTimeNode->valueint;
    }

    const cJSON* darkenDebounceTimeNode = cJSON_GetObjectItemCaseSensitive(item, "darkenDebounceTime");
    if (darkenDebounceTimeNode && cJSON_IsNumber(darkenDebounceTimeNode)) {
        config.darkenDebounceTime = darkenDebounceTimeNode->valueint;
    }

    ConfigParserBase::Get().ParsePointXy(item, "brightenPoints", config.brightenPoints);
    ConfigParserBase::Get().ParsePointXy(item, "darkenPoints", config.darkenPoints);

    data.modeArray.insert(std::make_pair(name, config));
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "<%{public}s> is insert!", name.c_str());
}

bool LuxThresholdConfigParser::ParseConfig(int displayId, LuxThresholdConfig::Data& data)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse LuxThresholdConfigParser start!", displayId);
    const cJSON* root = ConfigParserBase::Get().LoadConfigRoot(displayId, CONFIG_NAME);
    if (!root) {
        SetDefault(data);
        return false;
    }
    if (!cJSON_IsObject(root)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "root is not an object!");
        cJSON_Delete(const_cast<cJSON*>(root));
        return false;
    }
    const cJSON* isLevelEnableNode = cJSON_GetObjectItemCaseSensitive(root, "isLevelEnable");
    if (isLevelEnableNode && cJSON_IsBool(isLevelEnableNode)) {
        data.isLevelEnable = cJSON_IsTrue(isLevelEnableNode);
    }

    ConfigParserBase::Get().ParsePointXy(
        root, "brightenPointsForLevel", data.brightenPointsForLevel);
    ConfigParserBase::Get().ParsePointXy(
        root, "darkenPointsForLevel", data.darkenPointsForLevel);

    const cJSON* thresholdModeArray = cJSON_GetObjectItemCaseSensitive(root, "thresholdMode");
    if (!thresholdModeArray || !cJSON_IsArray(thresholdModeArray)) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "root <%{public}s> is not Array!", CONFIG_NAME.c_str());
        cJSON_Delete(const_cast<cJSON*>(root));
        return false;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, thresholdModeArray) {
        if (!cJSON_IsObject(item)) {
            continue;
        }
        LuxThresholdParseConfigParams(item, data);
    }

    cJSON_Delete(const_cast<cJSON*>(root));
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
