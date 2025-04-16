/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#include <dlfcn.h>
#include "brightness_manager_ext.h"

namespace OHOS {
namespace DisplayPowerMgr {
BrightnessManagerExt::BrightnessManagerExt()
{
}

BrightnessManagerExt::~BrightnessManagerExt()
{
    CloseBrightnessExtLibrary();
}

void BrightnessManagerExt::Init(uint32_t defaultMax, uint32_t defaultMin)
{
    if (!LoadBrightnessExtLibrary()) {
        return;
    }
    auto init = reinterpret_cast<void (*)(uint32_t, uint32_t)>(mBrightnessManagerInitFunc);
    init(defaultMax, defaultMin);
}

void BrightnessManagerExt::DeInit()
{
    auto deInit = reinterpret_cast<void (*)()>(mBrightnessManagerDeInitFunc);
    if (deInit) {
        deInit();
    }
    CloseBrightnessExtLibrary();
}

bool BrightnessManagerExt::LoadBrightnessExtLibrary()
{
#ifndef FUZZ_TEST
    mBrightnessManagerExtHandle = dlopen("libbrightness_wrapper.z.so", RTLD_NOW);
#endif
    if (!mBrightnessManagerExtHandle) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlopen libbrightness_wrapper.z.so failed!");
        return false;
    }
    if (LoadBrightnessOps() && LoadBrightnessStatus() && LoadBrightnessControl()) {
        mBrightnessManagerExtEnable = true;
    } else {
        mBrightnessManagerExtEnable = false;
        CloseBrightnessExtLibrary();
    }
    return mBrightnessManagerExtEnable;
}

bool BrightnessManagerExt::LoadBrightnessOps()
{
    mBrightnessManagerInitFunc = dlsym(mBrightnessManagerExtHandle, "Init");
    if (!mBrightnessManagerInitFunc) {
        return false;
    }
    mBrightnessManagerDeInitFunc = dlsym(mBrightnessManagerExtHandle, "DeInit");
    if (!mBrightnessManagerDeInitFunc) {
        return false;
    }
    mGetCurrentDisplayIdFunc = dlsym(mBrightnessManagerExtHandle, "GetCurrentDisplayId");
    if (!mGetCurrentDisplayIdFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym GetCurrentDisplayId func failed!");
        return false;
    }
    mSetDisplayIdFunc = dlsym(mBrightnessManagerExtHandle, "SetDisplayId");
    if (!mSetDisplayIdFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetDisplayId func failed!");
        return false;
    }
    mSetLightBrightnessThresholdFunc = dlsym(mBrightnessManagerExtHandle, "SetLightBrightnessThreshold");
    if (!mSetLightBrightnessThresholdFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetLightBrightnessThreshold func failed!");
        return false;
    }
    mSetMaxBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "SetMaxBrightness");
    if (!mSetMaxBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetMaxBrightness func failed!");
        return false;
    }
    mSetMaxBrightnessNitFunc = dlsym(mBrightnessManagerExtHandle, "SetMaxBrightnessNit");
    if (!mSetMaxBrightnessNitFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetMaxBrightnessNit func failed!");
        return false;
    }
    return true;
}

bool BrightnessManagerExt::LoadBrightnessStatus()
{
    mSetDisplayStateFunc = dlsym(mBrightnessManagerExtHandle, "SetDisplayState");
    if (!mSetDisplayStateFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetDisplayState func failed!");
        return false;
    }
    mGetDisplayStateFunc = dlsym(mBrightnessManagerExtHandle, "GetDisplayState");
    if (!mGetDisplayStateFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym GetDisplayState func failed!");
        return false;
    }
    mGetDiscountFunc = dlsym(mBrightnessManagerExtHandle, "GetDiscount");
    if (!mGetDiscountFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym GetDiscount func failed!");
        return false;
    }
    mGetScreenOnBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "GetScreenOnBrightness");
    if (!mGetScreenOnBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym GetScreenOnBrightness func failed!");
        return false;
    }
    mCancelBoostBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "CancelBoostBrightness");
    if (!mCancelBoostBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym CancelBoostBrightness func failed!");
        return false;
    }
    mIsBrightnessOverriddenFunc = dlsym(mBrightnessManagerExtHandle, "IsBrightnessOverridden");
    if (!mIsBrightnessOverriddenFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym IsBrightnessOverridden func failed!");
        return false;
    }
    mIsBrightnessBoostedFunc = dlsym(mBrightnessManagerExtHandle, "IsBrightnessBoosted");
    if (!mIsBrightnessBoostedFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym IsBrightnessBoosted func failed!");
        return false;
    }
    mGetBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "GetBrightness");
    if (!mGetBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym GetBrightness func failed!");
        return false;
    }
    mGetDeviceBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "GetDeviceBrightness");
    if (!mGetDeviceBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym GetDeviceBrightness func failed!");
        return false;
    }
    return true;
}

