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

#ifndef LIGHT_LUX_MANAGER_H
#define LIGHT_LUX_MANAGER_H

#include <vector>

#include "config_parser.h"
#include "ilight_lux_manager.h"
#include "light_lux_buffer.h"

namespace OHOS {
namespace DisplayPowerMgr {
class LightLuxManager : public ILightLuxManager {
public:
    LightLuxManager() = default;
    ~LightLuxManager() override = default;

    LightLuxManager(const LightLuxManager&) = delete;
    LightLuxManager& operator=(const LightLuxManager&) = delete;
    LightLuxManager(LightLuxManager&&) = delete;
    LightLuxManager& operator=(LightLuxManager&&) = delete;

    void InitParameters() override;
    void SetSceneMode(BrightnessSceneMode mode) override;
    float GetFilteredLux() const override;
    float GetSmoothedLux() const override;
    void ClearLuxData() override;
    void UpdateSmoothedLux(float lux);
    bool IsNeedUpdateBrightness(float lux) override;

    float GetLux() const override;
    void SetLux(const float lux) override;
    bool GetIsFirstLux();

private:
    void UpdateLuxBuffer(int64_t timestamp, float lux);
    bool IsUpdateLuxSuccess(int64_t timestamp);
    float CalcSmoothLux() const;
    int64_t GetNextBrightenTime(int64_t timestamp) const;
    int64_t GetNextDarkenTime(int64_t timestamp) const;
    void UpdateParam(float lux);
    float CalcDelta(const std::vector<PointXy>& pointList) const;
    float GetValidLux(float lux) const;
    int GetDarkenResponseTime() const;
    int GetBrightenResponseTime() const;
    const LuxThresholdConfig::Mode& GetCurrentModeData() const;
    std::vector<PointXy> GetBrightenLuxList();
    std::vector<PointXy> GetDarkenLuxList();
    int GetFilterNum();
    int GetNoFilterNum();
    void PrintCurrentLuxLog(int64_t timestamp);

    LightLuxBuffer mLuxBuffer{};
    LightLuxBuffer mLuxBufferFilter{};
    float mLux{0.0f};
    float mFilteredLux{0.0f};
    bool mIsFirstLux{false};
    float mSmoothedButNotStabledLux{0.0f};
    float mBrightenDelta{120.0f};
    float mDarkenDelta{110.0f};
    int64_t mPrintLogTime{0};
    BrightnessFilterMode mCurrentFilter{static_cast<int>(BrightnessFilterMode::MEAN_FILTER)};
    BrightnessSceneMode mCurrentSceneMode{static_cast<int>(BrightnessSceneMode::MODE_DEFAULT)};
    Config mBrightnessConfigData{};
};
} // namespace BrightnessPowerMgr
} // namespace OHOS
#endif // LIGHT_LUX_MANAGER_H