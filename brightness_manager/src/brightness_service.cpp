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

#include "brightness_service.h"

#include <file_ex.h>
#include <hisysevent.h>
#include <ipc_skeleton.h>
#include <securec.h>

#include "brightness_action.h"
#include "brightness_setting_helper.h"
#include "config_parser.h"
#include "delayed_sp_singleton.h"
#include "display_common.h"
#include "display_log.h"
#include "display_manager.h"
#include "dm_common.h"
#include "errors.h"
#include "ffrt_utils.h"
#include "light_lux_manager.h"
#include "new"
#include "permission.h"
#include "refbase.h"
#include "screen_manager.h"
#include "setting_provider.h"
#include "system_ability_definition.h"

#ifdef ENABLE_SENSOR_PART
#include "sensor_agent.h"
#endif

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr uint32_t MAX_DEFAULT_BRGIHTNESS_LEVEL = 255;
constexpr uint32_t MIN_DEFAULT_BRGIHTNESS_LEVEL = 4;
constexpr uint32_t MAX_MAPPING_BRGIHTNESS_LEVEL = 223;
constexpr uint32_t MIN_MAPPING_BRGIHTNESS_LEVEL = 4;
constexpr uint32_t MAX_HBM_BRGIHTNESS_NIT = 1000;
constexpr uint32_t MAX_DEFAULT_BRGIHTNESS_NIT = 600;
constexpr uint32_t MIN_DEFAULT_BRGIHTNESS_NIT = 2;
constexpr uint32_t MAX_DEFAULT_HIGH_BRGIHTNESS_LEVEL = 10000;
constexpr uint32_t MIN_DEFAULT_HIGH_BRGIHTNESS_LEVEL = 156;
constexpr uint32_t DEFAULT_ANIMATING_DURATION = 500;
constexpr uint32_t DEFAULT_BRIGHTEN_DURATION = 2000;
constexpr uint32_t DEFAULT_DARKEN_DURATION = 5000;

FFRTHandle g_cancelBoostTaskHandle{};
}

using namespace OHOS::PowerMgr;

BrightnessService::BrightnessService()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService created for displayId=%{public}d", mDisplayId);
    mAction = std::make_shared<BrightnessAction>(mDisplayId);
    if (mAction == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mAction is null");
        return;
    }
    mState = mAction->GetDisplayState();
    mDimmingCallback = std::make_shared<DimmingCallbackImpl>(mAction, [this](uint32_t brightness) {
        SetSettingBrightness(brightness);
    });
    if (mDimmingCallback == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "mDimmingCallback is null");
    }
    std::string name = "BrightnessService" + std::to_string(mDisplayId);
    mDimming = std::make_shared<BrightnessDimming>(name, mDimmingCallback);
}

BrightnessService& BrightnessService::Get()
{
    static BrightnessService brightnessManager;
    return brightnessManager;
}

void BrightnessService::Init()
{
    queue_ = std::make_shared<FFRTQueue> ("brightness_manager");
    if (queue_ == nullptr) {
        return;
    }
    if (!mDimming->Init()) {
        return;
    }
#ifdef ENABLE_SENSOR_PART
    InitSensors();
    mIsFoldDevice = Rosen::DisplayManager::GetInstance().IsFoldable();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Init mIsFoldDevice=%{public}d", mIsFoldDevice);
#endif
    ConfigParse::Get().Initialize();
    mLightLuxManager.InitParameters();
    mBrightnessCalculationManager.InitParameters();

    bool isFoldable = Rosen::DisplayManager::GetInstance().IsFoldable();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::init isFoldable=%{public}d", isFoldable);
    if (isFoldable) {
        RegisterFoldStatusListener();
    }
}

void BrightnessService::DeInit()
{
    bool isFoldable = Rosen::DisplayManager::GetInstance().IsFoldable();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::init isFoldable=%{public}d", isFoldable);
    if (isFoldable) {
        UnRegisterFoldStatusListener();
    }
    if (queue_) {
        queue_.reset();
        g_cancelBoostTaskHandle = nullptr;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "destruct brightness ffrt queue");
    }

    mDimming->Reset();
}

