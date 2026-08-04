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
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
#include <chrono>
#include <cinttypes>
#include <thread>
#include <common_event_data.h>
#include <common_event_manager.h>
#include <common_event_publish_info.h>
#include <want.h>
#include "imulti_screen_display_state_callback.h"
#include "multi_screen_display_state_callback_manager.h"
#include "screen_controller.h"
#endif
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
bool g_isMock = false;
NiceMock<DisplayServiceTest::BrightnessServiceMock>* g_brightnessServiceMock;
NiceMock<DisplayServiceTest::BrightnessServiceMock>* g_mock;
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
bool g_isNativePermissionGranted = true;
constexpr uint64_t MAIN_SCREEN_ID = 0;
constexpr uint64_t SECOND_SCREEN_ID = 1;
constexpr uint64_t THIRD_SCREEN_ID = 2;
constexpr uint64_t FOURTH_SCREEN_ID = 3;
constexpr uint64_t FIFTH_SCREEN_ID = 4;
constexpr uint64_t INVALID_SCREEN_ID = UINT64_MAX - 1;
constexpr uint32_t DEFAULT_REASON = 0;
constexpr const char* TEST_SCREEN_NAME = "testScreenName";
constexpr uint32_t MAX_SCREEN_NAME_LENGTH = 100;
constexpr int CONCURRENCY_DELAY_MS = 10;
constexpr int MIN_CONCURRENCY_DEPTH = 3;
bool g_mockSetDisplayStateRet = true;
bool g_mockWakeUpBeginRet = true;
bool g_mockSuspendBeginRet = true;
bool g_mockWakeUpEndRet = true;
bool g_mockSuspendEndRet = true;
bool g_mockSetScreenPowerRet = true;
int g_publishEventCount = 0;
uint64_t g_lastPubScreenId = 0;
std::string g_lastPubScreenName;
std::string g_lastAction;
std::string g_lastPermission;
std::string g_lastPubReason;
std::atomic<int> g_concurrencyCount{0};
std::atomic<int> g_maxConcurrencyCount{0};
#endif
} // namespace

namespace OHOS::PowerMgr {
bool Permission::IsSystem()
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest IsSystem, g_isPermissionGranted: %{public}d", g_isPermissionGranted);
    return g_isPermissionGranted;
}

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
bool Permission::IsNativePermissionGranted(const std::string& perm)
{
    return g_isNativePermissionGranted;
}

void TestMultiScreenCallback::OnMultiScreenDisplayStateChanged(
    uint64_t screenId, const std::string& screenName,
    OHOS::DisplayPowerMgr::DisplayState state,
    OHOS::DisplayPowerMgr::MultiScreenStateChangeReason reason)
{
    lastScreenId_ = screenId;
    lastScreenName_ = screenName;
    lastState_ = state;
    lastReason_ = static_cast<uint32_t>(reason);
    callCount_++;
    DISPLAY_HILOGI(LABEL_TEST, "TestMultiScreenCallback: screenId=%{public}" PRIu64
        " screenName=%{public}s state=%{public}u reason=%{public}u count=%{public}d",
        screenId, screenName.c_str(), static_cast<uint32_t>(state), static_cast<uint32_t>(reason), callCount_);
}
#endif
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

DisplayId DisplayManagerLite::GetDefaultDisplayId(int32_t userId)
{
    if (g_isMock) {
        constexpr DisplayId mockId = 3308;
        return mockId;
    }
    return DISPLAY_MAIN_ID;
}

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
bool DisplayManagerLite::SetDisplayState(DisplayId displayId, Rosen::DisplayState state, DisplayStateCallback callback)
{
    if (g_mockSetDisplayStateRet) {
        int count = g_concurrencyCount.fetch_add(1, std::memory_order_relaxed) + 1;
        int prev = g_maxConcurrencyCount.load(std::memory_order_relaxed);
        while (count > prev &&
               !g_maxConcurrencyCount.compare_exchange_weak(prev, count,
                   std::memory_order_relaxed)) {}
        // Small delay to widen concurrency window
        std::this_thread::sleep_for(std::chrono::milliseconds(CONCURRENCY_DELAY_MS));
        if (callback) {
            callback(state);
        }
        g_concurrencyCount.fetch_sub(1, std::memory_order_relaxed);
        return true;
    }
    return false;
}

bool DisplayManagerLite::WakeUpBegin(DisplayId displayId, PowerStateChangeReason reason)
{
    return g_mockWakeUpBeginRet;
}

bool DisplayManagerLite::SuspendBegin(DisplayId displayId, PowerStateChangeReason reason)
{
    return g_mockSuspendBeginRet;
}

bool DisplayManagerLite::WakeUpEnd(DisplayId displayId)
{
    return g_mockWakeUpEndRet;
}

bool DisplayManagerLite::SuspendEnd(DisplayId displayId)
{
    return g_mockSuspendEndRet;
}

bool ScreenManagerLite::SetScreenPowerForSpecifiedId(ScreenId screenId, ScreenPowerState state,
    PowerStateChangeReason reason)
{
    return g_mockSetScreenPowerRet;
}

ScreenPowerState ScreenManagerLite::GetScreenPower(ScreenId screenId)
{
    return g_powerState;
}
#endif
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
    g_service->autoBrightnessQueue_.Clear(); // 2026.08.13: this newly added ffrtTimer is not cleared

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

bool DisplayManagerLite::SetScreenBrightness(const DmsScreenBrightnessData& brightnessData)
{
    return true;
}

