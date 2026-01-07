/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "display_service_test.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "display_power_mgr_client.h"
#include "ipc_object_stub.h"
#include "power_state_machine_info.h"
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <memory>
#include <unistd.h>
#include "display_brightness_callback_stub.h"
#include "display_power_mgr_service.h"
#include "screen_manager_lite.h"
#include "permission.h"
#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
#include "miscellaneous_display_power_strategy.h"
#endif

using namespace testing;
using namespace testing::ext;
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
using namespace OHOS::HiviewDFX;
#endif
using namespace OHOS::PowerMgr;
using namespace OHOS::DisplayPowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static constexpr uint32_t BRIGHTNESS_SETTING_VALUE = 50;
static constexpr uint32_t BRIGHTNESS_OVERRIDE_VALUE = 100;
static constexpr uint32_t BRIGHTNESS_DURATION = 0;
static constexpr uint32_t BRIGHTNESS_MIN_VALUE = 1;
static constexpr uint32_t BRIGHTNESS_MAX_VALUE = 255;
static constexpr uint32_t BRIGHTNESS_NONE_VALUE = 0;
static constexpr uint32_t DISPLAY_ID = 0;
static constexpr uint32_t DISPLAY_MAIN_ID = 0;
static constexpr uint32_t DISPLAY_INVALID_ID = 211;
static constexpr uint32_t REASON = 0;
static constexpr int32_t BRIGHTNESS_ADJUST_VALUE = 150;
static constexpr int32_t TIMEOUT_MS = 500;
static constexpr int32_t INVALID_TIMEOUT_MS = 0;
static constexpr double DISCOUNT_VALUE = 0.30;
static constexpr uint32_t SET_SUCC = 1;
static constexpr uint32_t MAX_BRIGHTNESS_SET = 125;
static constexpr uint32_t TEST_MODE = 1;
static constexpr uint32_t DEFAULT_DURATION = 500;
static const uint32_t OVERRIDE_DELAY_TIME = 200;
static constexpr int32_t DEFAULT_VALUE = -1;
static constexpr uint32_t BRIGHTNESS_OFF = 0;
static const uint32_t TEST_DELAY_TIME_UNSET = 0;
static constexpr uint32_t DEFAULT_WAITING_TIME = 1200000;
sptr<DisplayPowerMgrService> g_service;
OHOS::Rosen::ScreenPowerState g_powerState = OHOS::Rosen::ScreenPowerState::POWER_ON;
bool g_isPermissionGranted = true;
NiceMock<DisplayServiceTest::BrightnessServiceMock>* g_brightnessServiceMock;
} // namespace

namespace OHOS::PowerMgr {
bool Permission::IsSystem()
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest IsSystem, g_isPermissionGranted: %{pubilc}d", g_isPermissionGranted);
    return g_isPermissionGranted;
}
} // namespace OHOS::PowerMgr

namespace OHOS::Rosen {
bool DisplayManagerLite::SetScreenPowerById(
    Rosen::ScreenId screenId, Rosen::ScreenPowerState status, Rosen::PowerStateChangeReason reason)
{
    if (reason == Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN) {
        return false;
    }
    return true;
}
} // namespace OHOS::Rosen

void DisplayServiceTest::SetUpTestCase()
{
    g_service = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    g_service->Init();

    g_brightnessServiceMock = new NiceMock<DisplayServiceTest::BrightnessServiceMock>();
}

void DisplayServiceTest::TearDownTestCase()
{
    g_service->Deinit();
    g_service->Reset();

    testing::Mock::AllowLeak(g_brightnessServiceMock);
    g_brightnessServiceMock = nullptr;
}

void DisplayServiceTest::DisplayPowerMgrTestCallback::OnDisplayStateChanged(
    uint32_t displayId, DisplayPowerMgr::DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrTestCallback::OnDisplayStateChangedStub");
}

namespace OHOS::Rosen {
bool DisplayManagerLite::SetDisplayState(DisplayState state, DisplayStateCallback callback)
{
    return true;
}

ScreenPowerState ScreenManagerLite::GetScreenPower()
{
    return g_powerState;
}
} //namespace OHOS::Rosen

