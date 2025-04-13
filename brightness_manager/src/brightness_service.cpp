/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <ipc_skeleton.h>
#include <securec.h>

#include "brightness_action.h"
#include "brightness_setting_helper.h"
#include "config_parser.h"
#include "delayed_sp_singleton.h"
#include "display_common.h"
#include "display_log.h"
#include "display_manager_lite.h"
#include "dm_common.h"
#include "errors.h"
#include "ffrt_utils.h"
#include "light_lux_manager.h"
#include "new"
#include "permission.h"
#include "refbase.h"
#include "screen_manager_lite.h"
#include "setting_provider.h"
#include "system_ability_definition.h"

#ifdef ENABLE_SENSOR_PART
#include "sensor_agent.h"
#endif

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr uint32_t MAX_DEFAULT_BRGIHTNESS_LEVEL = 255;
constexpr uint32_t MIN_DEFAULT_BRGIHTNESS_LEVEL = 1;
constexpr uint32_t MAX_MAPPING_BRGIHTNESS_LEVEL = 223;
constexpr uint32_t MIN_MAPPING_BRGIHTNESS_LEVEL = 1;
constexpr uint32_t MAX_DEFAULT_BRGIHTNESS_NIT = 600;
constexpr uint32_t MIN_DEFAULT_BRGIHTNESS_NIT = 2;
constexpr uint32_t MAX_DEFAULT_HIGH_BRGIHTNESS_LEVEL = 10000;
constexpr uint32_t MIN_DEFAULT_HIGH_BRGIHTNESS_LEVEL = 156;
constexpr uint32_t DEFAULT_ANIMATING_DURATION = 500;
constexpr uint32_t DEFAULT_BRIGHTEN_DURATION = 2000;
constexpr uint32_t DEFAULT_DARKEN_DURATION = 5000;
constexpr uint32_t DEFAULT_MAX_BRIGHTNESS_DURATION = 3000;

FFRTHandle g_cancelBoostTaskHandle{};
FFRTHandle g_waitForFirstLuxTaskHandle{};
}

const uint32_t BrightnessService::AMBIENT_LUX_LEVELS[BrightnessService::LUX_LEVEL_LENGTH] = { 1, 3, 5, 10, 20, 50, 200,
    500, 1000, 2000, 3000, 5000, 10000, 20000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, INT_MAX };

using namespace OHOS::PowerMgr;

uint32_t BrightnessService::brightnessValueMax = MAX_DEFAULT_BRGIHTNESS_LEVEL;
uint32_t BrightnessService::brightnessValueMin = MIN_DEFAULT_BRGIHTNESS_LEVEL;

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

void BrightnessService::Init(uint32_t defaultMax, uint32_t defaultMin)
{
    std::call_once(mInitCallFlag, [defaultMax, defaultMin, this] {
        queue_ = std::make_shared<FFRTQueue> ("brightness_manager");
        if (queue_ == nullptr) {
            return;
        }
        if (!mDimming->Init()) {
            return;
        }
#ifdef ENABLE_SENSOR_PART
        InitSensors();
        mIsFoldDevice = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Init mIsFoldDevice=%{public}d", mIsFoldDevice);
#endif
        ConfigParse::Get().Initialize();
        mLightLuxManager.InitParameters();
        mBrightnessCalculationManager.InitParameters();

        bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
        brightnessValueMax = defaultMax;
        brightnessValueMin = defaultMin;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::init isFoldable=%{public}d, max=%{public}u, min=%{public}u",
            isFoldable, brightnessValueMax, brightnessValueMin);
        if (isFoldable) {
            RegisterFoldStatusListener();
        }
    });
}

