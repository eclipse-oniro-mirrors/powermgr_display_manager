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

#include "lux_filter_config_parser.h"

#include <unistd.h>

#include "config_parser_base.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
const std::string CONFIG_NAME = "brightness_lux_filter_method_config";
void SetDefault(std::unordered_map<std::string, LuxFilterConfig::Data>& data)
{
    LuxFilterConfig::Data config{ 7, 5, -1, -1.0f, -1 };
    data.insert(std::make_pair("meanFilter", config));
}
} // namespace

using namespace OHOS::DisplayPowerMgr;

void LuxFilterConfigParser::LuxFilterParseConfigParams(
    cJSON* item, std::unordered_map<std::string, LuxFilterConfig::Data>& data)
{
    LuxFilterConfig::Data config{};
    std::string name;
    const cJSON* filterNameNode = cJSON_GetObjectItemCaseSensitive(item, "filterName");
    if (DisplayJsonUtils::IsValidJsonStringAndNoEmpty(filterNameNode)) {
        name = filterNameNode->valuestring;
    } else {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "<%{public}s> filterName is not found!", CONFIG_NAME.c_str());
        return;
    }
    const cJSON* filterNoFilterNumNode = cJSON_GetObjectItemCaseSensitive(item, "filterNoFilterNum");
    if (DisplayJsonUtils::IsValidJsonNumber(filterNoFilterNumNode)) {
        config.filterNoFilterNum = filterNoFilterNumNode->valueint;
    }
    const cJSON* filterNumNode = cJSON_GetObjectItemCaseSensitive(item, "filterNum");
    if (DisplayJsonUtils::IsValidJsonNumber(filterNumNode)) {
        config.filterNum = filterNumNode->valueint;
    }
    const cJSON* filterMaxFuncLuxNumNode = cJSON_GetObjectItemCaseSensitive(item, "filterMaxFuncLuxNum");
    if (DisplayJsonUtils::IsValidJsonNumber(filterMaxFuncLuxNumNode)) {
        config.filterMaxFuncLuxNum = filterMaxFuncLuxNumNode->valueint;
    }
    const cJSON* filterAlphaNode = cJSON_GetObjectItemCaseSensitive(item, "filterAlpha");
    if (DisplayJsonUtils::IsValidJsonNumber(filterAlphaNode)) {
        config.filterAlpha = static_cast<float>(filterAlphaNode->valuedouble);
    }
    const cJSON* filterLuxThNode = cJSON_GetObjectItemCaseSensitive(item, "filterLuxTh");
    if (DisplayJsonUtils::IsValidJsonNumber(filterLuxThNode)) {
        config.filterLuxTh = filterLuxThNode->valueint;
    }
    data.insert(std::make_pair(name, config));
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "<%{public}s> is insert!", name.c_str());
}

bool LuxFilterConfigParser::ParseConfig(
    int displayId, std::unordered_map<std::string, LuxFilterConfig::Data>& data)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse LuxFilterConfig start!", displayId);
    const std::string fileContent = ConfigParserBase::Get().LoadConfigRoot(displayId, CONFIG_NAME);
    if (!ParseConfigJsonRoot(fileContent, data)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse LuxFilterConfig error!", displayId);
        return false;
    }

    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse LuxFilterConfig over!", displayId);
    return true;
}

bool LuxFilterConfigParser::ParseConfigJsonRoot(
    const std::string& fileContent, std::unordered_map<std::string, LuxFilterConfig::Data>& data)
{
    const cJSON* root = cJSON_Parse(fileContent.c_str());
    if (!root) {
        SetDefault(data);
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "Parse file %{public}s failure.", fileContent.c_str());
        return false;
    }
    if (!cJSON_IsArray(root)) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "root <%{public}s> is not Array!", CONFIG_NAME.c_str());
        cJSON_Delete(const_cast<cJSON*>(root));
        return false;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        if (!cJSON_IsObject(item)) {
            continue;
        }
        LuxFilterParseConfigParams(item, data);
    }
    cJSON_Delete(const_cast<cJSON*>(root));
    return true;
}

void LuxFilterConfigParser::PrintConfig(
    int displayId, const std::unordered_map<std::string, LuxFilterConfig::Data>& data)
{
    std::string text = std::to_string(displayId).append(" ");
    for (auto [key, value] : data) {
        text.append("[filterName: ");
        text.append(key).append(", ");
        text.append("filterNoFilterNum: ");
        text.append(std::to_string(value.filterNoFilterNum)).append(", ");
        text.append("filterNum: ");
        text.append(std::to_string(value.filterNum)).append(", ");
        text.append("filterMaxFuncLuxNum: ");
        text.append(std::to_string(value.filterMaxFuncLuxNum)).append(", ");
        text.append("filterAlpha: ");
        text.append(std::to_string(value.filterAlpha)).append(", ");
        text.append("filterLuxTh: ");
        text.append(std::to_string(value.filterLuxTh)).append("]");
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "%{public}s", text.c_str());
}
} // namespace DisplayPowerMgr
} // namespace OHOS