void BrightnessService::FoldStatusLisener::OnFoldStatusChanged(Rosen::FoldStatus foldStatus)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnFoldStatusChanged currenFoldStatus=%{public}d", foldStatus);
    if (mLastFoldStatus == foldStatus) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "no need set foldStatus");
        return;
    }
    uint32_t currentBrightness = BrightnessService::Get().GetBrightness();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnFoldStatusChanged currentBrightness=%{public}d", currentBrightness);
    if (foldStatus == Rosen::FoldStatus::FOLDED) {
        BrightnessService::Get().SetDisplayId(OUTTER_SCREEN_DISPLAY_ID);
        BrightnessService::Get().SetBrightness(currentBrightness);
    } else if (foldStatus == Rosen::FoldStatus::EXPAND) {
        BrightnessService::Get().SetDisplayId(DEFAULT_DISPLAY_ID);
        BrightnessService::Get().SetBrightness(currentBrightness);
    } else {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "no need to changed displayID");
    }
}

void BrightnessService::RegisterFoldStatusListener()
{
    mFoldStatusistener = new FoldStatusLisener();
    if (mFoldStatusistener == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::RegisterDisplayModeListener newListener failed");
        return;
    }
    auto ret = Rosen::DisplayManager::GetInstance().RegisterFoldStatusListener(mFoldStatusistener);
    if (ret != Rosen::DMError::DM_OK) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Rosen::DisplayManager::RegisterDisplayModeListener failed");
        mFoldStatusistener = nullptr;
    } else {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::RegisterDisplayModeListener success");
    }
}

void BrightnessService::UnRegisterFoldStatusListener()
{
    if (mFoldStatusistener == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::UnRegistermFoldStatusistener listener is null");
        return;
    }
    auto ret = Rosen::DisplayManager::GetInstance().UnregisterFoldStatusListener(mFoldStatusistener);
    if (ret != Rosen::DMError::DM_OK) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::UnRegisterDisplayModeListener  failed");
    }
}

uint32_t BrightnessService::GetDisplayId()
{
    return mDisplayId;
}

uint32_t BrightnessService::GetCurrentDisplayId(uint32_t defaultId) const
{
    return mAction->GetCurrentDisplayId(defaultId);
}

void BrightnessService::SetDisplayId(uint32_t displayId)
{
    mDisplayId = displayId;
    if (mAction == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::SetDisplayId mAction == nullptr");
        return;
    }
    mAction->SetDisplayId(displayId);
    mState = mAction->GetDisplayState();
}

BrightnessService::DimmingCallbackImpl::DimmingCallbackImpl(
    const std::shared_ptr<BrightnessAction>& action, std::function<void(uint32_t)> callback)
    : mAction(action), mCallback(callback)
{
}

void BrightnessService::DimmingCallbackImpl::OnStart()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "BrightnessDimmingCallback onStart");
}

void BrightnessService::DimmingCallbackImpl::OnChanged(uint32_t currentValue)
{
    if (!BrightnessService::Get().IsDimming()) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnChanged currentValue=%{public}d already stopDimming, return",
            currentValue);
        return;
    }
    auto brightness = GetMappingBrightnessLevel(currentValue);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnChanged currentValue=%{public}d, mapBrightness=%{public}d",
        currentValue, brightness);
    bool isSuccess = mAction->SetBrightness(brightness);
    if (isSuccess) {
        if (!BrightnessService::Get().IsDimming()) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnChanged already stopDimming , not update setting brightness");
            return;
        }
        FFRTTask task = std::bind([this](uint32_t value) { mCallback(value); }, currentValue);
        FFRTUtils::SubmitTask(task);
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Update OnChanged,Setting brightness=%{public}d", currentValue);
    } else {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Update OnChanged failed, brightness=%{public}d", currentValue);
    }
}

void BrightnessService::DimmingCallbackImpl::OnEnd()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ScreenAnimatorCallback OnEnd");
}

void BrightnessService::DimmingCallbackImpl::DiscountBrightness(double discount)
{
    mDiscount = discount;
}

void BrightnessService::SetDisplayState(uint32_t id, DisplayState state)
{
    SetDisplayId(id);
    mState = state;
    bool isAutoMode = false;
    bool isScreenOn = IsScreenOnState(state); // depend on state on
    bool isSettingOn = false;
    if (isScreenOn) {
        isSettingOn = IsAutoAdjustBrightness();
    }
    isAutoMode = isScreenOn && isSettingOn;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetDisplayState id=%{public}d, isAutoMode=%{public}d, isScreenOn=%{public}d, "\
        "isSettingOn=%{public}d, state=%{public}d", id, isAutoMode, isScreenOn, isSettingOn, state);
#ifdef ENABLE_SENSOR_PART
    bool isModeChange = StateChangedSetAutoBrightness(isAutoMode);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetDisplayState id=%{public}d, isAutoMode=%{public}d, isModeChange=%{public}d",
        id, isAutoMode, isModeChange);