void BrightnessService::DeInit()
{
    bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::init isFoldable=%{public}d", isFoldable);
    if (isFoldable) {
        UnRegisterFoldStatusListener();
    }
    if (queue_) {
        queue_.reset();
        g_cancelBoostTaskHandle = nullptr;
        g_waitForFirstLuxTaskHandle = nullptr;
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
    bool isSensorEnable = BrightnessService::Get().GetIsSupportLightSensor();
    bool isAutoEnable = BrightnessService::Get().GetSettingAutoBrightness();
    bool isScreenOn = BrightnessService::Get().IsScreenOn();
    int displayId = BrightnessService::Get().GetDisplayIdWithFoldstatus(foldStatus);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnFoldStatusChanged isSensorEnable=%{public}d, isAutoEnable=%{public}d"\
        ", isScreenOn=%{public}d, displayid=%{public}d", isSensorEnable, isAutoEnable, isScreenOn, displayId);
    uint32_t currentBrightness = BrightnessService::Get().GetBrightness();
    BrightnessService::Get().SetDisplayId(displayId);

    if (Rosen::FoldStatus::FOLDED == foldStatus || Rosen::FoldStatus::EXPAND == foldStatus) {
        int sensorId = BrightnessService::Get().GetSensorIdWithFoldstatus(foldStatus);
        BrightnessService::Get().SetCurrentSensorId(static_cast<uint32_t>(sensorId));
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnFoldStatusChanged sensorid=%{public}d", sensorId);
        if (isSensorEnable && isAutoEnable && isScreenOn) {
            if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
                BrightnessService::Get().ActivateAmbientSensor();
            } else if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
                BrightnessService::Get().ActivateAmbientSensor1();
            }
        }
    } else if (Rosen::FoldStatus::HALF_FOLD == foldStatus) {
        uint32_t currentEffectSensorId = BrightnessService::Get().GetCurrentSensorId();
        if (isSensorEnable && isAutoEnable && isScreenOn) {
            if (currentEffectSensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
                BrightnessService::Get().DeactivateAmbientSensor();
            } else if (currentEffectSensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
                BrightnessService::Get().DeactivateAmbientSensor1();
            }
        }
    }

    if (BrightnessService::Get().CanSetBrightness()) {
        BrightnessService::Get().UpdateBrightness(currentBrightness, 0, true);
    }
    mLastFoldStatus = foldStatus;
}

void BrightnessService::RegisterFoldStatusListener()
{
    mFoldStatusistener = new FoldStatusLisener();
    if (mFoldStatusistener == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::RegisterDisplayModeListener newListener failed");
        return;
    }
    auto ret = Rosen::DisplayManagerLite::GetInstance().RegisterFoldStatusListener(mFoldStatusistener);
    if (ret != Rosen::DMError::DM_OK) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Rosen::DisplayManagerLite::RegisterDisplayModeListener failed");
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
    auto ret = Rosen::DisplayManagerLite::GetInstance().UnregisterFoldStatusListener(mFoldStatusistener);
    if (ret != Rosen::DMError::DM_OK) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::UnRegisterDisplayModeListener  failed");
    }
}

uint32_t BrightnessService::GetDisplayId()
{
    return mDisplayId;
}

uint32_t BrightnessService::GetCurrentDisplayId(uint32_t defaultId)
{
    uint32_t currentId = defaultId;
    bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
    if (!isFoldable) {
        DISPLAY_HILOGI(FEAT_STATE, "GetCurrentDisplayId not fold phone return default id=%{public}d", defaultId);
        return currentId;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto foldMode = Rosen::DisplayManagerLite::GetInstance().GetFoldDisplayMode();
    currentId = static_cast<uint32_t>(GetDisplayIdWithDisplayMode(foldMode));
    DISPLAY_HILOGI(FEAT_STATE, "GetCurrentDisplayId foldMode=%{public}u", foldMode);
    IPCSkeleton::SetCallingIdentity(identity);
    return static_cast<uint32_t>(currentId);
}

void BrightnessService::SetDisplayId(uint32_t displayId)
{
    mDisplayId = displayId;
    if (mAction == nullptr) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::SetDisplayId mAction == nullptr");
        return;
    }
    mAction->SetDisplayId(displayId);
}

uint32_t BrightnessService::GetCurrentSensorId()
{
    return mCurrentSensorId;
}