ScreenPowerState ScreenManagerLite::GetScreenPower()
{
    return g_powerState;
}
} //namespace OHOS::Rosen

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
namespace OHOS::EventFwk {
bool CommonEventManager::PublishCommonEvent(const CommonEventData& data,
    const CommonEventPublishInfo& publishInfo)
{
    g_publishEventCount++;
    auto want = data.GetWant();
    g_lastPubScreenId = static_cast<uint64_t>(want.GetLongParam("screenId", -1L));
    g_lastPubScreenName = want.GetStringParam("screenName");
    g_lastAction = want.GetAction();
    g_lastPubReason = want.GetStringParam("reason");
    auto perms = publishInfo.GetSubscriberPermissions();
    g_lastPermission = perms.empty() ? "" : perms[0];
    return true;
}
} // namespace OHOS::EventFwk
#endif

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
 * @tc.name: DisplayServiceTest031
 * @tc.desc: test set screen diaplay state
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest031, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest031 function start!");
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
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest031 function end!");
}

#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
/**
 * @tc.name: DisplayServiceTest032
 * @tc.desc: test DisplayPowerMgrService function SetScreenPowerOffStrategy
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
    EXPECT_EQ(ret, static_cast<int32_t>(DisplayErrors::ERR_SYSTEM_API_DENIED));

    g_isPermissionGranted = true;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_WIRED_APPCAST), token, ret);
    EXPECT_EQ(ret, static_cast<int32_t>(DisplayErrors::ERR_OK));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest032 function end!");
}
 
/**
 * @tc.name: DisplayServiceTest033
 * @tc.desc: test DisplayPowerMgrService function SetScreenPowerOffStrategy
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
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_WIRED_APPCAST), token, ret);
    EXPECT_EQ(ret, static_cast<int32_t>(DisplayErrors::ERR_OK));
    sptr<MockRemoteObject> tokenTwo = new MockRemoteObject();
    tokenTwo->isProxyObject_ = true;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
    static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_WIRED_APPCAST), tokenTwo, ret);
    EXPECT_EQ(ret, static_cast<int32_t>(DisplayErrors::ERR_OK));
    token->isProxyObject_ = false;
    g_service->SetScreenPowerOffStrategy(static_cast<uint32_t>(PowerOffStrategy::STRATEGY_SPECIFIC),
        static_cast<uint32_t>(PowerMgr::StateChangeReason::STATE_CHANGE_REASON_WIRED_APPCAST), token, ret);
    token = nullptr;
    tokenTwo = nullptr;
    EXPECT_EQ(ret, static_cast<int32_t>(DisplayErrors::ERR_OK));
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
    wptr<IRemoteObject> mockRemote = new MockRemoteObject();
    sptr<DisplayPowerMgrService::InvokerDeathRecipient> nullDeathRecipient =
        sptr<DisplayPowerMgrService::InvokerDeathRecipient>::MakeSptr(__func__, nullptr);
    nullDeathRecipient->OnRemoteDied(mockRemote);
    mockRemote = nullptr;
    remoteObj = nullptr;
    deathRecipient->OnRemoteDied(remoteObj);
    EXPECT_EQ(remoteObj, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest034 function end!");
}
#endif

/**
 * @tc.name: DisplayServiceTest036
 * @tc.desc: Test AutoAdjustBrightness returns false without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest036, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest036 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = false;
    g_service->HandleBootBrightness();
    bool result = false;
    g_service->AutoAdjustBrightness(true, result);
    EXPECT_FALSE(result);
    g_service->AutoAdjustBrightness(false, result);
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest036 function end!");
}

/**
 * @tc.name: DisplayServiceTest037
 * @tc.desc: Test AutoAdjustBrightness with permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest037, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest037 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    g_service->HandleBootBrightness();

    bool result = false;
    g_service->AutoAdjustBrightness(true, result);
    EXPECT_EQ(g_service->IsSupportLightSensor(), result);
    result = false;
    g_service->AutoAdjustBrightness(false, result);
    EXPECT_EQ(g_service->IsSupportLightSensor(), result);

    g_service->UnregisterSettingObservers();
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest037 function end!");
}

/**
 * @tc.name: DisplayServiceTest038
 * @tc.desc: Test RestoreBrightness with and without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest038, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest038 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = false;
    bool result = false;
    g_service->RestoreBrightness(DISPLAY_MAIN_ID, DEFAULT_DURATION, result);
    EXPECT_FALSE(result);
    g_isPermissionGranted = true;
    auto ret = g_service->RestoreBrightness(DISPLAY_MAIN_ID, DEFAULT_DURATION, result);
    EXPECT_EQ(ret, ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest038 function end!");
}

/**
 * @tc.name: DisplayServiceTest039
 * @tc.desc: Test RegisterBootCompletedCallback does not crash
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest039, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest039 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_service->RegisterBootCompletedCallback();
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest039 function end!");
}

/**
 * @tc.name: DisplayServiceTest040
 * @tc.desc: Test UpdateScreenPowerState with isScreenOn=true
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest040, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest040 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool result = false;
    auto ret = g_service->UpdateScreenPowerState(true, result);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest040 function end!");
}

/**
 * @tc.name: DisplayServiceTest041
 * @tc.desc: Test UpdateScreenPowerState with isScreenOn=false
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest041, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest041 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool result = false;
    auto ret = g_service->UpdateScreenPowerState(false, result);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest041 function end!");
}

/**
 * @tc.name: DisplayServiceTest042
 * @tc.desc: Test UpdateScreenPowerState fallback to mainId when DEFAULT_DISPLAY_ID not found
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest042, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest042 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool result = false;
    auto controllerMap = g_service->controllerMap_;
    g_service->controllerMap_.clear();
    auto ret = g_service->UpdateScreenPowerState(true, result);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(result);
    g_isMock = true;
    constexpr int32_t mockId = 3308;
    g_service->controllerMap_.emplace(mockId, std::make_shared<ScreenController>(mockId));
    ret = g_service->UpdateScreenPowerState(true, result);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(result);
    g_isMock = false;
    g_service->controllerMap_ = controllerMap;
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest042 function end!");
}

/**
 * @tc.name: DisplayServiceTest044
 * @tc.desc: Test GetFeatureSupport via service without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest044, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest044 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = false;
    bool result = false;
    auto ret = g_service->GetFeatureSupport(BrightnessFeatureType::DEFAULT, result);
    EXPECT_NE(ret, ERR_OK);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest044 function end!");
}

/**
 * @tc.name: DisplayServiceTest045
 * @tc.desc: Test GetFeatureSupport via service with permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest045, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest045 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    bool result = false;
    auto ret = g_service->GetFeatureSupport(BrightnessFeatureType::DEFAULT, result);
    EXPECT_EQ(ret, ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest045 function end!");
}

/**
 * @tc.name: DisplayServiceTest046
 * @tc.desc: Test SetForcedBrightness via service with permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest046, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest046 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    bool result = false;
    auto ret = g_service->SetForcedBrightness(0.5, DISPLAY_MAIN_ID, DEFAULT_DURATION,
        BrightnessValueType::RELATIVE_TO_CURRENT_RANGE, result);
    EXPECT_EQ(ret, ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest046 function end!");
}

/**
 * @tc.name: DisplayServiceTest047
 * @tc.desc: Test SetForcedBrightness via service without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest047, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest047 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = false;
    bool result = false;
    auto ret = g_service->SetForcedBrightness(0.5, DISPLAY_MAIN_ID, DEFAULT_DURATION,
        BrightnessValueType::RELATIVE_TO_CURRENT_RANGE, result);
    EXPECT_NE(ret, ERR_OK);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest047 function end!");
}

/**
 * @tc.name: DisplayServiceTest048
 * @tc.desc: Test SetForcedBrightness with invalid valueType
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest048, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest048 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    bool result = false;
    // valueType < DEFAULT
    auto valueTypeNum = static_cast<int>(BrightnessValueType::DEFAULT) - 1;
    auto valueType = static_cast<BrightnessValueType>(valueTypeNum);
    auto ret = g_service->SetForcedBrightness(0.5, DISPLAY_MAIN_ID, DEFAULT_DURATION, valueType, result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));

    // valueType >= MAX
    valueTypeNum = static_cast<int>(BrightnessValueType::MAX) + 1;
    valueType = static_cast<BrightnessValueType>(valueTypeNum);
    ret = g_service->SetForcedBrightness(0.5, DISPLAY_MAIN_ID, DEFAULT_DURATION, valueType, result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest048 function end!");
}

/**
 * @tc.name: DisplayServiceTest049
 * @tc.desc: Test GetFeatureSupport with invalid feature
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest049, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest049 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    bool result = false;

    // feature < DEFAULT
    auto featureNum = static_cast<int>(BrightnessFeatureType::DEFAULT) - 1;
    auto feature = static_cast<BrightnessFeatureType>(featureNum);
    auto ret = g_service->GetFeatureSupport(feature, result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));

    // feature >= MAX
    featureNum = static_cast<int>(BrightnessFeatureType::MAX) + 1;
    feature = static_cast<BrightnessFeatureType>(featureNum);
    ret = g_service->GetFeatureSupport(feature, result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest049 function end!");
}

/**
 * @tc.name: DisplayServiceTest050
 * @tc.desc: Test RegisterDataChangeListener and UnregisterDataChangeListener with invalid listenerType
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest050, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest050 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    int32_t result = 0;

    // listenerType < DEFAULT
    auto listenerTypeNum = static_cast<int>(DisplayDataChangeListenerType::DEFAULT) - 1;
    auto listenerType = static_cast<DisplayDataChangeListenerType>(listenerTypeNum);
    auto ret = g_service->RegisterDataChangeListener(nullptr, listenerType, "test", "", result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    ret = g_service->UnregisterDataChangeListener(listenerType, "test", result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));

    // listenerType >= MAX
    listenerTypeNum = static_cast<int>(DisplayDataChangeListenerType::MAX) + 1;
    listenerType = static_cast<DisplayDataChangeListenerType>(listenerTypeNum);
    ret = g_service->RegisterDataChangeListener(nullptr, listenerType, "test", "", result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    ret = g_service->UnregisterDataChangeListener(listenerType, "test", result);
    EXPECT_EQ(ret, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest050 function end!");
}

/**
 * @tc.name: DisplayServiceTest051
 * @tc.desc: Test UndoSetDisplayStateInner
 * @tc.type: FUNC
 */
