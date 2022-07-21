/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_DISPLAY_MANAGER_DISPLAY_PARAM_HELPER_H
#define POWERMGR_DISPLAY_MANAGER_DISPLAY_PARAM_HELPER_H

#include <singleton.h>
#include <cstdint>
#include <iosfwd>
#include <string>

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayParamHelper : public DelayedRefSingleton<DisplayParamHelper> {
    DECLARE_DELAYED_REF_SINGLETON(DisplayParamHelper);
public:
    uint32_t GetDefaultBrightness();
    uint32_t GetMaxBrightness();
    uint32_t GetMinBrightness();

private:
    const std::string KEY_DEFAULT_BRIGHTNESS = "const.display.brightness.default";
    const std::string KEY_MAX_BRIGHTNESS = "const.display.brightness.max";
    const std::string KEY_MIN_BRIGHTNESS = "const.display.brightness.min";
    static constexpr uint32_t BRIGHTNESS_MIN = 1;
    static constexpr uint32_t BRIGHTNESS_DEFAULT = 102;
    static constexpr uint32_t BRIGHTNESS_MAX = 255;
    static constexpr int32_t VALUE_MAX_LEN = 32;

    int32_t QueryIntValue(const std::string& key, int32_t def);
};
} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // POWERMGR_DISPLAY_MANAGER_DISPLAY_PARAM_HELPER_H
