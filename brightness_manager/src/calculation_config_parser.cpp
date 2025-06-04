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

#include "calculation_config_parser.h"

#include <unistd.h>
#include <cJSON.h>

#include "config_parser_base.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
const std::string CONFIG_NAME = "brightness_curve_config";
} // namespace

using namespace OHOS::DisplayPowerMgr;

bool CalculationConfigParser::ParseConfig(int displayId, CalculationConfig::Data& data)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse CalculationConfig start!", displayId);
    const cJSON* root = ConfigParserBase::Get().LoadConfigRoot(displayId, CONFIG_NAME);
    if (!root) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse CalculationConfig error!", displayId);
        return false;
    }
    if (!cJSON_IsObject(root)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "root is not an object!");
        cJSON_Delete(const_cast<cJSON*>(root));
        return false;
    }

    const cJSON* defaultBrightnessNode = cJSON_GetObjectItemCaseSensitive(root, "defaultBrightness");
    if (defaultBrightnessNode && cJSON_IsNumber(defaultBrightnessNode)) {
        data.defaultBrightness = static_cast<float>(defaultBrightnessNode->valuedouble);
    } else {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d] parse defaultBrightness error!", displayId);
    }

    ConfigParserBase::Get().ParsePointXy(root, "defaultPoints", data.defaultPoints);

    cJSON_Delete(const_cast<cJSON*>(root));
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] parse CalculationConfig over!", displayId);
    return true;
}

void CalculationConfigParser::PrintConfig(int displayId, const CalculationConfig::Data& data)
{
    std::string text = std::to_string(displayId);
    text.append(" defaultBrightness: ");
    text.append(std::to_string(data.defaultBrightness)).append(", ");
    text.append(ConfigParserBase::Get().PointXyToString("defaultPoints", data.defaultPoints));
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "%{public}s", text.c_str());
}
} // namespace DisplayPowerMgr
} // namespace OHOS
