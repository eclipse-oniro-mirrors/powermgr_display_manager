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

#ifndef ILIGHT_LUX_MANAGER_H
#define ILIGHT_LUX_MANAGER_H

#include "brightness_base.h"

namespace OHOS {
namespace DisplayPowerMgr {

class ILightLuxManager {
public:
    virtual ~ILightLuxManager() = default;

    virtual void InitParameters() = 0;
    virtual void SetSceneMode(BrightnessSceneMode mode) = 0;
    virtual float GetFilteredLux() const = 0;
    virtual float GetSmoothedLux() const = 0;
    virtual void ClearLuxData() = 0;
    virtual bool IsNeedUpdateBrightness(float lux) = 0;
    virtual float GetLux() const = 0;
    virtual void SetLux(const float lux) = 0;
};

} // namespace BrightnessPowerMgr
} // namespace OHOS
#endif // LIGHT_LUX_MANAGER_H