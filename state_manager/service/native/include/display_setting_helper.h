/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef POWERMGR_DISPLAY_MANAGER_DISPLAY_SETTING_HELPER_H
#define POWERMGR_DISPLAY_MANAGER_DISPLAY_SETTING_HELPER_H

#include <cstdint>
#include <string>

#include "setting_observer.h"

namespace OHOS {
namespace DisplayPowerMgr {
using namespace OHOS::PowerMgr;
class DisplaySettingHelper {
public:
    static void RegisterSettingBrightnessObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingBrightnessObserver();
    static void SetSettingBrightness(uint32_t value);
    static ErrCode GetSettingBrightness(uint32_t& brightness, const std::string& key = SETTING_BRIGHTNESS_KEY);

    static void RegisterSettingAutoBrightnessObserver(SettingObserver::UpdateFunc func);
    static void UnregisterSettingAutoBrightnessObserver();
    static void SetSettingAutoBrightness(bool enable);
    static bool GetSettingAutoBrightness(const std::string& key = SETTING_AUTO_ADJUST_BRIGHTNESS_KEY);
private:
    static const constexpr char* SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
    static const constexpr char* SETTING_AUTO_ADJUST_BRIGHTNESS_KEY {"settings.display.auto_screen_brightness"};

    static sptr<SettingObserver> autoBrightnessObserver_;
    static sptr<SettingObserver> brightnessObserver_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_DISPLAY_MANAGER_DISPLAY_SETTING_HELPER_H