extern "C" {
void __wrap__ZN4OHOS15DisplayPowerMgr17BrightnessManager21SetScreenOnBrightnessEv(BrightnessManager*)
{
    DISPLAY_HILOGI(LABEL_TEST, "Mock SetScreenOnBrightness called");
    // Use existing mock class but won't interfere with the existing (broken) tests:
    // The leaked mock object is left leaking, sigh
    if (!g_mock) {
        // If mock is unavailable just do nothing. The side effect is non-relevant in this test suite.
        // In case the original behaviour must be restored, use __real_<func>
        DISPLAY_HILOGE(LABEL_TEST, "Mock object is nullptr, do nothing");
        return;
    }
    g_mock->SetScreenOnBrightness();
}
}

HWTEST_F(DisplayServiceTest, UndoSetDisplayStateInner, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UndoSetDisplayStateInner function start!");
    // Reuse the existing mock class, although the redefined function is inside manager instead of service
    NiceMock<DisplayServiceTest::BrightnessServiceMock> mockObj;
    g_mock = &mockObj;
    ASSERT_TRUE(g_service != nullptr);
    EXPECT_CALL(mockObj, SetScreenOnBrightness).Times(2);
    g_service->UndoSetDisplayStateInner(0, DisplayPowerMgr::DisplayState::DISPLAY_ON, 0);
    g_service->UndoSetDisplayStateInner(0, DisplayPowerMgr::DisplayState::DISPLAY_DIM, 0);
    EXPECT_CALL(mockObj, SetScreenOnBrightness).Times(0);
    g_service->UndoSetDisplayStateInner(0, DisplayPowerMgr::DisplayState::DISPLAY_OFF, 0);
    g_mock = nullptr;
    DISPLAY_HILOGI(LABEL_TEST, "UndoSetDisplayStateInner function end!");
}

