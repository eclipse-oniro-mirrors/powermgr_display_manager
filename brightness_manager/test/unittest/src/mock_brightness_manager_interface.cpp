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

#include "mock_brightness_manager_interface.h"

#include <cstdint>
#include <list>
#include <vector>
namespace OHOS {
namespace DisplayPowerMgr {
static std::list<double> g_values{};

// 打桩接口
void MockSetValue(double value)
{
    g_values.emplace_back(value);
}

// 清理打桩数据
void MockClearValues(void)
{
    g_values.clear();
}

static double MockReturnValue(void)
{
    auto v = g_values.front();
    g_values.pop_front();
    return v;
}

static bool MockReturnBool(void)
{
    return MockReturnValue() >= 0;
}

void Init(uint32_t defaultMax, uint32_t defaultMin)
{
}

void DeInit()
{
}

void SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
}

DisplayState GetDisplayState()
{
    return static_cast<DisplayState>(MockReturnBool());
}

bool IsSupportLightSensor(void)
{
    return MockReturnBool();
}

bool IsAutoAdjustBrightness(void)
{
    return MockReturnBool();
}

bool AutoAdjustBrightness(bool enable)
{
    return MockReturnBool();
}

bool SetBrightness(uint32_t value, uint32_t gradualDuration, bool continuous)
{
    return MockReturnBool();
}

bool DiscountBrightness(double discount)
{
    return MockReturnBool();
}

double GetDiscount()
{
    return MockReturnValue();
}

void SetScreenOnBrightness()
{
}

uint32_t GetScreenOnBrightness()
{
    return MockReturnValue();
}

bool OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    return MockReturnBool();
}

bool RestoreBrightness(uint32_t gradualDuration)
{
    return MockReturnBool();
}

bool BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
    return MockReturnBool();
}

bool CancelBoostBrightness(uint32_t gradualDuration)
{
    return MockReturnBool();
}

bool IsBrightnessOverridden()
{
    return MockReturnBool();
}

bool IsBrightnessBoosted()
{
    return MockReturnBool();
}

uint32_t GetBrightness()
{
    return MockReturnValue();
}

uint32_t GetDeviceBrightness()
{
    return MockReturnValue();
}

void WaitDimmingDone()
{
}

void ClearOffset()
{
}

uint32_t GetCurrentDisplayId(uint32_t defaultId)
{
    return MockReturnValue();
}

void SetDisplayId(uint32_t id)
{
}

uint32_t SetLightBrightnessThreshold(std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    return MockReturnValue();
}

bool SetMaxBrightness(double value)
{
    return MockReturnBool();
}

bool SetMaxBrightnessNit(uint32_t maxNit)
{
    return MockReturnBool();
}

void MockInitBrightnessManagerExt(OHOS::DisplayPowerMgr::BrightnessManagerExt& ext)
{
    ext.mBrightnessManagerExtEnable = true;
    ext.mBrightnessManagerInitFunc = reinterpret_cast<void*>(Init);
    ext.mBrightnessManagerDeInitFunc = reinterpret_cast<void*>(DeInit);
    ext.mSetDisplayStateFunc = reinterpret_cast<void*>(SetDisplayState);
    ext.mGetDisplayStateFunc = reinterpret_cast<void*>(GetDisplayState);
    ext.mIsSupportLightSensorFunc = reinterpret_cast<void*>(IsSupportLightSensor);
    ext.mIsAutoAdjustBrightnessFunc = reinterpret_cast<void*>(IsAutoAdjustBrightness);
    ext.mAutoAdjustBrightnessFunc = reinterpret_cast<void*>(AutoAdjustBrightness);
    ext.mSetBrightnessFunc = reinterpret_cast<void*>(SetBrightness);
    ext.mDiscountBrightnessFunc = reinterpret_cast<void*>(DiscountBrightness);
    ext.mGetDiscountFunc = reinterpret_cast<void*>(GetDiscount);
    ext.mSetScreenOnBrightnessFunc = reinterpret_cast<void*>(SetScreenOnBrightness);
    ext.mGetScreenOnBrightnessFunc = reinterpret_cast<void*>(GetScreenOnBrightness);
    ext.mOverrideBrightnessFunc = reinterpret_cast<void*>(OverrideBrightness);
    ext.mRestoreBrightnessFunc = reinterpret_cast<void*>(RestoreBrightness);
    ext.mBoostBrightnessFunc = reinterpret_cast<void*>(BoostBrightness);
    ext.mCancelBoostBrightnessFunc = reinterpret_cast<void*>(CancelBoostBrightness);
    ext.mIsBrightnessOverriddenFunc = reinterpret_cast<void*>(IsBrightnessOverridden);
    ext.mIsBrightnessBoostedFunc = reinterpret_cast<void*>(IsBrightnessBoosted);
    ext.mGetBrightnessFunc = reinterpret_cast<void*>(GetBrightness);
    ext.mGetDeviceBrightnessFunc = reinterpret_cast<void*>(GetDeviceBrightness);
    ext.mWaitDimmingDoneFunc = reinterpret_cast<void*>(WaitDimmingDone);
    ext.mClearOffsetFunc = reinterpret_cast<void*>(ClearOffset);
    ext.mGetCurrentDisplayIdFunc = reinterpret_cast<void*>(GetCurrentDisplayId);
    ext.mSetDisplayIdFunc = reinterpret_cast<void*>(SetDisplayId);
    ext.mSetLightBrightnessThresholdFunc = reinterpret_cast<void*>(SetLightBrightnessThreshold);
    ext.mSetMaxBrightnessFunc = reinterpret_cast<void*>(SetMaxBrightness);
    ext.mSetMaxBrightnessNitFunc = reinterpret_cast<void*>(SetMaxBrightnessNit);
}

void MockDeInitBrightnessManagerExt(OHOS::DisplayPowerMgr::BrightnessManagerExt& ext)
{
    ext.mBrightnessManagerExtEnable = false;
    ext.mBrightnessManagerInitFunc = nullptr;
    ext.mBrightnessManagerDeInitFunc = nullptr;
    ext.mSetDisplayStateFunc = nullptr;
    ext.mGetDisplayStateFunc = nullptr;
    ext.mAutoAdjustBrightnessFunc = nullptr;
    ext.mSetBrightnessFunc = nullptr;
    ext.mDiscountBrightnessFunc = nullptr;
    ext.mGetDiscountFunc = nullptr;
    ext.mSetScreenOnBrightnessFunc = nullptr;
    ext.mGetScreenOnBrightnessFunc = nullptr;
    ext.mOverrideBrightnessFunc = nullptr;
    ext.mRestoreBrightnessFunc = nullptr;
    ext.mBoostBrightnessFunc = nullptr;
    ext.mCancelBoostBrightnessFunc = nullptr;
    ext.mIsBrightnessOverriddenFunc = nullptr;
    ext.mIsBrightnessBoostedFunc = nullptr;
    ext.mGetBrightnessFunc = nullptr;
    ext.mGetDeviceBrightnessFunc = nullptr;
    ext.mClearOffsetFunc = nullptr;
    ext.mGetCurrentDisplayIdFunc = nullptr;
    ext.mSetDisplayIdFunc = nullptr;
    ext.mSetLightBrightnessThresholdFunc = nullptr;
    ext.mSetMaxBrightnessFunc = nullptr;
    ext.mSetMaxBrightnessNitFunc = nullptr;
}
}
}