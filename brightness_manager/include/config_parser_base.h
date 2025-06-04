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

#ifndef CONFIG_PARSER_BASE_H
#define CONFIG_PARSER_BASE_H

#include <mutex>
#include <optional>
#include <string>

#include <cJSON.h>

#include "brightness_config_parser.h"
#include "calculation_config_parser.h"

namespace OHOS {
namespace DisplayPowerMgr {
class ConfigParserBase {
public:
    ConfigParserBase(const ConfigParserBase&) = delete;
    ConfigParserBase& operator=(const ConfigParserBase&) = delete;
    ConfigParserBase(ConfigParserBase&&) = delete;
    ConfigParserBase& operator=(ConfigParserBase&&) = delete;

    static ConfigParserBase& Get();

    void Initialize();
    const std::string LoadConfigPath(int displayId, const std::string& configName) const;
    const cJSON* LoadConfigRoot(int displayId, const std::string& configName) const;
    void ParsePointXy(const cJSON* root, const std::string& name, std::vector<PointXy>& data) const;
    const std::string PointXyToString(const std::string& name, const std::vector<PointXy>& data) const;
    void ParseScreenData(const cJSON* root, const std::string& name, std::unordered_map<int, ScreenData>& data,
       const std::string paramName) const;
    const std::string ScreenDataToString(const std::string& name, const std::unordered_map<int, ScreenData>& data,
       const std::string paramName) const;

private:
    struct ConfigInfo {
        std::string panelName{};
        std::string panelVersion{};
    };

    ConfigParserBase() = default;
    virtual ~ConfigParserBase() = default;

    std::unordered_map<int, ConfigInfo>::iterator GetDispIter(int displayId);
    std::unordered_map<int, ConfigInfo>::const_iterator GetConstDispIter(int displayId) const;

    mutable std::mutex mLock{};
    // Guarded by mLock begin
    std::atomic<bool> mIsInitialized{false};
    std::unordered_map<int, ConfigInfo> mConfigInfo{};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // CONFIG_PARSER_BASE_H
