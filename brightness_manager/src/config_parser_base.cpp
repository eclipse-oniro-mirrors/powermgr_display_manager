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

#include "config_parser_base.h"

#include <fstream>
#include <iostream>
#include <unistd.h>

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr int DISPLAY_ID_MAX = 5;
constexpr uint16_t POINT_XY_SIZE = 2;
constexpr int POINT_X_INDEX = 0;
constexpr int POINT_Y_INDEX = 1;
const std::string CONFIG_PATH_FOR_ROOT = "/sys_prod/etc/display/";
const std::string CONFIG_PATH_TYP = ".json";
const std::string CONFIG_PATHS[DISPLAY_ID_MAX] = {
    "brightness_config/", // Unkonwn config, default path
    "full/brightness_config/", // Full config
    "brightness_config/", // Main config
    "sub/brightness_config/", // Sub config
    "brightness_config/", // Others config
};
} // namespace

using namespace OHOS::DisplayPowerMgr;

ConfigParserBase& ConfigParserBase::Get()
{
    static ConfigParserBase brightnessConfigParserBase;
    return brightnessConfigParserBase;
}

void ConfigParserBase::Initialize()
{
    std::lock_guard<std::mutex> lock(mLock);
    if (mIsInitialized.load()) [[unlikely]]
    {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Already init!");
        return;
    }
    for (int displayId = 0; displayId < DISPLAY_ID_MAX; displayId++) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] Already init!", displayId);
        mConfigInfo[displayId] = ConfigInfo{};
    }
    mIsInitialized = true;
}

std::unordered_map<int, ConfigParserBase::ConfigInfo>::iterator ConfigParserBase::GetDispIter(int displayId)
{
    std::unordered_map<int, ConfigInfo>::iterator itDisp = mConfigInfo.find(displayId);
    if (itDisp == mConfigInfo.end()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d]Failed to find config", displayId);
    }
    return itDisp;
}

std::unordered_map<int, ConfigParserBase::ConfigInfo>::const_iterator ConfigParserBase::GetConstDispIter(
    int displayId) const
{
    std::unordered_map<int, ConfigInfo>::const_iterator itDisp = mConfigInfo.find(displayId);
    if (itDisp == mConfigInfo.end()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d]Failed to find config", displayId);
    }
    return itDisp;
}

const std::string ConfigParserBase::LoadConfigPath(int displayId, const std::string& configName) const
{
    auto itDisp = GetConstDispIter(displayId);
    std::string configPath{};
    configPath.append(CONFIG_PATH_FOR_ROOT).append(CONFIG_PATHS[displayId]).append(configName);
    // default path
    if (itDisp == mConfigInfo.end()) {
        configPath.append(CONFIG_PATH_TYP);
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d] default path [%{public}s]!", displayId, configPath.c_str());
        return configPath;
    }

    // default path
    const ConfigInfo& configInfo = itDisp->second;
    if (configInfo.panelName.empty()) {
        configPath.append(CONFIG_PATH_TYP);
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d] default path [%{public}s]!", displayId, configPath.c_str());
        return configPath;
    }

    // name + version path
    configPath.append("_").append(configInfo.panelName);
    if (!configInfo.panelVersion.empty()) {
        configPath = configPath.append("_").append(configInfo.panelVersion).append(CONFIG_PATH_TYP);
        if (access(configPath.c_str(), R_OK) == 0) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d] name + version path [%{public}s]!",
                displayId, configPath.c_str());
            return configPath;
        }
    }

    // version path
    configPath.append(CONFIG_PATH_TYP);
    if (access(configPath.c_str(), R_OK) == 0) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d] version path [%{public}s]!", displayId, configPath.c_str());
        return configPath;
    }

    // default path
    configPath.clear();
    configPath.append(CONFIG_PATH_FOR_ROOT).append(CONFIG_PATHS[displayId]).append(configName).append(CONFIG_PATH_TYP);
    DISPLAY_HILOGW(FEAT_BRIGHTNESS, "[%{public}d] default path [%{public}s]!", displayId, configPath.c_str());
    return configPath;
}