/**
 * @tc.name: DisplayServiceTest052
 * @tc.desc: Test SetSceneMode via service with permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest052, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest052 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    bool result = false;
    auto errCode = g_service->SetSceneMode(DISPLAY_MAIN_ID, SceneModeType::SCENE_MODE_BUSINESS, true, result);
    EXPECT_EQ(errCode, ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest052 function end!");
}

/**
 * @tc.name: DisplayServiceTest053
 * @tc.desc: Test SetSceneMode via service without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest053, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest053 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = false;
    bool result = false;
    auto errCode = g_service->SetSceneMode(DISPLAY_MAIN_ID, SceneModeType::SCENE_MODE_BUSINESS, true, result);
    EXPECT_NE(errCode, ERR_OK);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest053 function end!");
}

/**
 * @tc.name: DisplayServiceTest054
 * @tc.desc: Test SetSceneMode with invalid SceneModeType
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, DisplayServiceTest054, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest054 function start!");
    EXPECT_TRUE(g_service != nullptr);
    g_isPermissionGranted = true;
    bool result = false;

    auto typeNum = static_cast<int>(SceneModeType::DEFAULT) - 1;
    auto type = static_cast<SceneModeType>(typeNum);
    auto errCode = g_service->SetSceneMode(DISPLAY_MAIN_ID, type, true, result);
    EXPECT_EQ(errCode, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));

    typeNum = static_cast<int>(SceneModeType::MAX) + 1;
    type = static_cast<SceneModeType>(typeNum);
    errCode = g_service->SetSceneMode(DISPLAY_MAIN_ID, type, true, result);
    EXPECT_EQ(errCode, static_cast<ErrCode>(DisplayErrors::ERR_PARAM_INVALID));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayServiceTest054 function end!");
}
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest001
 * @tc.desc: Test SetMultiScreenDisplayStateInner without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest001 function start!");
    g_isPermissionGranted = false;
    DisplayErrors ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_SYSTEM_API_DENIED);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest001 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest002
 * @tc.desc: Test SetMultiScreenDisplayStateInner without MULTI_SCREEN_MANAGER permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest002 function start!");
    g_isPermissionGranted = true;
    g_isNativePermissionGranted = false;
    DisplayErrors ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PERMISSION_DENIED);
    g_isNativePermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest002 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest003
 * @tc.desc: Test SetMultiScreenDisplayStateInner with invalid state or screenName
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest003 function start!");
    DisplayErrors ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_DIM, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_SUSPEND, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_DOZE, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_DOZE_SUSPEND, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);

    std::string overlongScreenName(MAX_SCREEN_NAME_LENGTH + 1, 'a');
    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, overlongScreenName, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest003 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest004
 * @tc.desc: Test SetMultiScreenDisplayStateInner skip same state: no callback triggered
 *           and no common event published for same-state Set.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest004, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest004 function start!");

    DisplayErrors ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);

    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    ASSERT_EQ(DisplayErrors::ERR_OK,
        g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID));
    EXPECT_EQ(cb->callCount_, 0);

    g_publishEventCount = 0;
    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);
    EXPECT_EQ(cb->callCount_, 0);
    EXPECT_EQ(g_publishEventCount, 0);

    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest004 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest005
 * @tc.desc: Test SetMultiScreenDisplayStateInner with UpdateMultiScreenState failure
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest005, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest005 function start!");
    g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);

    g_mockSetDisplayStateRet = false;
    DisplayErrors ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_STATE_CHANGE_FAILED);
    g_mockSetDisplayStateRet = true;
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest005 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest006
 * @tc.desc: Test SetMultiScreenDisplayStateInner normal ON and OFF flow
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest006, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest006 function start!");

    DisplayErrors ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);
    DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    g_service->GetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, state);
    EXPECT_EQ(state, DisplayPowerMgr::DisplayState::DISPLAY_ON);

    ret = g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);
    g_service->GetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, state);
    EXPECT_EQ(state, DisplayPowerMgr::DisplayState::DISPLAY_OFF);

    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest006 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest007
 * @tc.desc: Test multi-screen independent state control: ON->OFF, OFF->ON, ON->ON, OFF->OFF, ON->OFF
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest007, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest007 function start!");

    g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    g_service->SetMultiScreenDisplayStateInner(
        SECOND_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
    g_service->SetMultiScreenDisplayStateInner(
        THIRD_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    g_service->SetMultiScreenDisplayStateInner(
        FOURTH_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
    g_service->SetMultiScreenDisplayStateInner(
        FIFTH_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);

    constexpr int SCREEN_COUNT = 5;
    uint64_t screenIds[SCREEN_COUNT] = {
        MAIN_SCREEN_ID, SECOND_SCREEN_ID, THIRD_SCREEN_ID, FOURTH_SCREEN_ID, FIFTH_SCREEN_ID};
    DisplayPowerMgr::DisplayState targets[SCREEN_COUNT] = {
        DisplayPowerMgr::DisplayState::DISPLAY_OFF,
        DisplayPowerMgr::DisplayState::DISPLAY_ON,
        DisplayPowerMgr::DisplayState::DISPLAY_ON,
        DisplayPowerMgr::DisplayState::DISPLAY_OFF,
        DisplayPowerMgr::DisplayState::DISPLAY_OFF,
    };

    std::thread threads[SCREEN_COUNT];
    DisplayErrors results[SCREEN_COUNT];
    for (int i = 0; i < SCREEN_COUNT; i++) {
        threads[i] = std::thread([&, i]() {
            results[i] = g_service->SetMultiScreenDisplayStateInner(
                screenIds[i], TEST_SCREEN_NAME, targets[i], DEFAULT_REASON);
        });
    }
    for (int i = 0; i < SCREEN_COUNT; i++) {
        threads[i].join();
    }
    for (int i = 0; i < SCREEN_COUNT; i++) {
        EXPECT_EQ(results[i], DisplayErrors::ERR_OK);
    }

    for (int i = 0; i < SCREEN_COUNT; i++) {
        DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
        g_service->GetMultiScreenDisplayStateInner(screenIds[i], state);
        EXPECT_EQ(state, targets[i]);
    }

    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest007 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest008
 * @tc.desc: Test concurrent SetMultiScreenDisplayState for 5 different screens.
 *           Uses concurrency depth counter in SetDisplayState mock to verify
 *           that different screens execute in parallel (different mutexes).
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest008, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest008 function start!");
    constexpr int SCREEN_COUNT = 5;
    for (uint64_t i = 0; i < SCREEN_COUNT; i++) {
        g_service->SetMultiScreenDisplayStateInner(i, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON,
            DEFAULT_REASON);
    }
    g_maxConcurrencyCount = 0;
    std::thread threads[SCREEN_COUNT];
    DisplayErrors results[SCREEN_COUNT];
    for (int i = 0; i < SCREEN_COUNT; i++) {
        threads[i] = std::thread([&, i]() {
            results[i] = g_service->SetMultiScreenDisplayStateInner(
                static_cast<uint64_t>(i), TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
        });
    }
    for (int i = 0; i < SCREEN_COUNT; i++) {
        threads[i].join();
    }
    for (int i = 0; i < SCREEN_COUNT; i++) {
        EXPECT_EQ(results[i], DisplayErrors::ERR_OK) << "screen " << i << " failed";
    }
    for (uint64_t i = 0; i < SCREEN_COUNT; i++) {
        DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
        g_service->GetMultiScreenDisplayStateInner(i, state);
        EXPECT_EQ(state, DisplayPowerMgr::DisplayState::DISPLAY_OFF);
    }
    EXPECT_GE(g_maxConcurrencyCount.load(), MIN_CONCURRENCY_DEPTH);
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest008 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest009
 * @tc.desc: Test concurrent SetMultiScreenDisplayState for the same screen is serialized by mutex.
 *           Uses concurrency depth counter to verify that only 1 thread enters
 *           SetDisplayState at a time for the same screen.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest009, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest009 function start!");

    g_service->SetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, TEST_SCREEN_NAME,
        DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);

    constexpr int THREAD_COUNT = 4;
    std::atomic<int> doneCount(0);
    DisplayErrors results[THREAD_COUNT];
    DisplayPowerMgr::DisplayState states[THREAD_COUNT] = {
        DisplayPowerMgr::DisplayState::DISPLAY_OFF,
        DisplayPowerMgr::DisplayState::DISPLAY_ON,
        DisplayPowerMgr::DisplayState::DISPLAY_OFF,
        DisplayPowerMgr::DisplayState::DISPLAY_ON,
    };

    g_maxConcurrencyCount = 0;

    std::thread threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i] = std::thread([&, i]() {
            results[i] = g_service->SetMultiScreenDisplayStateInner(
                MAIN_SCREEN_ID, TEST_SCREEN_NAME, states[i], static_cast<uint32_t>(i));
            doneCount.fetch_add(1);
        });
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i].join();
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        EXPECT_EQ(results[i], DisplayErrors::ERR_OK) << "thread " << i << " failed";
    }

    EXPECT_EQ(doneCount.load(), THREAD_COUNT);
    EXPECT_EQ(g_maxConcurrencyCount.load(), 1);

    DisplayPowerMgr::DisplayState finalState = DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    g_service->GetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, finalState);
    EXPECT_TRUE(finalState == DisplayPowerMgr::DisplayState::DISPLAY_ON ||
                finalState == DisplayPowerMgr::DisplayState::DISPLAY_OFF);

    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest009 function end!");
}

/**
 * @tc.name: SetMultiScreenDisplayStateInnerTest010
 * @tc.desc: Test no callback or common event loss under concurrent Set: 3 callbacks each register
 *           for 5 screens, concurrent Set triggers all callbacks and common events.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, SetMultiScreenDisplayStateInnerTest010, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest010 function start!");

    constexpr int SCREEN_COUNT = 5;
    for (uint64_t i = 0; i < SCREEN_COUNT; i++) {
        g_service->SetMultiScreenDisplayStateInner(i, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON,
            DEFAULT_REASON);
    }

    sptr<TestMultiScreenCallback> cb0 = new TestMultiScreenCallback(true);
    sptr<TestMultiScreenCallback> cb1 = new TestMultiScreenCallback(true);
    sptr<TestMultiScreenCallback> cb2 = new TestMultiScreenCallback(true);
    for (uint64_t i = 0; i < SCREEN_COUNT; i++) {
        ASSERT_EQ(DisplayErrors::ERR_OK,
            g_service->RegisterMultiScreenDisplayStateCallbackInner(cb0, i));
        ASSERT_EQ(DisplayErrors::ERR_OK,
            g_service->RegisterMultiScreenDisplayStateCallbackInner(cb1, i));
        ASSERT_EQ(DisplayErrors::ERR_OK,
            g_service->RegisterMultiScreenDisplayStateCallbackInner(cb2, i));
    }

    g_publishEventCount = 0;
    std::thread threads[SCREEN_COUNT];
    DisplayErrors results[SCREEN_COUNT];
    for (int i = 0; i < SCREEN_COUNT; i++) {
        threads[i] = std::thread([&, i]() {
            results[i] = g_service->SetMultiScreenDisplayStateInner(
                static_cast<uint64_t>(i), TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
        });
    }
    for (int i = 0; i < SCREEN_COUNT; i++) {
        threads[i].join();
    }
    for (int i = 0; i < SCREEN_COUNT; i++) {
        EXPECT_EQ(results[i], DisplayErrors::ERR_OK);
    }

    EXPECT_EQ(cb0->callCount_, SCREEN_COUNT);
    EXPECT_EQ(cb1->callCount_, SCREEN_COUNT);
    EXPECT_EQ(cb2->callCount_, SCREEN_COUNT);
    EXPECT_EQ(g_publishEventCount, SCREEN_COUNT);

    g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb0, SCREEN_ID_ALL);
    g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb1, SCREEN_ID_ALL);
    g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb2, SCREEN_ID_ALL);

    DISPLAY_HILOGI(LABEL_TEST, "SetMultiScreenDisplayStateInnerTest010 function end!");
}

/**
 * @tc.name: GetMultiScreenDisplayStateInnerTest001
 * @tc.desc: Test GetMultiScreenDisplayStateInner without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, GetMultiScreenDisplayStateInnerTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMultiScreenDisplayStateInnerTest001 function start!");
    g_isPermissionGranted = false;
    DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_ON;
    DisplayErrors ret = g_service->GetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, state);
    EXPECT_EQ(ret, DisplayErrors::ERR_SYSTEM_API_DENIED);
    EXPECT_EQ(state, DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "GetMultiScreenDisplayStateInnerTest001 function end!");
}

/**
 * @tc.name: GetMultiScreenDisplayStateInnerTest002
 * @tc.desc: Test GetMultiScreenDisplayStateInner without MULTI_SCREEN_MANAGER permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, GetMultiScreenDisplayStateInnerTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMultiScreenDisplayStateInnerTest002 function start!");
    g_isPermissionGranted = true;
    g_isNativePermissionGranted = false;
    DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_ON;
    DisplayErrors ret = g_service->GetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, state);
    EXPECT_EQ(ret, DisplayErrors::ERR_PERMISSION_DENIED);
    EXPECT_EQ(state, DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN);
    g_isNativePermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "GetMultiScreenDisplayStateInnerTest002 function end!");
}

/**
 * @tc.name: GetMultiScreenDisplayStateInnerTest003
 * @tc.desc: Test GetMultiScreenDisplayStateInner successfully
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, GetMultiScreenDisplayStateInnerTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMultiScreenDisplayStateInnerTest003 function start!");
    g_service->SetMultiScreenDisplayStateInner(
        MAIN_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    DisplayPowerMgr::DisplayState state = DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN;
    DisplayErrors ret = g_service->GetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, state);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);
    EXPECT_EQ(state, DisplayPowerMgr::DisplayState::DISPLAY_ON);
    DISPLAY_HILOGI(LABEL_TEST, "GetMultiScreenDisplayStateInnerTest003 function end!");
}

/**
 * @tc.name: RegisterMultiScreenDisplayStateCallbackInnerTest001
 * @tc.desc: Test RegisterMultiScreenDisplayStateCallbackInner without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, RegisterMultiScreenDisplayStateCallbackInnerTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest001 function start!");
    g_isPermissionGranted = false;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    DisplayErrors ret = g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_SYSTEM_API_DENIED);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest001 function end!");
}

/**
 * @tc.name: RegisterMultiScreenDisplayStateCallbackInnerTest002
 * @tc.desc: Test RegisterMultiScreenDisplayStateCallbackInner without MULTI_SCREEN_MANAGER permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, RegisterMultiScreenDisplayStateCallbackInnerTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest002 function start!");
    g_isPermissionGranted = true;
    g_isNativePermissionGranted = false;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    DisplayErrors ret = g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_PERMISSION_DENIED);
    g_isNativePermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest002 function end!");
}

/**
 * @tc.name: RegisterMultiScreenDisplayStateCallbackInnerTest003
 * @tc.desc: Test RegisterMultiScreenDisplayStateCallbackInner with null callback
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, RegisterMultiScreenDisplayStateCallbackInnerTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest003 function start!");
    DisplayErrors ret = g_service->RegisterMultiScreenDisplayStateCallbackInner(nullptr, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest003 function end!");
}

/**
 * @tc.name: RegisterMultiScreenDisplayStateCallbackInnerTest004
 * @tc.desc: Test Register/Unregister with non-proxy, proxy, duplicate, and verify callback
 *           triggered by Set with correct parameters; after unregister, Set does not trigger.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, RegisterMultiScreenDisplayStateCallbackInnerTest004, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest004 function start!");
    sptr<TestMultiScreenCallback> nonProxyCb = new TestMultiScreenCallback(false);
    DisplayErrors ret = g_service->RegisterMultiScreenDisplayStateCallbackInner(nonProxyCb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_REGISTRATION_FAILED);

    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    ret = g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);

    g_service->SetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, TEST_SCREEN_NAME,
        DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
    EXPECT_EQ(cb->callCount_, 1);
    EXPECT_EQ(cb->lastScreenId_, MAIN_SCREEN_ID);
    EXPECT_EQ(cb->lastState_, DisplayPowerMgr::DisplayState::DISPLAY_OFF);
    EXPECT_EQ(cb->lastReason_, DEFAULT_REASON);

    ret = g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);

    ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);

    int callCountAfterUnregister = cb->callCount_;
    g_service->SetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, TEST_SCREEN_NAME,
        DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(cb->callCount_, callCountAfterUnregister);

    ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "RegisterMultiScreenDisplayStateCallbackInnerTest004 function end!");
}

/**
 * @tc.name: UnregisterMultiScreenDisplayStateCallbackInnerTest001
 * @tc.desc: Test UnregisterMultiScreenDisplayStateCallbackInner without permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, UnregisterMultiScreenDisplayStateCallbackInnerTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest001 function start!");
    g_isPermissionGranted = false;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    DisplayErrors ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_SYSTEM_API_DENIED);
    g_isPermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest001 function end!");
}

/**
 * @tc.name: UnregisterMultiScreenDisplayStateCallbackInnerTest002
 * @tc.desc: Test UnregisterMultiScreenDisplayStateCallbackInner without MULTI_SCREEN_MANAGER permission
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, UnregisterMultiScreenDisplayStateCallbackInnerTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest002 function start!");
    g_isPermissionGranted = true;
    g_isNativePermissionGranted = false;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    DisplayErrors ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_PERMISSION_DENIED);
    g_isNativePermissionGranted = true;
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest002 function end!");
}

/**
 * @tc.name: UnregisterMultiScreenDisplayStateCallbackInnerTest003
 * @tc.desc: Test UnregisterMultiScreenDisplayStateCallbackInner with null callback
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, UnregisterMultiScreenDisplayStateCallbackInnerTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest003 function start!");
    DisplayErrors ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(nullptr, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest003 function end!");
}

/**
 * @tc.name: UnregisterMultiScreenDisplayStateCallbackInnerTest004
 * @tc.desc: Test Unregister with SCREEN_ID_ALL and specific screen; after unregister, Set does not trigger callback.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, UnregisterMultiScreenDisplayStateCallbackInnerTest004, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest004 function start!");
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);

    EXPECT_EQ(g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, SCREEN_ID_ALL),
        DisplayErrors::ERR_OK);
    DisplayErrors ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb, SCREEN_ID_ALL);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);

    EXPECT_EQ(g_service->RegisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID),
        DisplayErrors::ERR_OK);
    ret = g_service->UnregisterMultiScreenDisplayStateCallbackInner(cb, MAIN_SCREEN_ID);
    EXPECT_EQ(ret, DisplayErrors::ERR_OK);

    g_service->SetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, TEST_SCREEN_NAME,
        DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
    EXPECT_EQ(cb->callCount_, 0);
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterMultiScreenDisplayStateCallbackInnerTest004 function end!");
}

/**
 * @tc.name: MultiScreenDisplayStateCallbackManagerTest001
 * @tc.desc: Test MultiScreenDisplayStateCallbackManager Register with null callback
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, MultiScreenDisplayStateCallbackManagerTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest001 function start!");
    MultiScreenDisplayStateCallbackManager mgr;
    EXPECT_FALSE(mgr.Register(nullptr, MAIN_SCREEN_ID));
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest001 function end!");
}

/**
 * @tc.name: MultiScreenDisplayStateCallbackManagerTest002
 * @tc.desc: Test MultiScreenDisplayStateCallbackManager Unregister with null callback
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, MultiScreenDisplayStateCallbackManagerTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest002 function start!");
    MultiScreenDisplayStateCallbackManager mgr;
    EXPECT_FALSE(mgr.Unregister(nullptr, MAIN_SCREEN_ID));
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest002 function end!");
}

/**
 * @tc.name: MultiScreenDisplayStateCallbackManagerTest003
 * @tc.desc: Test MultiScreenDisplayStateCallbackManager Unregister with remaining and last entry
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, MultiScreenDisplayStateCallbackManagerTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest003 function start!");
    MultiScreenDisplayStateCallbackManager mgr;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    sptr<IRemoteObject> obj = cb->AsObject();

    mgr.callbacks_.emplace(obj, MAIN_SCREEN_ID);
    mgr.callbacks_.emplace(obj, SECOND_SCREEN_ID);
    sptr<MultiScreenDisplayStateCallbackManager::CallbackDeathRecipient> dr(
        new MultiScreenDisplayStateCallbackManager::CallbackDeathRecipient(mgr));
    mgr.deathRecipients_[obj] = dr;

    EXPECT_TRUE(mgr.Unregister(obj, MAIN_SCREEN_ID));
    EXPECT_EQ(mgr.callbacks_.size(), static_cast<size_t>(1));
    EXPECT_EQ(mgr.deathRecipients_.count(obj), static_cast<size_t>(1));

    EXPECT_TRUE(mgr.Unregister(obj, SECOND_SCREEN_ID));
    auto range = mgr.callbacks_.equal_range(obj);
    EXPECT_EQ(range.first, range.second);
    EXPECT_EQ(mgr.deathRecipients_.count(obj), static_cast<size_t>(0));
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest003 function end!");
}

/**
 * @tc.name: MultiScreenDisplayStateCallbackManagerTest004
 * @tc.desc: Test MultiScreenDisplayStateCallbackManager UnregisterAll with null callback
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, MultiScreenDisplayStateCallbackManagerTest004, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest004 function start!");
    MultiScreenDisplayStateCallbackManager mgr;
    EXPECT_FALSE(mgr.RemoveAll(nullptr));
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest004 function end!");
}

/**
 * @tc.name: MultiScreenDisplayStateCallbackManagerTest005
 * @tc.desc: Test MultiScreenDisplayStateCallbackManager RemoveAll with null and valid callback and OnRemoteDied
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, MultiScreenDisplayStateCallbackManagerTest005, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest005 function start!");
    MultiScreenDisplayStateCallbackManager mgr;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    sptr<TestMultiScreenCallback> cb2 = new TestMultiScreenCallback(true);
    sptr<IRemoteObject> obj = cb->AsObject();
    sptr<IRemoteObject> obj2 = cb2->AsObject();

    mgr.RemoveAll(nullptr);
    EXPECT_EQ(mgr.callbacks_.size(), static_cast<size_t>(0));

    mgr.callbacks_.emplace(obj, MAIN_SCREEN_ID);
    mgr.callbacks_.emplace(obj, SECOND_SCREEN_ID);
    mgr.callbacks_.emplace(obj2, MAIN_SCREEN_ID);
    mgr.deathRecipients_[obj] = nullptr;
    EXPECT_EQ(mgr.callbacks_.size(), static_cast<size_t>(3));

    mgr.RemoveAll(obj);
    EXPECT_EQ(mgr.callbacks_.size(), static_cast<size_t>(1));
    EXPECT_EQ(mgr.deathRecipients_.count(obj), static_cast<size_t>(0));
    mgr.callbacks_.clear();

    mgr.callbacks_.emplace(obj, MAIN_SCREEN_ID);
    mgr.callbacks_.emplace(obj, SECOND_SCREEN_ID);
    EXPECT_EQ(mgr.callbacks_.size(), static_cast<size_t>(2));
    sptr<MultiScreenDisplayStateCallbackManager::CallbackDeathRecipient> dr(
        new MultiScreenDisplayStateCallbackManager::CallbackDeathRecipient(mgr));
    mgr.deathRecipients_[obj] = dr;

    dr->OnRemoteDied(wptr<IRemoteObject>(obj));
    EXPECT_EQ(mgr.callbacks_.size(), static_cast<size_t>(0));
    EXPECT_EQ(mgr.deathRecipients_.size(), static_cast<size_t>(0u));
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest005 function end!");
}

/**
 * @tc.name: MultiScreenDisplayStateCallbackManagerTest006
 * @tc.desc: Test MultiScreenDisplayStateCallbackManager Notify with mixed screen id matching
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, MultiScreenDisplayStateCallbackManagerTest006, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest006 function start!");
    MultiScreenDisplayStateCallbackManager mgr;
    sptr<TestMultiScreenCallback> cb = new TestMultiScreenCallback(true);
    sptr<TestMultiScreenCallback> cbAll = new TestMultiScreenCallback(true);
    sptr<IRemoteObject> obj = cb->AsObject();
    sptr<IRemoteObject> objAll = cbAll->AsObject();

    mgr.callbacks_.emplace(obj, SECOND_SCREEN_ID);
    mgr.callbacks_.emplace(obj, FOURTH_SCREEN_ID);
    mgr.callbacks_.emplace(objAll, SCREEN_ID_ALL);

    mgr.Notify(SECOND_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);
    EXPECT_EQ(cb->callCount_, 1);
    EXPECT_EQ(cbAll->callCount_, 1);

    mgr.Notify(THIRD_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(cb->callCount_, 1);
    EXPECT_EQ(cbAll->callCount_, 2);

    mgr.Notify(FOURTH_SCREEN_ID, TEST_SCREEN_NAME, DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);
    EXPECT_EQ(cb->callCount_, 2);
    EXPECT_EQ(cbAll->callCount_, 3);
    mgr.callbacks_.clear();
    DISPLAY_HILOGI(LABEL_TEST, "MultiScreenDisplayStateCallbackManagerTest006 function end!");
}

/**
 * @tc.name: CommonEventTest001
 * @tc.desc: Test common event published with correct parameters after SetMultiScreenDisplayState
 * @tc.type: FUNC
 */