void BrightnessService::SetCurrentSensorId(uint32_t sensorId)
{
    mCurrentSensorId = sensorId;
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
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnChanged currentValue=%{public}d already stopDimming, return", currentValue);
        return;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnChanged brightness,mapBrightness=%{public}d", currentValue);
    bool isSuccess = mAction->SetBrightness(currentValue);
    if (isSuccess) {
        BrightnessService::Get().ReportBrightnessBigData(currentValue);
    }
    if (isSuccess && !BrightnessService::Get().IsSleepStatus()) {
        if (!BrightnessService::Get().IsDimming()) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "OnChanged already stopDimming , not update setting brightness");
            return;
        }
        FFRTTask task = [this, currentValue] {
            auto tmpVal = BrightnessService::Get().GetOrigBrightnessLevel(currentValue);
            this->mCallback(tmpVal);
        };
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
        if (mIsSleepStatus) {
            mIsBrightnessOverridden = false;
            mIsSleepStatus = false;
        }
        mBrightnessTarget.store(0);
        if (mDimming->IsDimming()) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DISPLAY_OFF StopDimming");
            mDimming->StopDimming();
        }
    } else if (state == DisplayState::DISPLAY_DIM) {
        SetSleepBrightness();
    } else if (state == DisplayState::DISPLAY_ON) {
        mIsDisplayOnWhenFirstLuxReport.store(true);
        if (mIsSleepStatus) {
            RestoreBrightness(0);
            mIsSleepStatus = false;
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

uint32_t BrightnessService::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    uint32_t result = 0;
    if (threshold.empty() || !callback) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "BrightnessService::SetLightBrightnessThreshold params verify faild.");
        return result;
    }
    result = 1;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "BrightnessService::SetLightBrightnessThreshold set listener success");
    return result;
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
        ActivateValidAmbientSensor();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable enable");
    } else {
        if (!mIsAutoBrightnessEnabled) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetAutoBrightnessEnable is already disabled");
            return true;
        }
        DeactivateAllAmbientSensor();
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
        if (IsCurrentSensorEnable()) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness is already enabled");
            return true;
        }
        ActivateValidAmbientSensor();
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness enable");
    } else {
        if (!IsCurrentSensorEnable()) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "StateChangedSetAutoBrightness is already disabled");
            return true;
        }
        DeactivateAllAmbientSensor();
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
        if (sensorInfo[i].sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT ||
            sensorInfo[i].sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
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
    if (event->sensorTypeId != SENSOR_TYPE_ID_AMBIENT_LIGHT && event->sensorTypeId != SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Sensor Callback is not AMBIENT_LIGHT id=%{public}d", event->sensorTypeId);
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

void BrightnessService::ActivateAmbientSensor1()
{
    if (!mIsAutoBrightnessEnabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "auto brightness1 is not enabled");
        return;
    }
    if (mIsLightSensor1Enabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Ambient Sensor1 is already on");
        return;
    }
    (void)strcpy_s(mSensorUser1.name, sizeof(mSensorUser1.name), "BrightnessService");
    mSensorUser1.userData = nullptr;
    mSensorUser1.callback = &AmbientLightCallback;
    SubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT1, &mSensorUser1);
    SetBatch(SENSOR_TYPE_ID_AMBIENT_LIGHT1, &mSensorUser1, SAMPLING_RATE, SAMPLING_RATE);
    ActivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT1, &mSensorUser1);
    SetMode(SENSOR_TYPE_ID_AMBIENT_LIGHT1, &mSensorUser1, SENSOR_ON_CHANGE);
    mIsLightSensor1Enabled = true;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ActivateAmbientSensor1");
}

void BrightnessService::DeactivateAmbientSensor1()
{
    if (!mIsAutoBrightnessEnabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "auto brightness1 is not enabled");
        return;
    }
    if (!mIsLightSensor1Enabled) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Ambient Sensor1 is already off");
        return;
    }
    DeactivateSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT1, &mSensorUser1);
    UnsubscribeSensor(SENSOR_TYPE_ID_AMBIENT_LIGHT1, &mSensorUser1);
    mIsLightSensor1Enabled = false;
    mLightLuxManager.ClearLuxData();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DeactivateAmbientSensor1");
}

void BrightnessService::ActivateValidAmbientSensor()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ActivateValidAmbientSensor");
    bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
    if (!isFoldable) {
        ActivateAmbientSensor();
        return;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto foldMode = Rosen::DisplayManagerLite::GetInstance().GetFoldDisplayMode();
    int sensorId = GetSensorIdWithDisplayMode(foldMode);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ActivateValidAmbientSensor sensorId=%{public}d, mode=%{public}d",
        sensorId, foldMode);
    if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        ActivateAmbientSensor();
    } else if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
        ActivateAmbientSensor1();
    }
    IPCSkeleton::SetCallingIdentity(identity);
}

void BrightnessService::DeactivateValidAmbientSensor()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DeactivateValidAmbientSensor");
    bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
    if (!isFoldable) {
        DeactivateAmbientSensor();
        return;
    }
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto foldMode = Rosen::DisplayManagerLite::GetInstance().GetFoldDisplayMode();
    int sensorId = GetSensorIdWithDisplayMode(foldMode);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DeactivateValidAmbientSensor sensorId=%{public}d, mode=%{public}d",
        sensorId, foldMode);
    if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        DeactivateAmbientSensor();
    } else if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
        DeactivateAmbientSensor1();
    }
    IPCSkeleton::SetCallingIdentity(identity);
}

