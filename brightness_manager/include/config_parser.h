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

#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <mutex>

#include "calculation_config_parser.h"
#include "lux_filter_config_parser.h"
#include "lux_threshold_config_parser.h"
#include "brightness_config_parser.h"

namespace OHOS {
namespace DisplayPowerMgr {
struct Config {
    CalculationConfig::Data calculationConfig{};
    std::unordered_map<std::string, LuxFilterConfig::Data> luxFilterConfig{};
    LuxThresholdConfig::Data luxThresholdConfig{};
};
struct ScreenConfig {
    BrightnessConfig::Data brightnessConfig{};
};
class ConfigParse {
public:
    ConfigParse(const ConfigParse&) = delete;
    ConfigParse& operator=(const ConfigParse&) = delete;
    ConfigParse(ConfigParse&&) = delete;
    ConfigParse& operator=(ConfigParse&&) = delete;

    static ConfigParse& Get();

    void Initialize();
    const std::unordered_map<int, Config>& GetBrightnessConfig() const;
    const ScreenConfig& GetScreenConfig() const;

private:
    ConfigParse() = default;
    virtual ~ConfigParse() = default;

    bool ParseConfig(int displayId, Config& data) const;
    void PrintConfig(int displayId, const Config& data) const;

    mutable std::mutex mLock{};
    // Guarded by mLock begin
    std::atomic<bool> mIsInitialized{false};
    std::unordered_map<int, Config> mConfig{};
    ScreenConfig mScreenConfig{};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // CONFIG_PARSER_H