bool BrightnessManagerExt::LoadBrightnessControl()
{
    mAutoAdjustBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "AutoAdjustBrightness");
    if (!mAutoAdjustBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym AutoAdjustBrightness func failed!");
        return false;
    }
    mSetBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "SetBrightness");
    if (!mSetBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetBrightness func failed!");
        return false;
    }
    mDiscountBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "DiscountBrightness");
    if (!mDiscountBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym DiscountBrightness func failed!");
        return false;
    }
    mSetScreenOnBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "SetScreenOnBrightness");
    if (!mSetScreenOnBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym SetScreenOnBrightness func failed!");
        return false;
    }
    mOverrideBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "OverrideBrightness");
    if (!mOverrideBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym OverrideBrightness func failed!");
        return false;
    }
    mRestoreBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "RestoreBrightness");
    if (!mRestoreBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym RestoreBrightness func failed!");
        return false;
    }
    mBoostBrightnessFunc = dlsym(mBrightnessManagerExtHandle, "BoostBrightness");
    if (!mBoostBrightnessFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym BoostBrightness func failed!");
        return false;
    }
    mClearOffsetFunc = dlsym(mBrightnessManagerExtHandle, "ClearOffset");
    if (!mClearOffsetFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym ClearOffset func failed!");
        return false;
    }
    mNotifyScreenPowerStatusFunc = dlsym(mBrightnessManagerExtHandle, "NotifyScreenPowerStatus");
    if (!mNotifyScreenPowerStatusFunc) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "dlsym mNotifyScreenPowerStatusFunc func failed!");
        return false;
    }
    return true;
}

void BrightnessManagerExt::CloseBrightnessExtLibrary()
{
    if (mBrightnessManagerExtHandle) {
#ifndef FUZZ_TEST
        dlclose(mBrightnessManagerExtHandle);
#endif
        mBrightnessManagerExtHandle = nullptr;
    }
    mBrightnessManagerInitFunc = nullptr;
    mBrightnessManagerDeInitFunc = nullptr;
    mSetDisplayStateFunc = nullptr;
    mGetDisplayStateFunc = nullptr;
    mAutoAdjustBrightnessFunc = nullptr;
    mSetBrightnessFunc = nullptr;
    mDiscountBrightnessFunc = nullptr;
    mGetDiscountFunc = nullptr;
    mSetScreenOnBrightnessFunc = nullptr;
    mGetScreenOnBrightnessFunc = nullptr;
    mOverrideBrightnessFunc = nullptr;
    mRestoreBrightnessFunc = nullptr;
    mBoostBrightnessFunc = nullptr;
    mCancelBoostBrightnessFunc = nullptr;
    mIsBrightnessOverriddenFunc = nullptr;
    mIsBrightnessBoostedFunc = nullptr;
    mGetBrightnessFunc = nullptr;
    mGetDeviceBrightnessFunc = nullptr;
    mClearOffsetFunc = nullptr;
    mGetCurrentDisplayIdFunc = nullptr;
    mSetDisplayIdFunc = nullptr;
    mSetMaxBrightnessFunc = nullptr;
    mSetMaxBrightnessNitFunc = nullptr;
    mNotifyScreenPowerStatusFunc = nullptr;
}

void BrightnessManagerExt::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
    if (!mBrightnessManagerExtEnable) {
        return;
    }
    auto setDisplayStateFunc = reinterpret_cast<void (*)(uint32_t, DisplayState state, uint32_t reason)>
        (mSetDisplayStateFunc);
    setDisplayStateFunc(id, state, reason);
}

DisplayState BrightnessManagerExt::GetState()
{
    if (!mBrightnessManagerExtEnable) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    auto getDisplayStateFunc = reinterpret_cast<DisplayState (*)()>(mGetDisplayStateFunc);
    return getDisplayStateFunc();
}

bool BrightnessManagerExt::AutoAdjustBrightness(bool enable)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto autoAdjustBrightnessFunc = reinterpret_cast<bool (*)(bool)>(mAutoAdjustBrightnessFunc);
    return autoAdjustBrightnessFunc(enable);
}

bool BrightnessManagerExt::SetBrightness(uint32_t value, uint32_t gradualDuration, bool continuous)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto setBrightnessFunc = reinterpret_cast<bool (*)(uint32_t, uint32_t, bool)>(mSetBrightnessFunc);
    return setBrightnessFunc(value, gradualDuration, continuous);
}

bool BrightnessManagerExt::DiscountBrightness(double discount)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto discountBrightnessFunc = reinterpret_cast<bool (*)(double)>(mDiscountBrightnessFunc);
    return discountBrightnessFunc(discount);
}

double BrightnessManagerExt::GetDiscount() const
{
    if (!mBrightnessManagerExtEnable) {
        return 1.0;
    }
    auto getDiscountFunc = reinterpret_cast<double (*)()>(mGetDiscountFunc);
    return getDiscountFunc();
}

void BrightnessManagerExt::SetScreenOnBrightness()
{
    if (!mBrightnessManagerExtEnable) {
        return;
    }
    auto setScreenOnBrightnessFunc = reinterpret_cast<void (*)()>(mSetScreenOnBrightnessFunc);
    setScreenOnBrightnessFunc();
}