#endif
    if (state == DisplayState::DISPLAY_OFF) {
        mBrightnessTarget.store(0);
        if (mDimming->IsDimming()) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DISPLAY_OFF StopDimming");
            mDimming->StopDimming();
        }
    }
}

DisplayState BrightnessService::GetDisplayState()
{
    return mState;
}

bool BrightnessService::IsScreenOnState(DisplayState state)
{
    return state == DisplayState::DISPLAY_ON || state == DisplayState::DISPLAY_DIM;
}

bool BrightnessService::GetSettingAutoBrightness(const std::string& key)
{
    return BrightnessSettingHelper::GetSettingAutoBrightness(key);
}

void BrightnessService::SetSettingAutoBrightness(bool enable)
{
    BrightnessSettingHelper::SetSettingAutoBrightness(enable);
}

#ifdef ENABLE_SENSOR_PART
bool BrightnessService::AutoAdjustBrightness(bool enable)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable start, enable=%{public}d, isEnabled=%{public}d, "\
        "isSupport=%{public}d", enable, mIsAutoBrightnessEnabled, mIsSupportLightSensor);
    if (!mIsSupportLightSensor) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable not support");
        SetSettingAutoBrightness(false);
        return false;
    }
    if (enable) {
        if (mIsAutoBrightnessEnabled) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable is already enabled");
            return true;
        }
        mIsAutoBrightnessEnabled = true;
        ActivateAmbientSensor();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable enable");
    } else {
        if (!mIsAutoBrightnessEnabled) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable is already disabled");
            return true;
        }
        DeactivateAmbientSensor();
        mIsAutoBrightnessEnabled = false;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable disable");
    }
    return true;
}

bool BrightnessService::StateChangedSetAutoBrightness(bool enable)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness start, enable=%{public}d, "\
        "isSensorEnabled=%{public}d, isSupport=%{public}d", enable, mIsLightSensorEnabled, mIsSupportLightSensor);
    if (!mIsSupportLightSensor) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness not support");
        SetSettingAutoBrightness(false);
        return false;
    }
    if (enable) {
        if (mIsLightSensorEnabled) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness is already enabled");
            return true;
        }
        ActivateAmbientSensor();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness enable");
    } else {
        if (!mIsLightSensorEnabled) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness is already disabled");
            return true;
        }
        DeactivateAmbientSensor();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness disable");
    }
    return true;
}

void BrightnessService::InitSensors()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "InitSensors start");
    SensorInfo* sensorInfo = nullptr;
    int32_t count;
    int ret = GetAllSensors(&sensorInfo, &count);
    if (ret != 0 || sensorInfo == nullptr || count < 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Can't get sensors");
        return;
    }
    mIsSupportLightSensor = false;
    for (int i = 0; i < count; i++) {
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AMBIENT_LIGHT Support");
            mIsSupportLightSensor = true;
            break;
        }
    }
    if (!mIsSupportLightSensor) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "AMBIENT_LIGHT not support");
    }
}

void BrightnessService::AmbientLightCallback(SensorEvent* event)
{
    if (event == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Sensor event is nullptr");
        return;
    }
    if (event->sensorTypeId != SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Sensor Callback is not AMBIENT_LIGHT");
        return;
    }
    AmbientLightData* data = reinterpret_cast<AmbientLightData*>(event->data);
    if (data == nullptr) {
        DISPLAY_HILOGE(FEAT_BRIGHTNESS, "AmbientLightData is null");
        return;
    }
    BrightnessService::Get().ProcessLightLux(data->intensity);
}

void BrightnessService::ActivateAmbientSensor()
{
    if (!mIsAutoBrightnessEnabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "auto brightness is not enabled");
        return;
    }
    if (mIsLightSensorEnabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Ambient Sensor is already on");
        return;
    }
    (void)strcpy_s(mSensorUser.name, sizeof(mSensorUser.name), "BrightnessService");
    mSensorUser.userData = nullptr;
    mSensorUser.callback = &AmbientLightCallback;
    SubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &mSensorUser);
    SetBatch(SENSOR_TYPE_ID_AMBIENT_LIGHT, &mSensorUser, SAMPLING_RATE, SAMPLING_RATE);
    ActivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &mSensorUser);
    SetMode(SENSOR_TYPE_ID_AMBIENT_LIGHT, &mSensorUser, SENSOR_ON_CHANGE);
    mIsLightSensorEnabled = true;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ActivateAmbientSensor");
}

