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
#include "display_setting_helper.h"

#include "display_log.h"
#include "setting_provider.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr int32_t AUTO_BRIGHTNESS_DISABLE = 0;
constexpr int32_t AUTO_BRIGHTNESS_ENABLE = 1;
}
using namespace OHOS::PowerMgr;
sptr<SettingObserver> DisplaySettingHelper::autoBrightnessObserver_;
sptr<SettingObserver> DisplaySettingHelper::brightnessObserver_;

void DisplaySettingHelper::RegisterSettingBrightnessObserver(SettingObserver::UpdateFunc func)
{
    if (brightnessObserver_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "setting brightness observer is already registered");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    brightnessObserver_ = provider.CreateObserver(SETTING_BRIGHTNESS_KEY, func);
    ErrCode ret = provider.RegisterObserver(brightnessObserver_);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "register setting brightness observer failed, ret=%{public}d", ret);
        brightnessObserver_ = nullptr;
    }
}

void DisplaySettingHelper::UnregisterSettingBrightnessObserver()
{
    if (brightnessObserver_ == nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "brightnessObserver_ is nullptr, no need to unregister");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    ErrCode ret = provider.UnregisterObserver(brightnessObserver_);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "unregister setting brightness observer failed, ret=%{public}d", ret);
    }
    brightnessObserver_ = nullptr;
}

void DisplaySettingHelper::SetSettingBrightness(uint32_t value)
{
    uint32_t settingBrightness;
    if (GetSettingBrightness(settingBrightness) != ERR_OK) {
        return;
    }
    if (value == static_cast<uint32_t>(settingBrightness)) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "no need to set setting brightness");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    auto ret = provider.PutIntValue(SETTING_BRIGHTNESS_KEY, static_cast<int32_t>(value));
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "set setting brightness failed, ret=%{public}d", ret);
        return;
    }
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "set setting brightness=%{public}u", value);
}

ErrCode DisplaySettingHelper::GetSettingBrightness(uint32_t& brightness, const std::string& key)
{
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    int32_t value;
    ErrCode ret = provider.GetIntValue(key, value);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "get setting brightness failed, ret=%{public}d", ret);
        return ret;
    }
    brightness = static_cast<uint32_t>(value);
    return ERR_OK;
}

void DisplaySettingHelper::RegisterSettingAutoBrightnessObserver(SettingObserver::UpdateFunc func)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Register setting auto brightness observer start");
    if (autoBrightnessObserver_) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "setting auto brightness observer is already registered");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    autoBrightnessObserver_ = provider.CreateObserver(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, func);
    ErrCode ret = provider.RegisterObserver(autoBrightnessObserver_);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "register setting auto brightness observer failed, ret=%{public}d", ret);
        autoBrightnessObserver_ = nullptr;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Register setting auto brightness observer end");
}
void DisplaySettingHelper::UnregisterSettingAutoBrightnessObserver()
{
    if (autoBrightnessObserver_ == nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "autoBrightnessObserver_ is nullptr, no need to unregister");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    ErrCode ret = provider.UnregisterObserver(autoBrightnessObserver_);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "unregister setting auto brightness observer failed, ret=%{public}d", ret);
    }
    autoBrightnessObserver_ = nullptr;
}

void DisplaySettingHelper::SetSettingAutoBrightness(bool enable)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetSettingAutoBrightness mode, enable=%{public}d", enable);
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    int32_t value = enable ? AUTO_BRIGHTNESS_ENABLE : AUTO_BRIGHTNESS_DISABLE;
    ErrCode ret = provider.PutIntValue(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, value);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(
            FEAT_BRIGHTNESS, "set setting auto brightness failed, enable=%{public}d, ret=%{public}d", enable, ret);
    }
}
bool DisplaySettingHelper::GetSettingAutoBrightness(const std::string& key)
{
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    int32_t value;
    ErrCode ret = provider.GetIntValue(key, value);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(
            FEAT_BRIGHTNESS, "get setting auto brightness failed key=%{public}s, ret=%{public}d", key.c_str(), ret);
    }
    return (value == AUTO_BRIGHTNESS_ENABLE);
}
} // namespace DisplayPowerMgr
} // namespace OHOS