const cJSON* ConfigParserBase::LoadConfigRoot(int displayId, const std::string& configName) const
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "[%{public}d] LoadConfigRoot [%{public}s]!", displayId, configName.c_str());
    const std::string configPath = LoadConfigPath(displayId, configName);
    std::ifstream fileStream(configPath, std::ios::in | std::ios::binary);
    if (!fileStream) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "Open file %{public}s failure.", configName.c_str());
        return nullptr;
    }

    std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    fileStream.close();
    cJSON* root = cJSON_Parse(fileContent.c_str());
    if (!root) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "Parse file %{public}s failure.", configName.c_str());
        return nullptr;
    }
    return root;
}

void ConfigParserBase::ParsePointXy(
    const cJSON* root, const std::string& name, std::vector<PointXy>& data) const
{
    data.clear();
    const cJSON* array = cJSON_GetObjectItemCaseSensitive(root, name.c_str());
    if (!array || !cJSON_IsArray(array)) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "root <%{public}s> is not found or is not an array!", name.c_str());
        return;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, array) {
        if (!cJSON_IsArray(item)) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "array <%{public}s> element is not an array!", name.c_str());
            return;
        }

        PointXy pointXy{};
        uint32_t arraySize = (uint32_t)cJSON_GetArraySize(item);
        if (arraySize != POINT_XY_SIZE) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "array <%{public}s> size!=%{public}d!", name.c_str(), POINT_XY_SIZE);
            return;
        }

        const cJSON* xNode = cJSON_GetArrayItem(item, POINT_X_INDEX);
        if (xNode && cJSON_IsNumber(xNode)) {
            pointXy.x = static_cast<float>(xNode->valuedouble);
        } else {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "parse [%{public}s] error!", name.c_str());
        }

        const cJSON* yNode = cJSON_GetArrayItem(item, POINT_Y_INDEX);
        if (yNode && cJSON_IsNumber(yNode)) {
            pointXy.y = static_cast<float>(yNode->valuedouble);
        } else {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "parse [%{public}s] error!", name.c_str());
        }

        data.emplace_back(pointXy);
    }
}

const std::string ConfigParserBase::PointXyToString(
    const std::string& name, const std::vector<PointXy>& data) const
{
    std::string text{};
    text.append(name).append(": ");
    for (auto value : data) {
        text.append(std::to_string(value.x)).append(" ").append(std::to_string(value.y)).append(", ");
    }
    return text;
}


void ConfigParserBase::ParseScreenData(const cJSON* root, const std::string& name,
    std::unordered_map<int, ScreenData>& data, const std::string paramName) const
{
    data.clear();
    const cJSON* array = cJSON_GetObjectItemCaseSensitive(root, name.c_str());
    if (!array || !cJSON_IsArray(array)) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "root <%{public}s> is not found or is not an array!", name.c_str());
        return;
    }

    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, array) {
        if (!cJSON_IsObject(item)) {
            continue;
        }
        ScreenData screenData{};
        int displayMode = 0;
        const cJSON* displayModeNode = cJSON_GetObjectItemCaseSensitive(item, paramName.c_str());
        if (displayModeNode && cJSON_IsNumber(displayModeNode)) {
            displayMode = displayModeNode->valueint;
        }

        const cJSON* displayIdNode = cJSON_GetObjectItemCaseSensitive(item, "displayId");
        if (displayIdNode && cJSON_IsNumber(displayIdNode)) {
            screenData.displayId = displayIdNode->valueint;
        }

        const cJSON* sensorIdNode = cJSON_GetObjectItemCaseSensitive(item, "sensorId");
        if (sensorIdNode && cJSON_IsNumber(sensorIdNode)) {
            screenData.sensorId = sensorIdNode->valueint;
        }

        data[displayMode] = screenData;
    }
}

const std::string ConfigParserBase::ScreenDataToString(const std::string& name,
    const std::unordered_map<int, ScreenData>& data, const std::string paramName) const
{
    std::string text{};
    text.append(name).append(": ");
    for (const auto& [key, value] : data) {
        text.append(paramName).append(": ").append(std::to_string(key)).append(" ");
        text.append("displayId").append(": ").append(std::to_string(value.displayId)).append(" ");
        text.append("sensorId").append(": ").append(std::to_string(value.sensorId)).append(" ");
    }
    return text;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