void BrightnessService::DeactivateAmbientSensor()
{
    if (!mIsAutoBrightnessEnabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "auto brightness is not enabled");
        return;
    }
    if (!mIsLightSensorEnabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Ambient Sensor is already off");
        return;
    }
    DeactivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &mSensorUser);
    UnsubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT, &mSensorUser);
    mIsLightSensorEnabled = false;
    mLightLuxManager.ClearLuxData();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DeactivateAmbientSensor");
}
#endif

bool BrightnessService::IsAutoAdjustBrightness()
{
    return mIsAutoBrightnessEnabled;
}

void BrightnessService::ProcessLightLux(float lux)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ProcessLightLux, lux=%{public}f, mLightLux=%{public}f",
        lux, mLightLuxManager.GetSmoothedLux());
    if (mLightLuxManager.IsNeedUpdateBrightness(lux)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateLightLux, lux=%{public}f, mLightLux=%{public}f, isFirst=%{public}d",
            lux, mLightLuxManager.GetSmoothedLux(), mLightLuxManager.GetIsFirstLux());
        UpdateCurrentBrightnessLevel(lux, mLightLuxManager.GetIsFirstLux());
    }
}

void BrightnessService::UpdateCurrentBrightnessLevel(float lux, bool isFastDuration)
{
    if (!CanSetBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot UpdateCurrentBrightnessLevel, ignore the change");
        mLightLuxManager.ClearLuxData(); //clear luxData, prevent the next lux unadjusted
        return;
    }

    uint32_t brightnessLevel = GetBrightnessLevel(lux);
    if (mBrightnessLevel != brightnessLevel) {
        uint32_t duration = DEFAULT_BRIGHTEN_DURATION;
        if (brightnessLevel < mBrightnessLevel) {
            duration = DEFAULT_DARKEN_DURATION;
        }
        if (isFastDuration) {
            duration = DEFAULT_ANIMATING_DURATION;
        }
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateCurrentBrightnessLevel lux=%{public}f, mBrightnessLevel=%{public}d, "\
            "brightnessLevel=%{public}d, duration=%{public}d", lux, mBrightnessLevel, brightnessLevel, duration);
        mBrightnessLevel = brightnessLevel;
        mBrightnessTarget.store(brightnessLevel);
        SetBrightnessLevel(brightnessLevel, duration);
    }
}

void BrightnessService::SetBrightnessLevel(uint32_t value, uint32_t duration)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetBrightnessLevel value=%{public}d, duration=%{public}d",
        value, duration);
    UpdateBrightness(value, duration, true);
}

uint32_t BrightnessService::GetBrightnessLevel(float lux)
{
    uint32_t brightnessLevel = static_cast<int>(mBrightnessCalculationManager.GetInterpolatedValue(lux)
        * MAX_DEFAULT_BRGIHTNESS_LEVEL);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetBrightnessLevel lux=%{public}f, brightnessLevel=%{public}d",
        lux, brightnessLevel);
    return brightnessLevel;
}

uint32_t BrightnessService::GetBrightnessHighLevel(uint32_t level)
{
    uint32_t brightnessHighLevel = level;
    return level;
}

bool BrightnessService::SetBrightness(uint32_t value, uint32_t gradualDuration, bool continuous)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "SetBrightness val=%{public}d, duration=%{public}d", value, gradualDuration);
    if (!CanSetBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot set brightness, ignore the change");
        mCachedSettingBrightness = value;
        return false;
    }
    if (gradualDuration == 0) {
        bool isSettingOn = IsAutoAdjustBrightness();
        if (isSettingOn && mIsLightSensorEnabled) {
        mBrightnessCalculationManager.UpdateBrightnessOffset(value, mLightLuxManager.GetSmoothedLux());
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightnessOffset level=%{public}d, mLightLux=%{public}f",
            value, mLightLuxManager.GetSmoothedLux());
        }
    }
    mBrightnessTarget.store(value);
    bool isSuccess = UpdateBrightness(value, gradualDuration, !continuous);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "SetBrightness val=%{public}d, isSuccess=%{public}d", value, isSuccess);
    return isSuccess;
}