void BrightnessService::DeactivateAllAmbientSensor()
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "DeactivateAllAmbientSensor");
    bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
    if (!isFoldable) {
        DeactivateAmbientSensor();
        return;
    }
    DeactivateAmbientSensor();
    DeactivateAmbientSensor1();
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
    if (!CanSetBrightness()) {
        if (mIsLuxActiveWithLog) {
            mIsLuxActiveWithLog = false;
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ProcessLightLux:mIsLuxActiveWithLog=false");
        }
        return;
    }
    if (!mIsLuxActiveWithLog) {
        mIsLuxActiveWithLog = true;
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "ProcessLightLux:mIsLuxActiveWithLog=true");
    }
    if (mLightLuxManager.IsNeedUpdateBrightness(lux)) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateLightLux, lux=%{public}f, mLightLux=%{public}f, isFirst=%{public}d",
            lux, mLightLuxManager.GetSmoothedLux(), mLightLuxManager.GetIsFirstLux());
        UpdateCurrentBrightnessLevel(lux, mLightLuxManager.GetIsFirstLux());
    }

    for (int index = 0; index < LUX_LEVEL_LENGTH; index++) {
        if (static_cast<uint32_t>(lux) < AMBIENT_LUX_LEVELS[index]) {
            if (index != mLuxLevel || mLightLuxManager.GetIsFirstLux()) {
                mLuxLevel = index;
                // Notify ambient lux change event to battery statistics
                // type:0 auto brightness, 1 manual brightness, 2 window brightness, 3 others
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
                HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "AMBIENT_LIGHT",
                    HiviewDFX::HiSysEvent::EventType::STATISTIC, "LEVEL", mLuxLevel, "TYPE", 0);
#endif
            }
            break;
        }
    }
}

void BrightnessService::UpdateCurrentBrightnessLevel(float lux, bool isFastDuration)
{
    uint32_t brightnessLevel = GetBrightnessLevel(lux);
    if (mBrightnessLevel != brightnessLevel || isFastDuration) {
        uint32_t duration = DEFAULT_BRIGHTEN_DURATION;
        if (brightnessLevel < mBrightnessLevel) {
            duration = DEFAULT_DARKEN_DURATION;
        }
        if (isFastDuration) {
            duration = DEFAULT_ANIMATING_DURATION;
        }
        if (isFastDuration && mIsDisplayOnWhenFirstLuxReport) {
            duration = 0;
            mIsDisplayOnWhenFirstLuxReport.store(false);
        }
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateCurrentBrightnessLevel lux=%{public}f, mBrightnessLevel=%{public}d, "\
            "brightnessLevel=%{public}d, duration=%{public}d", lux, mBrightnessLevel, brightnessLevel, duration);
        mBrightnessLevel = brightnessLevel;
        mCurrentBrightness.store(brightnessLevel);
        if (mWaitForFirstLux) {
            std::lock_guard<std::mutex> lock(mFirstLuxHandleLock);
            FFRT_CANCEL(g_waitForFirstLuxTaskHandle, queue_);
            mWaitForFirstLux = false;
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateCurrentBrightnessLevel CancelScreenOn waitforFisrtLux Task");
        }
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
    uint32_t brightnessLevel = static_cast<uint32_t>(mBrightnessCalculationManager.GetInterpolatedValue(lux) *
        MAX_DEFAULT_BRGIHTNESS_LEVEL);

    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "GetBrightnessLevel lux=%{public}f, brightnessLevel=%{public}d",
        lux, brightnessLevel);
    return brightnessLevel;
}

uint32_t BrightnessService::GetBrightnessHighLevel(uint32_t level)
{
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
        if (isSettingOn && IsCurrentSensorEnable()) {
            mIsUserMode = true;
            mBrightnessCalculationManager.UpdateBrightnessOffset(value, mLightLuxManager.GetSmoothedLux());
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightnessOffset level=%{public}d, mLightLux=%{public}f",
                value, mLightLuxManager.GetSmoothedLux());
        }
    }
    mBrightnessTarget.store(value);
    mCurrentBrightness.store(value);
    bool isSuccess = UpdateBrightness(value, gradualDuration, !continuous);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "SetBrightness val=%{public}d, isSuccess=%{public}d", value, isSuccess);
    mIsUserMode = false;
    return isSuccess;
}

