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

#include "brightness_manager.h"

namespace OHOS {
namespace DisplayPowerMgr {
BrightnessManager& BrightnessManager::Get()
{
    static BrightnessManager brightnessManager;
    return brightnessManager;
}

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
std::shared_ptr<BrightnessManagerExt> BrightnessManager::GetExt(uint32_t screenId) const
{
    std::shared_lock<std::shared_mutex> lock(mMutex);
    auto it = mMultiBrtMgrExt.find(screenId);
    if (it == mMultiBrtMgrExt.end()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "No bmext found for screen id = %{public}u", screenId);
        return nullptr;
    }
    return it->second;
}

void BrightnessManager::CreateExt(const uint32_t screenId)
{
    std::unique_lock<std::shared_mutex> lock(mMutex);
    auto [it, inserted] = mMultiBrtMgrExt.emplace(screenId, std::make_shared<BrightnessManagerExt>(screenId));
    if (inserted) {
        it->second->Init(defaultMax_, defaultMin_);
        it->second->SetDisplayId(screenId);
    } else {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "BMExt for display id=%{public}u already exists",
            static_cast<uint32_t>(screenId));
    }
}

void BrightnessManager::RegisterScreenListener()
{
    if (multiScreenListener_ == nullptr) {
        multiScreenListener_ = sptr<MultiScreenListener>::MakeSptr();
        auto ret = Rosen::ScreenManagerLite::GetInstance().RegisterScreenListener(multiScreenListener_);
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "MultiscreenListener for brightness created, ret: %{public}d",
            static_cast<int32_t>(ret));
    }
}

void BrightnessManager::UnRegisterScreenListener()
{
    if (multiScreenListener_ != nullptr) {
        auto ret = Rosen::ScreenManagerLite::GetInstance().UnregisterScreenListener(multiScreenListener_);
        multiScreenListener_ = nullptr;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "MultiscreenListener for brightness destroyed, ret: %{public}d",
            static_cast<int32_t>(ret));
    }
}

void BrightnessManager::MultiScreenListener::OnConnect(uint64_t screenId)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Display[%{public}u] connected.", static_cast<uint32_t>(screenId));
    BrightnessManager::Get().CreateExt(static_cast<uint32_t>(screenId));
}
#endif

void BrightnessManager::Init(uint32_t defaultMax, uint32_t defaultMin)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    defaultMax_ = defaultMax;
    defaultMin_ = defaultMin;
    RegisterScreenListener();
    std::vector<Rosen::ScreenId> screenIds;
    Rosen::ScreenManagerLite::GetInstance().GetPhysicalScreenIds(screenIds);
    if (screenIds.empty()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Get no screenId from DMS, fill with DEFAULT_DISPLAY_ID.");
        screenIds.emplace_back(DEFAULT_DISPLAY_ID);
    }
    for (const auto& id: screenIds) {
        CreateExt(id);
    }
#else
    mBrightnessManagerExt.Init(defaultMax, defaultMin);
#endif
#else
    BrightnessService::Get().Init(defaultMax, defaultMin);
#endif
}

void BrightnessManager::DeInit()
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    UnRegisterScreenListener();
    std::unique_lock<std::shared_mutex> lock(mMutex);
    for (auto& [id, ext]: mMultiBrtMgrExt) {
        ext->DeInit();
    }
#else
    mBrightnessManagerExt.DeInit();
#endif
#else
    BrightnessService::Get().DeInit();
#endif
}

bool BrightnessManager::GetFeatureSupport(BrightnessFeatureType feature, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->GetFeatureSupport(feature);
    }
    return false;
#else
    return mBrightnessManagerExt.GetFeatureSupport(feature);
#endif
#else
    return BrightnessService::Get().GetFeatureSupport(feature);
#endif
}

void BrightnessManager::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        ext->SetDisplayState(id, state, reason);
    }
#else
    mBrightnessManagerExt.SetDisplayState(id, state, reason);
#endif
#else
    BrightnessService::Get().SetDisplayState(id, state);
#endif
}

DisplayState BrightnessManager::GetState(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->GetState();
    }
    return DisplayState::DISPLAY_UNKNOWN;
#else
    return mBrightnessManagerExt.GetState();
#endif
#else
    return BrightnessService::Get().GetDisplayState();
#endif
}

bool BrightnessManager::IsSupportLightSensor(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->IsSupportLightSensor();
    }
    return false;
#else
    return mBrightnessManagerExt.IsSupportLightSensor();
#endif
#else
    return BrightnessService::Get().IsSupportLightSensor();
#endif
}

bool BrightnessManager::IsAutoAdjustBrightness(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->IsAutoAdjustBrightness();
    }
    return false;
#else
    return mBrightnessManagerExt.IsAutoAdjustBrightness();
#endif
#else
    return BrightnessService::Get().IsAutoAdjustBrightness();