void BrightnessService::SetScreenOnBrightness()
{
    uint32_t screenOnBrightness = GetScreenOnBrightness(true);
    bool needUpdateBrightness = true;
    if (IsBrightnessBoosted() || IsBrightnessOverridden()) {
        needUpdateBrightness = false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetScreenOnBrightness screenOnBrightness=%{public}d, needUpdate=%{public}d",
        screenOnBrightness, needUpdateBrightness);
    UpdateBrightness(screenOnBrightness, 0, needUpdateBrightness);
}

void BrightnessService::ClearOffset()
{
    if (mDimming->IsDimming()) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "mode off StopDimming");
        mDimming->StopDimming();
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ClearOffset mLightLux=%{public}f", mLightLuxManager.GetSmoothedLux());
    mBrightnessTarget.store(0);
    mBrightnessCalculationManager.UpdateBrightnessOffset(0, mLightLuxManager.GetSmoothedLux());
}

uint32_t BrightnessService::GetBrightness()
{
    return GetSettingBrightness();
}

uint32_t BrightnessService::GetDeviceBrightness()
{
    uint32_t brightness = DEFAULT_BRIGHTNESS;
    if (mAction != nullptr) {
        brightness = mAction->GetBrightness();
    }
    return GetOrigBrightnessLevel(brightness);
}

uint32_t BrightnessService::GetCachedSettingBrightness()
{
    return mCachedSettingBrightness;
}

bool BrightnessService::DiscountBrightness(double discount, uint32_t gradualDuration)
{
    if (!CanDiscountBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot discount brightness, ignore the change");
        return false;
    }
    auto safeDiscount = discount;
    if (safeDiscount > DISCOUNT_MAX) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "discount value is greater than max, discount=%{public}lf", discount);
        safeDiscount = DISCOUNT_MAX;
    }
    if (safeDiscount < DISCOUNT_MIN) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "discount value is less than min, discount=%{public}lf", discount);
        safeDiscount = DISCOUNT_MIN;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Discount brightness, safeDiscount=%{public}lf", safeDiscount);
    mDiscount = safeDiscount;
    if (mDimmingCallback) {
        mDimmingCallback->DiscountBrightness(safeDiscount);
    }
    uint32_t screenOnBrightness = GetScreenOnBrightness(false);
    return UpdateBrightness(screenOnBrightness, gradualDuration);
}

bool BrightnessService::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    if (!CanOverrideBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot override brightness, ignore the change");
        return false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Override brightness, value=%{public}u", value);
    if (!mIsBrightnessOverridden) {
        mIsBrightnessOverridden = true;
    }
    mOverriddenBrightness = value;
    mBeforeOverriddenBrightness = GetSettingBrightness();
    return UpdateBrightness(value, gradualDuration);
}

bool BrightnessService::RestoreBrightness(uint32_t gradualDuration)
{
    if (!IsBrightnessOverridden()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not override, no need to restore");
        return false;
    }
    mIsBrightnessOverridden = false;
    return UpdateBrightness(mBeforeOverriddenBrightness, gradualDuration, true);
}

bool BrightnessService::IsBrightnessOverridden()
{
    return mIsBrightnessOverridden;
}

bool BrightnessService::BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration)
{
    if (!CanBoostBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot boost brightness, ignore the change");
        return false;
    }
    bool isSuccess = true;
    if (!mIsBrightnessBoosted) {
        uint32_t maxBrightness = BrightnessParamHelper::GetMaxBrightness();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Boost brightness, maxBrightness: %{public}d", maxBrightness);
        mIsBrightnessBoosted = true;
        isSuccess = UpdateBrightness(maxBrightness, gradualDuration);
    }

    // If boost multi-times, we will resend the cancel boost event.
    FFRTUtils::CancelTask(g_cancelBoostTaskHandle, queue_);
    FFRTTask task = std::bind(&BrightnessService::CancelBoostBrightness, this, gradualDuration);
    g_cancelBoostTaskHandle = FFRTUtils::SubmitDelayTask(task, timeoutMs, queue_);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BoostBrightness update timeout=%{public}u, isSuccess=%{public}d", timeoutMs,
        isSuccess);
    return isSuccess;
}