void BrightnessService::SetScreenOnBrightness()
{
    uint32_t screenOnBrightness = GetScreenOnBrightness(true);
    if (mWaitForFirstLux) {
        std::lock_guard<std::mutex> lock(mFirstLuxHandleLock);
        if (queue_ == nullptr) {
            DISPLAY_HILOGW(FEAT_BRIGHTNESS, "SetScreenOnBrightness, queue is null");
        } else {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetScreenOnBrightness waitForFirstLux");
            FFRT_CANCEL(g_waitForFirstLuxTaskHandle, queue_);
            screenOnBrightness = mCachedSettingBrightness;
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetScreenOnBrightness waitForFirstLux,GetSettingBrightness=%{public}d",
                screenOnBrightness);
            FFRTTask setBrightnessTask = [this, screenOnBrightness] {
                this->UpdateBrightness(screenOnBrightness, 0, true);
            };
            g_waitForFirstLuxTaskHandle = FFRTUtils::SubmitDelayTask(setBrightnessTask, WAIT_FOR_FIRST_LUX_MAX_TIME,
                queue_);
        }
        return;
    }
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

void BrightnessService::SetSleepBrightness()
{
    uint32_t value = GetSettingBrightness();
    if (value <= MIN_DEFAULT_BRGIHTNESS_LEVEL) {
        return;
    }
    uint32_t sleepBrightness = BrightnessParamHelper::GetSleepBrightness();
    uint32_t sleepMinumumReductionBrightness = BrightnessParamHelper::GetSleepMinumumReductionBrightness();
    if (value < sleepMinumumReductionBrightness) {
        value = sleepMinumumReductionBrightness;
    }
    uint32_t enterSleepBrightness = std::max(std::min(value - sleepMinumumReductionBrightness, sleepBrightness),
        MIN_DEFAULT_BRGIHTNESS_LEVEL);
    uint32_t sleepDarkenTime = BrightnessParamHelper::GetSleepDarkenTime();
    mIsSleepStatus = true;
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetSleepBrightness enterSleepBrightness=%{public}d, sleepDarkenTime=%{public}d",
        enterSleepBrightness, sleepDarkenTime);
    OverrideBrightness(enterSleepBrightness, sleepDarkenTime);
}

bool BrightnessService::OverrideBrightness(uint32_t value, uint32_t gradualDuration)
{
    if (!CanOverrideBrightness()) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Cannot override brightness, ignore the change");
        return false;
    }
    if (!mIsBrightnessOverridden) {
        mIsBrightnessOverridden = true;
    }
    mOverriddenBrightness = value;
    mBeforeOverriddenBrightness = GetSettingBrightness();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Override brightness, value=%{public}u, mBeforeOverriddenBrightness=%{public}d",
        value, mBeforeOverriddenBrightness);
    return UpdateBrightness(value, gradualDuration);
}

bool BrightnessService::RestoreBrightness(uint32_t gradualDuration)
{
    if (!IsBrightnessOverridden()) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness is not override, no need to restore");
        return false;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "restore brightness=%{public}d", mBeforeOverriddenBrightness);
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
    FFRT_CANCEL(g_cancelBoostTaskHandle, queue_);
    FFRTTask task = [this, gradualDuration] { this->CancelBoostBrightness(gradualDuration); };
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
    mBoostHandleLock.lock();
    FFRT_CANCEL(g_cancelBoostTaskHandle, queue_);
    mBoostHandleLock.unlock();
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
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "UpdateBrightness, value=%{public}u, discount=%{public}f,"\
        "duration=%{public}u, updateSetting=%{public}d", value, mDiscount, gradualDuration, updateSetting);
    mWaitForFirstLux = false;
    auto safeBrightness = GetSafeBrightness(value);
    if (mDimming->IsDimming()) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightness StopDimming");
        mDimming->StopDimming();
    }
    auto brightness = static_cast<uint32_t>(safeBrightness * mDiscount);
    brightness = GetMappingBrightnessLevel(brightness);
    if (gradualDuration > 0) {
        mDimming->StartDimming(GetSettingBrightness(), brightness, gradualDuration);
        return true;
    }
    bool isSuccess = mAction->SetBrightness(brightness);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "UpdateBrightness is %{public}s, brightness: %{public}u",
        isSuccess ? "succ" : "failed", brightness);
    if (isSuccess && updateSetting) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "UpdateBrightness, settings, value=%{public}u", safeBrightness);
        FFRTUtils::SubmitTask([this, safeBrightness] { this->SetSettingBrightness(safeBrightness); });
    }
    if (isSuccess) {
        ReportBrightnessBigData(brightness);
    }
    return isSuccess;
}