uint32_t BrightnessManagerExt::GetScreenOnBrightness() const
{
    if (!mBrightnessManagerExtEnable) {
        return 0;
    }
    auto getScreenOnBrightnessFunc = reinterpret_cast<uint32_t (*)(bool)>(mGetScreenOnBrightnessFunc);
    return getScreenOnBrightnessFunc(false);
}

void BrightnessManagerExt::ClearOffset()
{
    if (!mBrightnessManagerExtEnable) {
        return;
    }
    auto clearOffsetFunc = reinterpret_cast<void (*)()>(mClearOffsetFunc);
    clearOffsetFunc();
}

bool BrightnessManagerExt::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto overrideBrightnessFunc = reinterpret_cast<bool (*)(uint32_t, uint32_t)>(mOverrideBrightnessFunc);
    return overrideBrightnessFunc(value, gradualDuration);
}

bool BrightnessManagerExt::RestoreBrightness(uint32_t gradualDuration)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto restoreBrightnessFunc = reinterpret_cast<bool (*)(uint32_t)>(mRestoreBrightnessFunc);
    return restoreBrightnessFunc(gradualDuration);
}

bool BrightnessManagerExt::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto boostBrightnessFunc = reinterpret_cast<bool (*)(uint32_t, uint32_t)>(mBoostBrightnessFunc);
    return boostBrightnessFunc(timeoutMs, gradualDuration);
}

bool BrightnessManagerExt::CancelBoostBrightness(uint32_t gradualDuration)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto cancelBoostBrightnessFunc = reinterpret_cast<bool (*)(uint32_t)>(mCancelBoostBrightnessFunc);
    return cancelBoostBrightnessFunc(gradualDuration);
}

uint32_t BrightnessManagerExt::GetBrightness()
{
    if (!mBrightnessManagerExtEnable) {
        return 0;
    }
    auto getBrightnessFunc = reinterpret_cast<uint32_t (*)()>(mGetBrightnessFunc);
    return getBrightnessFunc();
}

uint32_t BrightnessManagerExt::GetDeviceBrightness()
{
    if (!mBrightnessManagerExtEnable) {
        return 0;
    }
    auto getDeviceBrightnessFunc = reinterpret_cast<uint32_t (*)()>(mGetDeviceBrightnessFunc);
    return getDeviceBrightnessFunc();
}

bool BrightnessManagerExt::IsBrightnessOverridden() const
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto isBrightnessOverriddenFunc = reinterpret_cast<bool (*)()>(mIsBrightnessOverriddenFunc);
    return isBrightnessOverriddenFunc();
}

bool BrightnessManagerExt::IsBrightnessBoosted() const
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto isBrightnessBoostedFunc = reinterpret_cast<bool (*)()>(mIsBrightnessBoostedFunc);
    return isBrightnessBoostedFunc();
}

uint32_t BrightnessManagerExt::GetCurrentDisplayId(uint32_t defaultId) const
{
    if (!mBrightnessManagerExtEnable) {
        return defaultId;
    }
    auto getCurrentDisplayIdFunc = reinterpret_cast<uint32_t (*)(uint32_t)>(mGetCurrentDisplayIdFunc);
    return getCurrentDisplayIdFunc(defaultId);
}

void BrightnessManagerExt::SetDisplayId(uint32_t id)
{
    if (!mBrightnessManagerExtEnable) {
        return;
    }
    auto setDisplayIdFunc = reinterpret_cast<void (*)(uint32_t)>(mSetDisplayIdFunc);
    setDisplayIdFunc(id);
}

uint32_t BrightnessManagerExt::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    uint32_t result = 0;
    if (!mBrightnessManagerExtEnable) {
        return result;
    }
    auto setLightBrightnessThresholdFunc =
        reinterpret_cast<uint32_t (*)(std::vector<int32_t>, sptr<IDisplayBrightnessCallback>)>(
        mSetLightBrightnessThresholdFunc);
    if (!setLightBrightnessThresholdFunc) {
        return result;
    }
    return setLightBrightnessThresholdFunc(threshold, callback);
}

bool BrightnessManagerExt::SetMaxBrightness(double value)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto setMaxBrightnessFunc = reinterpret_cast<bool (*)(double)>(mSetMaxBrightnessFunc);
    return setMaxBrightnessFunc(value);
}

bool BrightnessManagerExt::SetMaxBrightnessNit(uint32_t nit)
{
    if (!mBrightnessManagerExtEnable) {
        return false;
    }
    auto setMaxBrightnessNitFunc = reinterpret_cast<bool (*)(uint32_t)>(mSetMaxBrightnessNitFunc);
    return setMaxBrightnessNitFunc(nit);
}

int BrightnessManagerExt::NotifyScreenPowerStatus(uint32_t displayId, uint32_t status)
{
    if (!mBrightnessManagerExtEnable) {
        return -1; // -1 means return failed
    }
    auto NotifyScreenPowerStatusFunc =
        reinterpret_cast<int (*)(uint32_t, uint32_t)>(mNotifyScreenPowerStatusFunc);
    return NotifyScreenPowerStatusFunc(displayId, status);
}
} // namespace DisplayPowerMgr
} // namespace OHOS