HWTEST_F(DisplayServiceTest, CommonEventTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "CommonEventTest001 function start!");

    g_publishEventCount = 0;
    g_service->SetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, TEST_SCREEN_NAME,
        DisplayPowerMgr::DisplayState::DISPLAY_ON, DEFAULT_REASON);

    EXPECT_EQ(g_publishEventCount, 1);
    EXPECT_EQ(g_lastAction, "usual.event.display.MULTI_SCREEN_ON");
    EXPECT_EQ(g_lastPubScreenId, MAIN_SCREEN_ID);
    EXPECT_EQ(g_lastPubReason, "DEFAULT");
    EXPECT_EQ(g_lastPermission, "ohos.permission.MULTI_SCREEN_MANAGER");

    g_publishEventCount = 0;
    g_service->SetMultiScreenDisplayStateInner(MAIN_SCREEN_ID, TEST_SCREEN_NAME,
        DisplayPowerMgr::DisplayState::DISPLAY_OFF, DEFAULT_REASON);

    EXPECT_EQ(g_publishEventCount, 1);
    EXPECT_EQ(g_lastAction, "usual.event.display.MULTI_SCREEN_OFF");
    EXPECT_EQ(g_lastPubScreenId, MAIN_SCREEN_ID);
    EXPECT_EQ(g_lastPubReason, "DEFAULT");
    EXPECT_EQ(g_lastPermission, "ohos.permission.MULTI_SCREEN_MANAGER");

    DISPLAY_HILOGI(LABEL_TEST, "CommonEventTest001 function end!");
}
#endif
} // namespace