uint32_t BrightnessService::GetSettingBrightness(const std::string& key)
{
    uint32_t settingBrightness = DEFAULT_BRIGHTNESS;
    auto isSuccess = BrightnessSettingHelper::GetSettingBrightness(settingBrightness, key);
    if (isSuccess != ERR_OK) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS,
            "get setting brightness failed, return default=%{public}d, key=%{public}s, ret=%{public}d",
            settingBrightness, key.c_str(), isSuccess);
    }
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
    } else if (isUpdateTarget && mIsAutoBrightnessEnabled) {
        if (mBrightnessTarget.load() > 0) {
            DISPLAY_HILOGI(FEAT_BRIGHTNESS, "update, return mBrightnessTarget=%{public}d", mBrightnessTarget.load());
            screenOnbrightness = mBrightnessTarget.load();
        } else {
            screenOnbrightness = 0;
            mWaitForFirstLux = true;
        }
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
    if (levelIn < MIN_MAPPING_BRGIHTNESS_LEVEL) {
        levelIn = MIN_MAPPING_BRGIHTNESS_LEVEL;
    }
    if (levelIn > MAX_MAPPING_BRGIHTNESS_LEVEL) {
        levelIn = MAX_MAPPING_BRGIHTNESS_LEVEL;
    }
    double nitOut = (double)(levelIn - MIN_MAPPING_BRGIHTNESS_LEVEL)
        * (MAX_DEFAULT_BRGIHTNESS_NIT - MIN_DEFAULT_BRGIHTNESS_NIT)
         / (MAX_MAPPING_BRGIHTNESS_LEVEL - MIN_MAPPING_BRGIHTNESS_LEVEL) + MIN_DEFAULT_BRGIHTNESS_NIT;
    return round(nitOut);
}

uint32_t BrightnessService::GetBrightnessLevelFromNit(uint32_t nit)
{
    uint32_t nitIn = nit;
    if (nitIn < MIN_DEFAULT_BRGIHTNESS_NIT) {
        nitIn = MIN_DEFAULT_BRGIHTNESS_NIT;
    }
    if (nitIn > MAX_DEFAULT_BRGIHTNESS_NIT) {
        nitIn = MAX_DEFAULT_BRGIHTNESS_NIT;
    }
    double levelOut = (double)(nitIn - MIN_DEFAULT_BRGIHTNESS_NIT)
        * (MAX_DEFAULT_BRGIHTNESS_LEVEL - MIN_DEFAULT_BRGIHTNESS_LEVEL)
         / (MAX_DEFAULT_BRGIHTNESS_NIT - MIN_DEFAULT_BRGIHTNESS_NIT) + MIN_DEFAULT_BRGIHTNESS_LEVEL;
    return round(levelOut);
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

std::string BrightnessService::GetReason()
{
    if (mIsBrightnessOverridden) {
        return "APP";
    }
    if (mIsUserMode) {
        return "USER";
    }
    if (mIsAutoBrightnessEnabled) {
        return "AUTO";
    }
    return "MANUAL";
}

void BrightnessService::ReportBrightnessBigData(uint32_t brightness)
{
    std::string reason = GetReason();
    uint32_t nit = GetMappingBrightnessNit(brightness);
    // Notify screen brightness change event to battery statistics
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::DISPLAY, "BRIGHTNESS_NIT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC, "BRIGHTNESS", brightness, "REASON", reason, "NIT", nit);
#endif
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "BigData brightness=%{public}d,reason=%{public}s,nit=%{public}d",
        brightness, reason.c_str(), nit);
}

bool BrightnessService::IsSleepStatus()
{
    return mIsSleepStatus;
}

bool BrightnessService::GetIsSupportLightSensor()
{
    return mIsSupportLightSensor;
}