#endif
}

bool BrightnessManager::AutoAdjustBrightness(bool enable, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->AutoAdjustBrightness(enable);
    }
    return false;
#else
    return mBrightnessManagerExt.AutoAdjustBrightness(enable);
#endif
#else
    #ifdef ENABLE_SENSOR_PART
        return BrightnessService::Get().AutoAdjustBrightness(enable);
    #else
        return false;
    #endif
#endif
}

bool BrightnessManager::SetForcedBrightness(double value, uint32_t duration, BrightnessValueType valueType, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->SetForcedBrightness(value, duration, valueType);
    }
    return false;
#else
    return mBrightnessManagerExt.SetForcedBrightness(value, duration, valueType);
#endif
#else
    return BrightnessService::Get().SetForcedBrightness(value, duration, valueType);
#endif
}

bool BrightnessManager::SetBrightness(uint32_t value,
    uint32_t gradualDuration, bool continuous, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->SetBrightness(value, gradualDuration, continuous);
    }
    return false;
#else
    return mBrightnessManagerExt.SetBrightness(value, gradualDuration, continuous);
#endif
#else
    return BrightnessService::Get().SetBrightness(value, gradualDuration, continuous);
#endif
}

bool BrightnessManager::DiscountBrightness(double discount, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->DiscountBrightness(discount);
    }
    return false;
#else
    return mBrightnessManagerExt.DiscountBrightness(discount);
#endif
#else
    return BrightnessService::Get().DiscountBrightness(discount);
#endif
}

double BrightnessManager::GetDiscount(uint32_t id) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->GetDiscount();
    }
    return 1.0f;
#else
    return mBrightnessManagerExt.GetDiscount();
#endif
#else
    return BrightnessService::Get().GetDiscount();
#endif
}

void BrightnessManager::SetScreenOnBrightness(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        ext->SetScreenOnBrightness();
    }
#else
    mBrightnessManagerExt.SetScreenOnBrightness();
#endif
#else
    BrightnessService::Get().SetScreenOnBrightness();
#endif
}

uint32_t BrightnessManager::GetScreenOnBrightness(uint32_t id) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->GetScreenOnBrightness();
    }
    return 0;
#else
    return mBrightnessManagerExt.GetScreenOnBrightness();
#endif
#else
    return BrightnessService::Get().GetScreenOnBrightness(false);
#endif
}

void BrightnessManager::ClearOffset(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        ext->ClearOffset();
    }
#else
    mBrightnessManagerExt.ClearOffset();
#endif
#else
    BrightnessService::Get().ClearOffset();
#endif
}

bool BrightnessManager::OverrideBrightness(uint32_t value, uint32_t gradualDuration, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->OverrideBrightness(value, gradualDuration);
    }
    return false;
#else
    return mBrightnessManagerExt.OverrideBrightness(value, gradualDuration);
#endif
#else
    return BrightnessService::Get().OverrideBrightness(value, gradualDuration);
#endif
}

bool BrightnessManager::RestoreBrightness(uint32_t gradualDuration, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->RestoreBrightness(gradualDuration);
    }
    return false;
#else
    return mBrightnessManagerExt.RestoreBrightness(gradualDuration);
#endif
#else
    return BrightnessService::Get().RestoreBrightness(gradualDuration);
#endif
}

bool BrightnessManager::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->BoostBrightness(timeoutMs, gradualDuration);
    }
    return false;
#else
    return mBrightnessManagerExt.BoostBrightness(timeoutMs, gradualDuration);
#endif
#else
    return BrightnessService::Get().BoostBrightness(timeoutMs, gradualDuration);
#endif
}

bool BrightnessManager::CancelBoostBrightness(uint32_t gradualDuration, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->CancelBoostBrightness(gradualDuration);
    }
    return false;
#else
    return mBrightnessManagerExt.CancelBoostBrightness(gradualDuration);
#endif
#else
    return BrightnessService::Get().CancelBoostBrightness(gradualDuration);
#endif
}

uint32_t BrightnessManager::GetBrightness(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->GetBrightness();
    }
    return 0;
#else
    return mBrightnessManagerExt.GetBrightness();
#endif
#else
    return BrightnessService::Get().GetBrightness();
#endif
}

uint32_t BrightnessManager::GetDeviceBrightness(bool useHbm, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->GetDeviceBrightness(useHbm);
    }
    return 0;
#else
    return mBrightnessManagerExt.GetDeviceBrightness(useHbm);
#endif
#else
    return BrightnessService::Get().GetDeviceBrightness(useHbm);
#endif
}

void BrightnessManager::WaitDimmingDone(uint32_t id) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        ext->WaitDimmingDone();
    }
#else
    mBrightnessManagerExt.WaitDimmingDone();
#endif
#else
    BrightnessService::Get().WaitDimmingDone();
#endif
}