namespace {
/**
 * @tc.name: DisplayServiceInnerTest001
 * @tc.desc: test DisplayPowerMgrService function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceInnerTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceInnerTest001 function start!");
    EXPECT_TRUE(g_service != nullptr);
    sptr<IDisplayPowerCallback> callbackPtr = new DisplayServiceTest::DisplayPowerMgrTestCallback();

    EXPECT_FALSE(g_service->RegisterCallbackInner(callbackPtr));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceInnerTest001 function end!");
}

/**
 * @tc.name: DisplayServiceInnerTest002
 * @tc.desc: test DisplayPowerMgrService function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceInnerTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceInnerTest002 function start!");
    EXPECT_TRUE(g_service != nullptr);
    sptr<IDisplayPowerCallback> callbackPtr = new DisplayServiceTest::DisplayPowerMgrTestCallback();
    g_service->UnregisterCallbackInner();
    EXPECT_FALSE(callbackPtr == nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceInnerTest002 function end!");
}

/**
 * @tc.name: DisplayServiceTest001
 * @tc.desc: test DisplayPowerMgrService function SetDisplayState id != DISPLAY_MAIN_ID
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest001 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        REASON, ret);
    int32_t displayState = 0;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest001 function end!");
}

/**
 * @tc.name: DisplayServiceTest002
 * @tc.desc: test DisplayPowerMgrService function SetDisplayState id != DISPLAY_MAIN_ID
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest002 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    auto controller = g_service->controllerMap_[DISPLAY_MAIN_ID];
    g_service->controllerMap_.erase(DISPLAY_MAIN_ID);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        REASON, ret);
    EXPECT_FALSE(ret);
    int32_t displayState = 0;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    EXPECT_EQ(static_cast<int32_t>(DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN), displayState);
    g_service->controllerMap_.emplace(DISPLAY_MAIN_ID, controller);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest002 function end!");
}

/**
 * @tc.name: DisplayServiceTest003
 * @tc.desc: test DisplayPowerMgrService function GetDisplayIds
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest003 function start!");
    EXPECT_TRUE(g_service != nullptr);
    std::vector<uint32_t> ids;
    g_service->GetDisplayIds(ids);
    EXPECT_FALSE(ids.empty());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest003 function end!");
}

/**
 * @tc.name: DisplayServiceTest004
 * @tc.desc: test DisplayPowerMgrService function NotifyScreenPowerStatus
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest004, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest004 function start!");
    EXPECT_TRUE(g_service != nullptr);
    int32_t errCode = 0;
    int32_t displayState = 0;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    g_service->NotifyScreenPowerStatus(DISPLAY_MAIN_ID, displayState, errCode);
    EXPECT_NE(errCode, DEFAULT_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest004 function end!");
}

/**
 * @tc.name: DisplayServiceTest005
 * @tc.desc: test DisplayPowerMgrService function RegisterCallback
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest005, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest005 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    sptr<IDisplayPowerCallback> callbackPtr = new DisplayServiceTest::DisplayPowerMgrTestCallback();
    g_service->RegisterCallback(callbackPtr, ret);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest005 function end!");
}

/**
 * @tc.name: DisplayServiceTest006
 * @tc.desc: test DisplayPowerMgrService function SetCoordinated
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest006, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest006 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->SetCoordinated(true, DISPLAY_MAIN_ID, ret);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest006 function end!");
}

/**
 * @tc.name: DisplayServiceTest007
 * @tc.desc: test DisplayPowerMgrService function Dump
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest007, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest007 function start!");
    EXPECT_TRUE(g_service != nullptr);
    int fd = 1;
    std::vector<std::u16string> args;
    g_service->isBootCompleted_ = false;
    auto ret = g_service->Dump(fd, args);
    EXPECT_EQ(ret, ERR_NO_INIT);
    g_service->isBootCompleted_ = true;
    ret = g_service->Dump(fd, args);
    EXPECT_EQ(ret, ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest007 function end!");
}

/**
 * @tc.name: DisplayServiceTest008
 * @tc.desc: test DisplayPowerMgrService function OverrideDisplayOffDelay
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest008, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest008 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->OverrideDisplayOffDelay(TEST_DELAY_TIME_UNSET, ret);
    EXPECT_FALSE(g_service->isDisplayDelayOff_);
    EXPECT_FALSE(ret);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF),
        REASON, ret);
    EXPECT_TRUE(ret);
    g_service->OverrideDisplayOffDelay(OVERRIDE_DELAY_TIME, ret);
    EXPECT_FALSE(g_service->isDisplayDelayOff_);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest008 function end!");
}

/**
 * @tc.name: DisplayServiceTest009
 * @tc.desc: test DisplayPowerMgrService function UndoSetDisplayStateInner
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest009, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest009 function start!");
    DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_ON;
    uint32_t reason = 0;
    g_service->UndoSetDisplayStateInner(DISPLAY_ID, state, reason);
    EXPECT_TRUE(g_service != nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest009 function end!");
}

/**
 * @tc.name: DisplayServiceTest010
 * @tc.desc: test set screen state
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest010, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest010 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool result = false;
    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF), REASON, result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        REASON, result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_DIM),
        REASON, result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_SUSPEND), REASON, result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN), REASON, result);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest010 function end!");
}

/**
 * @tc.name: DisplayServiceTest011
 * @tc.desc: test get screen state
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest011, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest011 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool result = false;
    int32_t displayState = 0;
    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF), REASON, result);
    EXPECT_TRUE(result);
    g_powerState = OHOS::Rosen::ScreenPowerState::POWER_OFF;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    EXPECT_EQ(displayState, 0);

    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        REASON, result);
    EXPECT_TRUE(result);
    g_powerState = OHOS::Rosen::ScreenPowerState::POWER_ON;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    EXPECT_EQ(displayState, 2);

    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_DIM),
        REASON, result);
    EXPECT_TRUE(result);
    g_powerState = OHOS::Rosen::ScreenPowerState::POWER_STAND_BY;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    EXPECT_EQ(displayState, 1);

    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_SUSPEND), REASON, result);
    EXPECT_TRUE(result);
    g_powerState = OHOS::Rosen::ScreenPowerState::POWER_SUSPEND;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    EXPECT_EQ(displayState, 3);

    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN), REASON, result);
    EXPECT_TRUE(result);
    g_powerState = OHOS::Rosen::ScreenPowerState::INVALID_STATE;
    g_service->GetDisplayState(DISPLAY_MAIN_ID, displayState);
    EXPECT_EQ(displayState, 7);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest011 function end!");
}


/**
 * @tc.name: DisplayServiceTest012
 * @tc.desc: Test set coordinated and set display state
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest012 function start!");
    bool result = false;
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF),
        REASON, result);
    int sleepTime = 2;
    //Callback for waiting window 2s
    sleep(sleepTime);
    // Prepare test environment.
    // If the state of display is DISPLAY_ON, change it to DISPLAY_OFF.
    g_service->SetDisplayState(DISPLAY_MAIN_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPLICATION), result);
    EXPECT_TRUE(result);
    sleep(sleepTime);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPLICATION), result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPLICATION), result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT), result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS), result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON), result);
    EXPECT_TRUE(result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF),
        result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest012 function end!");
}

/**
 * @tc.name: DisplayServiceTest013
 * @tc.desc: Test srceen delay off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest013 function start!");
    bool result = false;
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        REASON, result);
    uint32_t delayTime = 10; //delay time is 10ms
    int32_t displayState = 0;
    g_service->OverrideDisplayOffDelay(delayTime, result);
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF),
        REASON, result);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest013 function end!");
}

/**
 * @tc.name: DisplayServiceTest014
 * @tc.desc: test set screen diaplay state
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest014, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest014 function start!");
    EXPECT_TRUE(g_service != nullptr);
    ASSERT_TRUE(g_brightnessServiceMock != nullptr);
    EXPECT_CALL(*g_brightnessServiceMock, SetScreenOnBrightness());
    
    ErrCode ret = ERR_OK;
    g_isPermissionGranted = false;
    ret = g_service->SetScreenDisplayState(0, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON), 0);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_SYSTEM_API_DENIED));

    g_isPermissionGranted = true;
    ret = g_service->SetScreenDisplayState(0, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON), 0);
    EXPECT_EQ(ret, ERR_OK);
    ret = g_service->SetScreenDisplayState(0, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_OFF), 0);
    EXPECT_EQ(ret, ERR_OK);
    ret = g_service->SetScreenDisplayState(
        0, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN), 0);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));

    ret = g_service->SetScreenDisplayState(0, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        static_cast<uint32_t>(Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN));
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest014 function end!");
}

#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
/**
 * @tc.name: DisplayServiceTest032
 * @tc.desc: test DisplayPowerMgrService function SpecificScreenPowerStrategy
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest032, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest032 function start!");
    EXPECT_TRUE(g_service != nullptr);
    int32_t ret = 0;
    sptr<IRemoteObject> token = nullptr;
    g_isPermissionGranted = false;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_ALL),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_UNKNOWN), token, ret);
    EXPECT_FALSE(ret);
 
    g_isPermissionGranted = true;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPCAST), token, ret);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest032 function end!");
}
 
/**
 * @tc.name: DisplayServiceTest033
 * @tc.desc: test DisplayPowerMgrService function SpecificScreenPowerStrategy
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest033, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest033 function start!");
    EXPECT_TRUE(g_service != nullptr);
    int32_t ret = 0;
    sptr<MockRemoteObject> token = new MockRemoteObject();
    token->isProxyObject_ = true;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPCAST), token, ret);
    EXPECT_TRUE(ret);
    sptr<MockRemoteObject> tokenTwo = new MockRemoteObject();
    tokenTwo->isProxyObject_ = true;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
    static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPCAST), tokenTwo, ret);
    EXPECT_TRUE(ret);
    token->isProxyObject_ = false;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPCAST), token, ret);
    token = nullptr;
    tokenTwo = nullptr;
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest033 function end!");
}
 
/**
 * @tc.name: DisplayServiceTest034
 * @tc.desc: test DisplayPowerMgrService function OnRemoteDied
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest034, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest034 function start!");
    EXPECT_TRUE(g_service != nullptr);
    sptr<DisplayPowerMgrService::InvokerDeathRecipient> deathRecipient =
        sptr<DisplayPowerMgrService::InvokerDeathRecipient>::MakeSptr(__func__,
            [](const sptr<DisplayPowerMgrService>& service) {
            DISPLAY_HILOGE(COMP_SVC, "client dead! reset specific screen power strategy");
        });
    EXPECT_NE(deathRecipient, nullptr);
    wptr<IRemoteObject> remoteObj = new MockRemoteObject();
    deathRecipient->OnRemoteDied(remoteObj);
    remoteObj = nullptr;
    deathRecipient->OnRemoteDied(remoteObj);
    EXPECT_EQ(remoteObj, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest034 function end!");
}
#endif
} // namespace