bool BrightnessService::CancelBoostBrightness(uint32_t gradualDuration)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Cancel boost brightness");
    if (!IsBrightnessBoosted()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not boost, no need to restore");
        return false;
    }
    FFRTUtils::CancelTask(g_cancelBoostTaskHandle, queue_);
    mIsBrightnessBoosted = false;
    return UpdateBrightness(mCachedSettingBrightness, gradualDuration, true);
}

bool BrightnessService::IsBrightnessBoosted()
{
    return mIsBrightnessBoosted;
}

bool BrightnessService::IsScreenOn()
{
    return (mState == DisplayState::DISPLAY_ON || mState == DisplayState::DISPLAY_DIM);
}

bool BrightnessService::CanSetBrightness()
{
    bool isScreenOn = IsScreenOn();
    bool isOverridden = IsBrightnessOverridden();
    bool isBoosted = IsBrightnessBoosted();
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "isScreenOn: %{public}d, isOverridden: %{public}d, isBoosted: %{public}d",
        isScreenOn, isOverridden, isBoosted);
    return isScreenOn && !isOverridden && !isBoosted;
}

bool BrightnessService::CanDiscountBrightness()
{
    return IsScreenOn();
}

bool BrightnessService::CanOverrideBrightness()
{
    return IsScreenOn() && !IsBrightnessBoosted();
}

bool BrightnessService::CanBoostBrightness()
{
    return IsScreenOn() && !IsBrightnessOverridden();
}

bool BrightnessService::UpdateBrightness(uint32_t value, uint32_t gradualDuration, bool updateSetting)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "UpdateBrightness, value=%{public}u, discount=%{public}lf,"\
        "duration=%{public}u, updateSetting=%{public}d", value, mDiscount, gradualDuration, updateSetting);
    if (mDimming->IsDimming()) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightness StopDimming");
        mDimming->StopDimming();
    }
    if (gradualDuration > 0) {
        mDimming->StartDimming(GetSettingBrightness(), value, gradualDuration);
        return true;
    }
    auto brightness = static_cast<uint32_t>(value * mDiscount);
    brightness = GetMappingBrightnessLevel(brightness);
    bool isSuccess = mAction->SetBrightness(brightness);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "UpdateBrightness is %{public}s, brightness: %{public}u",
        isSuccess ? "succ" : "failed", brightness);
    if (isSuccess && updateSetting) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightness, settings, value=%{public}u", value);
        FFRTUtils::SubmitTask(std::bind(&BrightnessService::SetSettingBrightness, this, value));
    }
    return isSuccess;
}

uint32_t BrightnessService::GetSettingBrightness(const std::string& key)
{
    uint32_t settingBrightness;
    auto isSuccess = BrightnessSettingHelper::GetSettingBrightness(settingBrightness, key);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetSettingBrightness=%{public}d", settingBrightness);
    return settingBrightness;
}

void BrightnessService::SetSettingBrightness(uint32_t value)
{
    BrightnessSettingHelper::SetSettingBrightness(value);
    mBrightnessLevel = value;
    mCachedSettingBrightness = value;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetSettingBrightness brightness=%{public}u", value);
}

uint32_t BrightnessService::GetScreenOnBrightness(bool isUpdateTarget)
{
    uint32_t screenOnbrightness;
    if (IsBrightnessBoosted()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is boosted, return max brightness");
        screenOnbrightness = BrightnessParamHelper::GetMaxBrightness();
    } else if (IsBrightnessOverridden()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is overridden, return overridden brightness=%{public}u",
            mOverriddenBrightness);
        screenOnbrightness = mOverriddenBrightness;
    } else if (isUpdateTarget && mBrightnessTarget.load() > 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "update, return mBrightnessTarget=%{public}d", mBrightnessTarget.load());
        screenOnbrightness = mBrightnessTarget.load();
    } else {
        screenOnbrightness = GetSettingBrightness();
    }
    return screenOnbrightness;
}

void BrightnessService::RegisterSettingBrightnessObserver()
{
}

void BrightnessService::UpdateBrightnessSettingFunc(const std::string& key)
{
}

void BrightnessService::UnregisterSettingBrightnessObserver()
{
    BrightnessSettingHelper::UnregisterSettingBrightnessObserver();
}

double BrightnessService::GetDiscount() const
{
    return mDiscount;
}

uint32_t BrightnessService::GetDimmingUpdateTime() const
{
    return mDimming->GetDimmingUpdateTime();
}