bool BrightnessService::IsCurrentSensorEnable()
{
    bool isFoldable = Rosen::DisplayManagerLite::GetInstance().IsFoldable();
    if (!isFoldable) {
        return mIsLightSensorEnabled;
    }
    bool result = false;
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto foldMode = Rosen::DisplayManagerLite::GetInstance().GetFoldDisplayMode();
    int sensorId = GetSensorIdWithDisplayMode(foldMode);
    if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT) {
        result = mIsLightSensorEnabled;
    } else if (sensorId == SENSOR_TYPE_ID_AMBIENT_LIGHT1) {
        result = mIsLightSensor1Enabled;
    }
    IPCSkeleton::SetCallingIdentity(identity);
    return result;
}

int BrightnessService::GetDisplayIdWithFoldstatus(Rosen::FoldStatus foldStatus)
{
    return mBrightnessCalculationManager.GetDisplayIdWithFoldstatus(static_cast<int>(foldStatus));
}

int BrightnessService::GetSensorIdWithFoldstatus(Rosen::FoldStatus foldStatus)
{
    return mBrightnessCalculationManager.GetSensorIdWithFoldstatus(static_cast<int>(foldStatus));
}

int BrightnessService::GetDisplayIdWithDisplayMode(Rosen::FoldDisplayMode mode)
{
    return mBrightnessCalculationManager.GetDisplayIdWithDisplayMode(static_cast<int>(mode));
}

int BrightnessService::GetSensorIdWithDisplayMode(Rosen::FoldDisplayMode mode)
{
    return mBrightnessCalculationManager.GetSensorIdWithDisplayMode(static_cast<int>(mode));
}

bool BrightnessService::SetMaxBrightness(double value)
{
    uint32_t maxValue = static_cast<uint32_t>(value * MAX_DEFAULT_BRGIHTNESS_LEVEL);
    if (maxValue == 0 || value < 0) {
        maxValue = brightnessValueMin;
    }
    if (maxValue == brightnessValueMax) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetMaxBrightness value=oldMax");
        return true;
    }
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetMaxBrightness value=%{public}u, oldMax=%{public}u",
        maxValue, brightnessValueMax);
    brightnessValueMax =
        (maxValue > MAX_DEFAULT_BRGIHTNESS_LEVEL ? MAX_DEFAULT_BRGIHTNESS_LEVEL : maxValue);
    uint32_t currentBrightness = GetSettingBrightness();
    if (brightnessValueMax < currentBrightness) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetMaxBrightness currentBrightness=%{public}u", currentBrightness);
        return UpdateBrightness(brightnessValueMax, DEFAULT_MAX_BRIGHTNESS_DURATION, true);
    }
    if (mCurrentBrightness.load() == 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "No need to update brightne during init");
        return true;
    }
    return UpdateBrightness(mCurrentBrightness.load(), DEFAULT_MAX_BRIGHTNESS_DURATION, true);
}

bool BrightnessService::SetMaxBrightnessNit(uint32_t maxNit)
{
    uint32_t max_value = GetBrightnessLevelFromNit(maxNit);
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetMaxBrightnessNit nitIn=%{public}u, levelOut=%{public}u",
        maxNit, max_value);
    if (max_value == brightnessValueMax) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetMaxBrightness value=oldMax");
        return true;
    }
    brightnessValueMax =
        (max_value > MAX_DEFAULT_BRGIHTNESS_LEVEL ? MAX_DEFAULT_BRGIHTNESS_LEVEL : max_value);
    uint32_t currentBrightness = GetSettingBrightness();
    if (brightnessValueMax < currentBrightness) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "SetMaxBrightnessNit currentBrightness=%{public}u", currentBrightness);
        return UpdateBrightness(brightnessValueMax, DEFAULT_MAX_BRIGHTNESS_DURATION, true);
    }
    if (mCurrentBrightness.load() == 0) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "No need to update brightne during init");
        return true;
    }
    return UpdateBrightness(mCurrentBrightness.load(), DEFAULT_MAX_BRIGHTNESS_DURATION, true);
}

uint32_t BrightnessService::GetSafeBrightness(uint32_t value)
{
    auto brightnessValue = value;
    if (brightnessValue > brightnessValueMax) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "value is bigger than max=%{public}u, value=%{public}u",
            brightnessValueMax, value);
        brightnessValue = brightnessValueMax;
    }
    if (brightnessValue < brightnessValueMin) {
        DISPLAY_HILOGI(FEAT_BRIGHTNESS, "brightness value is less than min, value=%{public}u", value);
        brightnessValue = brightnessValueMin;
    }
    return brightnessValue;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
