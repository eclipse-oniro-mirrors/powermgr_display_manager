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

#include "brightness_setting_helper.h"

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
sptr<SettingObserver> BrightnessSettingHelper::mAutoBrightnessObserver;
sptr<SettingObserver> BrightnessSettingHelper::mBrightnessObserver;

void BrightnessSettingHelper::RegisterSettingBrightnessObserver(SettingObserver::UpdateFunc func)
{
    if (mBrightnessObserver) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "setting brightness observer is already registered");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    mBrightnessObserver = provider.CreateObserver(SETTING_BRIGHTNESS_KEY, func);
    ErrCode ret = provider.RegisterObserver(mBrightnessObserver);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "register setting brightness observer failed, ret=%{public}d", ret);
        mBrightnessObserver = nullptr;
    }
}

void BrightnessSettingHelper::UnregisterSettingBrightnessObserver()
{
    if (mBrightnessObserver == nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "mBrightnessObserver is nullptr, no need to unregister");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    ErrCode ret = provider.UnregisterObserver(mBrightnessObserver);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "unregister setting brightness observer failed, ret=%{public}d", ret);
    }
    mBrightnessObserver = nullptr;
}

void BrightnessSettingHelper::SetSettingBrightness(uint32_t value)
{
    uint32_t settingBrightness;
    if (GetSettingBrightness(settingBrightness) != ERR_OK) {
        return;
    }
    if (value == static_cast<uint32_t>(settingBrightness)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "no need to set setting brightness");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    auto ret = provider.PutIntValue(SETTING_BRIGHTNESS_KEY, static_cast<int32_t>(value));
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "set setting brightness failed, ret=%{public}d", ret);
        return;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "set setting brightness=%{public}u", value);
}

ErrCode BrightnessSettingHelper::GetSettingBrightness(uint32_t& brightness, const std::string& key)
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

void BrightnessSettingHelper::RegisterSettingAutoBrightnessObserver(SettingObserver::UpdateFunc func)
{
    if (mAutoBrightnessObserver) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "setting auto brightness observer is already registered");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    mAutoBrightnessObserver = provider.CreateObserver(SETTING_AUTO_ADJUST_BRIGHTNESS_KEY, func);
    ErrCode ret = provider.RegisterObserver(mAutoBrightnessObserver);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "register setting auto brightness observer failed, ret=%{public}d", ret);
        mAutoBrightnessObserver = nullptr;
    }
}

void BrightnessSettingHelper::UnregisterSettingAutoBrightnessObserver()
{
    if (mAutoBrightnessObserver == nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "mAutoBrightnessObserver is nullptr, no need to unregister");
        return;
    }
    SettingProvider& provider = SettingProvider::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    ErrCode ret = provider.UnregisterObserver(mAutoBrightnessObserver);
    if (ret != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "unregister setting auto brightness observer failed, ret=%{public}d", ret);
    }
    mAutoBrightnessObserver = nullptr;
}

void BrightnessSettingHelper::SetSettingAutoBrightness(bool enable)
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

bool BrightnessSettingHelper::GetSettingAutoBrightness(const std::string& key)
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