uint32_t BrightnessService::GetMappingBrightnessLevel(uint32_t level)
{
    if (level < MIN_DEFAULT_BRGIHTNESS_LEVEL) {
        level = MIN_DEFAULT_BRGIHTNESS_LEVEL;
    }
    if (level > MAX_DEFAULT_BRGIHTNESS_LEVEL) {
        level = MAX_DEFAULT_BRGIHTNESS_LEVEL;
    }
    float levelOut = static_cast<float>(1.0f * (level - MIN_DEFAULT_BRGIHTNESS_LEVEL)
         * (MAX_MAPPING_BRGIHTNESS_LEVEL - MIN_MAPPING_BRGIHTNESS_LEVEL)
         / (MAX_DEFAULT_BRGIHTNESS_LEVEL - MIN_DEFAULT_BRGIHTNESS_LEVEL)) + MIN_MAPPING_BRGIHTNESS_LEVEL + 0.5f;
    if (static_cast<uint32_t>(levelOut) < BrightnessParamHelper::GetMinBrightness()) {
        return BrightnessParamHelper::GetMinBrightness();
    }
    return static_cast<uint32_t>(levelOut);
}

uint32_t BrightnessService::GetOrigBrightnessLevel(uint32_t level)
{
    if (level < MIN_MAPPING_BRGIHTNESS_LEVEL) {
        level = MIN_MAPPING_BRGIHTNESS_LEVEL;
    }
    if (level > MAX_MAPPING_BRGIHTNESS_LEVEL) {
        level = MAX_MAPPING_BRGIHTNESS_LEVEL;
    }
    float levelOut = static_cast<float>(1.0f * (level - MIN_MAPPING_BRGIHTNESS_LEVEL)
        * (MAX_DEFAULT_BRGIHTNESS_LEVEL - MIN_DEFAULT_BRGIHTNESS_LEVEL)
         / (MAX_MAPPING_BRGIHTNESS_LEVEL - MIN_MAPPING_BRGIHTNESS_LEVEL) + MIN_MAPPING_BRGIHTNESS_LEVEL + 0.5f);
    return static_cast<int>(levelOut);
}

uint32_t BrightnessService::GetMappingBrightnessNit(uint32_t level)
{
    uint32_t levelIn = level;
    if (levelIn < MIN_DEFAULT_BRGIHTNESS_LEVEL) {
        levelIn = MIN_DEFAULT_BRGIHTNESS_LEVEL;
    }
    if (levelIn > MAX_DEFAULT_BRGIHTNESS_LEVEL) {
        levelIn = MAX_DEFAULT_BRGIHTNESS_LEVEL;
    }
    uint32_t nitOut = (levelIn - MIN_DEFAULT_BRGIHTNESS_LEVEL)
        * (MAX_DEFAULT_BRGIHTNESS_NIT - MIN_DEFAULT_BRGIHTNESS_NIT)
         / (MAX_DEFAULT_BRGIHTNESS_LEVEL - MIN_DEFAULT_BRGIHTNESS_LEVEL) + MIN_DEFAULT_BRGIHTNESS_NIT;
    return nitOut;
}

uint32_t BrightnessService::GetMappingHighBrightnessLevel(uint32_t level)
{
    uint32_t levelIn = level;
    if (levelIn < MIN_DEFAULT_BRGIHTNESS_LEVEL) {
        levelIn = MIN_DEFAULT_BRGIHTNESS_LEVEL;
    }
    if (levelIn > MAX_DEFAULT_BRGIHTNESS_LEVEL) {
        levelIn = MAX_DEFAULT_BRGIHTNESS_LEVEL;
    }
    uint32_t levelOut = (levelIn - MIN_DEFAULT_BRGIHTNESS_LEVEL)
         * (MAX_DEFAULT_HIGH_BRGIHTNESS_LEVEL - MIN_DEFAULT_HIGH_BRGIHTNESS_LEVEL)
         / (MAX_DEFAULT_BRGIHTNESS_LEVEL - MIN_DEFAULT_BRGIHTNESS_LEVEL) + MIN_DEFAULT_HIGH_BRGIHTNESS_LEVEL;
    return levelOut;
}

void BrightnessService::UpdateBrightnessSceneMode(BrightnessSceneMode mode)
{
}

bool BrightnessService::IsDimming()
{
    if (mDimming == nullptr) {
        return false;
    }
    return mDimming->IsDimming();
}

} // namespace DisplayPowerMgr
} // namespace OHOS