std::string BrightnessManager::RunJsonCommand(const std::string& request, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->RunJsonCommand(request);
    }
    return "";
#else
    return mBrightnessManagerExt.RunJsonCommand(request);
#endif
#else
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "not support RunJsonCommand: %{public}s", request.c_str());
    return R"({"ret": -1, "error": "not support RunJsonCommand"})";
#endif
}

int32_t BrightnessManager::RegisterDataChangeListener(const sptr<IDisplayBrightnessListener>& listener,
    DisplayDataChangeListenerType listenerType, const std::string& callerId, const std::string& params,
    uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->RegisterDataChangeListener(listener, listenerType, callerId, params);
    }
    return -1;
#else
    return mBrightnessManagerExt.RegisterDataChangeListener(listener, listenerType, callerId, params);
#endif
#else
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "RegisterDataChangeListener: %{public}u, %{public}s",
        listenerType, callerId.c_str());
    return 0;
#endif
}

int32_t BrightnessManager::UnregisterDataChangeListener(
    DisplayDataChangeListenerType listenerType, const std::string& callerId,
    uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->UnregisterDataChangeListener(listenerType, callerId);
    }
    return -1;
#else
    return mBrightnessManagerExt.UnregisterDataChangeListener(listenerType, callerId);
#endif
#else
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UnregisterDataChangeListener: %{public}u, %{public}s",
        listenerType, callerId.c_str());
    return 0;
#endif
}

uint32_t BrightnessManager::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback,
    uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->SetLightBrightnessThreshold(threshold, callback);
    }
    return 0;
#else
    return mBrightnessManagerExt.SetLightBrightnessThreshold(threshold, callback);
#endif
#else
    DISPLAY_HILOGW(FEAT_BRIGHTNESS, "BrightnessManager::SetLightBrightnessThreshold not support");
    return BrightnessService::Get().SetLightBrightnessThreshold(threshold, callback);
#endif
}

bool BrightnessManager::IsBrightnessOverridden(uint32_t id) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->IsBrightnessOverridden();
    }
    return false;
#else
    return mBrightnessManagerExt.IsBrightnessOverridden();
#endif
#else
    return BrightnessService::Get().IsBrightnessOverridden();
#endif
}

bool BrightnessManager::IsBrightnessBoosted(uint32_t id) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->IsBrightnessBoosted();
    }
    return false;
#else
    return mBrightnessManagerExt.IsBrightnessBoosted();
#endif
#else
    return BrightnessService::Get().IsBrightnessBoosted();
#endif
}

uint32_t BrightnessManager::GetCurrentDisplayId(uint32_t defaultId) const
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(defaultId)) {
        return ext->GetCurrentDisplayId(defaultId);
    }
    return defaultId;
#else
    return mBrightnessManagerExt.GetCurrentDisplayId(defaultId);
#endif
#else
    return BrightnessService::Get().GetCurrentDisplayId(defaultId);
#endif
}

void BrightnessManager::SetDisplayId(uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        ext->SetDisplayId(id);
    }
#else
    mBrightnessManagerExt.SetDisplayId(id);
#endif
#else
    BrightnessService::Get().SetDisplayId(id);
#endif
}

bool BrightnessManager::SetMaxBrightness(double value, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->SetMaxBrightness(value);
    }
    return false;
#else
    return mBrightnessManagerExt.SetMaxBrightness(value);
#endif
#else
    return BrightnessService::Get().SetMaxBrightness(value);
#endif
}

bool BrightnessManager::SetMaxBrightnessNit(uint32_t nit, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->SetMaxBrightnessNit(nit);
    }
    return false;
#else
    return mBrightnessManagerExt.SetMaxBrightnessNit(nit);
#endif
#else
    return BrightnessService::Get().SetMaxBrightnessNit(nit);
#endif
}

int BrightnessManager::NotifyScreenPowerStatus(uint32_t displayId, uint32_t status)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS,
        "BrightnessManager::NotifyScreenPowerStatus displayId:%{public}u, status:%{public}u", displayId, status);
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(displayId)) {
        return ext->NotifyScreenPowerStatus(displayId, status);
    }
    return -1;
#else
    return mBrightnessManagerExt.NotifyScreenPowerStatus(displayId, status);
#endif
#else
    return BrightnessService::Get().NotifyScreenPowerStatus(displayId, status);
#endif
}

bool BrightnessManager::SetSceneMode(SceneModeType type, bool enable, uint32_t id)
{
#ifdef OHOS_BUILD_ENABLE_BRIGHTNESS_WRAPPER
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    if (auto ext = GetExt(id)) {
        return ext->SetSceneMode(type, enable);
    }
    return false;
#else
    return mBrightnessManagerExt.SetSceneMode(type, enable);
#endif
#else
    return BrightnessService::Get().SetSceneMode(type, enable);
#endif
}
} // namespace DisplayPowerMgr
} // namespace OHOS