/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifdef GTEST
#define private   public
#define protected public
#endif
#include "display_power_mgr_service_test.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <vector>

#include "display_power_mgr_client.h"
#include "display_log.h"
#include "display_xcollie.h"
#include "mock_display_power_mgr_proxy.h"
#include "mock_display_remote_object.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
namespace {
sptr<IRemoteObject> g_testRemoteObj;
sptr<MockDisplayPowerMgrProxy> g_mockProxy;
}

void DisplayPowerMgrServiceTest::SetUp()
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    g_testRemoteObj = sptr<PowerMgr::MockDisplayRemoteObject>::MakeSptr(u"DisplayPowerMgrServiceTest");
    g_mockProxy = sptr<MockDisplayPowerMgrProxy>::MakeSptr(g_testRemoteObj);
}

void DisplayPowerMgrServiceTest::TearDown()
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    g_testRemoteObj = nullptr;
    g_mockProxy = nullptr;
}

void DisplayPowerMgrServiceTest::DisplayPowerMgrTestCallback::OnDisplayStateChanged(
    uint32_t displayId, DisplayPowerMgr::DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrTestCallback::OnDisplayStateChanged");
}

namespace {
/**
 * @tc.name: DisplayPowerMgrService001
 * @tc.desc: Test DisplayPowerMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService001 function start!");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sam != nullptr) << "DisplayPowerMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    EXPECT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService001 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService002
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService002 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService002 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService003
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService003 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    EXPECT_TRUE(ret);
    auto temp = DisplayPowerMgrClient::GetInstance().SetScreenOnBrightness();
    EXPECT_TRUE(temp);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService003 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService004
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService004 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService004 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService005
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService005 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService005 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService006
 * @tc.desc: Test set screen state unknown
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService006 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_UNKNOWN);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService006 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService007
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService007 function start!");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_ON);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService007 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService008
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService008, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService008 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_DIM);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService008 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService009
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService009, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService009 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService009 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService010
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService010, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService010 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_SUSPEND);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService010 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService011
 * @tc.desc: Test set screen state unknown
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService011, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService011 function start!");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_UNKNOWN);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_UNKNOWN);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService011 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService012
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService012 function start!");
    std::vector<uint32_t> ret = DisplayPowerMgrClient::GetInstance().GetDisplayIds();
    EXPECT_TRUE(ret.size() != 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService012 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService013
 * @tc.desc: Test GetMainDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService013 function start!");
    int32_t ret = DisplayPowerMgrClient::GetInstance().GetMainDisplayId();
    EXPECT_TRUE(ret == 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService013 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService014
 * @tc.desc: Test AutoAdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService014, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService014 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    if (ret) {
        DISPLAY_HILOGI(LABEL_TEST, "AutoAdjustBrightness: is supported");
        ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        EXPECT_TRUE(ret);
    } else {
        DISPLAY_HILOGI(LABEL_TEST, "AutoAdjustBrightness: is not supported");
        EXPECT_FALSE(ret);
    }
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService014 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService015
 * @tc.desc: Test GetError
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService015, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService015 function start!");
    DisplayErrors error = DisplayPowerMgrClient::GetInstance().GetError();
    DISPLAY_HILOGI(LABEL_TEST, "get error %{public}d", static_cast<int32_t>(error));
    EXPECT_TRUE(error == DisplayErrors::ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService015 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService016
 * @tc.desc: Test RegisterCallback(Input callback is nullptr, function return false)
 * @tc.type: FUNC
 * @tc.require: issueI63PTF
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService016, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService016 function start!");
    bool result = DisplayPowerMgrClient::GetInstance().RegisterCallback(nullptr);
    EXPECT_EQ(result, false);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService016 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService017
 * @tc.desc: Test OverrideDisplayOffDelay para
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService017, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService017 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(0);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService017 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService018
 * @tc.desc: Test prohibitting writing delay screen off time in screen off state
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService018, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService018 function start!");
    DisplayState State = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    if (State != DisplayState::DISPLAY_OFF) {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    }
    uint32_t delayTime = 10; //delay time is 10ms
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(delayTime);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService018 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService019
 * @tc.desc: Test srceen delay off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService019, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService019 function start!");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    uint32_t delayTime = 10; //delay time is 10ms
    DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(delayTime);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    DisplayState delayState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(delayState == DisplayState::DISPLAY_DELAY_OFF);
    //Callback for waiting window
    int sleepTime = 3; // sleep time is 3s
    sleep(sleepTime);
    DisplayState offState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(offState == DisplayState::DISPLAY_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService019 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService020
 * @tc.desc: Test srceen delay off interrupt
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService020, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService020 function start!");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    uint32_t delayTime = 200; //delay time is 200ms
    DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(delayTime);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    int sleepTime = 100000; // sleep time is 100ms
    usleep(sleepTime);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    sleepTime = 300000; // sleep time is 300ms
    usleep(sleepTime);
    DisplayState onState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(onState == DisplayState::DISPLAY_ON);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService020 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService021
 * @tc.desc: Test set coordinated and set display state
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService021, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService021 function start!");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    int sleepTime = 2;
    //Callback for waiting window 2s
    sleep(sleepTime);
    bool coordinated = true;
    auto ret = DisplayPowerMgrClient::GetInstance().SetCoordinated(coordinated);
    EXPECT_TRUE(ret);
    // Prepare test environment.
    // If the state of display is DISPLAY_ON, change it to DISPLAY_OFF.
    DisplayPowerMgrClient::GetInstance().SetDisplayState(
        DisplayState::DISPLAY_OFF, PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    sleep(sleepTime);
    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(
        DisplayState::DISPLAY_ON, PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_TRUE(ret);
    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(
        DisplayState::DISPLAY_OFF, PowerMgr::StateChangeReason::STATE_CHANGE_REASON_APPLICATION);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService021 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService022
 * @tc.desc: Test set display state in Pre-light the screen
 * @tc.type: FUNCs
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService022, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService022 function start!");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    int sleepTime = 2;
    //Callback for waiting window 2s
    sleep(sleepTime);
    DisplayState currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);

    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT);
    EXPECT_FALSE(ret);
    currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService022 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService023
 * @tc.desc: Test set display state in Pre-light the screen and auth success
 * @tc.type: FUNC
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService023, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService023 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    int sleepTime = 2;
    //Callback for waiting window 2s
    sleep(sleepTime);
    DisplayState currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);

    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_SUCCESS);
    EXPECT_TRUE(ret);
    currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_ON);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService023 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService024
 * @tc.desc: Test set display state in Pre-light the screen and auth failed but screen on
 * @tc.type: FUNC
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService024, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService024 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    int sleepTime = 2;
    //Callback for waiting window 2s
    sleep(sleepTime);
    DisplayState currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);

    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_ON);
    EXPECT_TRUE(ret);
    currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_ON);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService024 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService025
 * @tc.desc: Test set display state in Pre-light the screen and auth failed but screen on
 * @tc.type: FUNC
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService025, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService025 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    int sleepTime = 2;
    //Callback for waiting window 2s
    sleep(sleepTime);
    DisplayState currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);

    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF);
    sleep(sleepTime);
    EXPECT_FALSE(ret);
    currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService025 function end!");
}

/**
 * @tc.name: DisplayXCollie001
 * @tc.desc: Test functions DisplayXCollie default
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayXCollie001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie001 function start!");
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDisplayState");
    EXPECT_FALSE(displayXCollie.isCanceled_.load());
    displayXCollie.CancelDisplayXCollie();
    EXPECT_TRUE(displayXCollie.isCanceled_.load());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie001 function end!");
}

/**
 * @tc.name: DisplayXCollie002
 * @tc.desc: Test functions DisplayXCollie isRecovery = true
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayXCollie002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie002 function start!");
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDisplayState", true);
    EXPECT_FALSE(displayXCollie.isCanceled_.load());
    displayXCollie.CancelDisplayXCollie();
    EXPECT_TRUE(displayXCollie.isCanceled_.load());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie002 function end!");
}

/**
 * @tc.name: DisplayXCollie003
 * @tc.desc: Test functions DisplayXCollie empty logtag
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayXCollie003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie003 function start!");
    std::string emptyString;
    DisplayXCollie displayXCollie(emptyString, true);
    EXPECT_FALSE(displayXCollie.isCanceled_.load());
    displayXCollie.CancelDisplayXCollie();
    EXPECT_TRUE(displayXCollie.isCanceled_.load());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie003 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient001
 * @tc.desc: test SetDisplayState() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient001 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    bool ret = client.SetDisplayState(DisplayState::DISPLAY_OFF);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient001 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient002
 * @tc.desc: test GetDisplayState() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient002 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto displayState = client.GetDisplayState();
    client.proxy_ = proxy;
    EXPECT_EQ(displayState, DisplayState::DISPLAY_UNKNOWN);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient002 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient003
 * @tc.desc: test GetDisplayIds() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient003 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ids = client.GetDisplayIds();
    client.proxy_ = proxy;
    EXPECT_TRUE(ids.size() == 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient003 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient004
 * @tc.desc: test GetMainDisplayId() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient004 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto id = client.GetMainDisplayId();
    client.proxy_ = proxy;
    EXPECT_EQ(id, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient004 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient005
 * @tc.desc: test SetBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient004 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_BRIGHTNESS = 150;
    auto result = client.SetBrightness(SET_BRIGHTNESS);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient005 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient006
 * @tc.desc: test SetMaxBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient006 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_MAX_BRIGHTNESS = 231;
    auto result = client.SetMaxBrightness(SET_MAX_BRIGHTNESS,
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DisplayPowerMgrClient007
 * @tc.desc: test SetMaxBrightnessNit() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient006 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_MAX_BRIGHTNESS = 231;
    auto result = client.SetMaxBrightnessNit(SET_MAX_BRIGHTNESS,
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient007 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient008
 * @tc.desc: test SetMaxBrightnessNit() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient008, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient008 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const double NO_DISCOUNT = 1.00;
    auto result = client.DiscountBrightness(NO_DISCOUNT);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient008 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient009
 * @tc.desc: test OverrideBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient009, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient009 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_OVERRIDE_BRIGHTNESS = 202;
    auto result = client.OverrideBrightness(SET_OVERRIDE_BRIGHTNESS);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient009 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient010
 * @tc.desc: test OverrideDisplayOffDelay() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient010, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient010 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t OVERRIDE_DELAY_TIME = 200;
    auto result = client.OverrideDisplayOffDelay(OVERRIDE_DELAY_TIME);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient010 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient011
 * @tc.desc: test RestoreBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient011, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient011 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto result = client.RestoreBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient011 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient012
 * @tc.desc: test GetBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient012 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient012 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient013
 * @tc.desc: test GetDefaultBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient013 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient013 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient014
 * @tc.desc: test GetDefaultBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient014, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient013 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetDefaultBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient014 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient015
 * @tc.desc: test GetMaxBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient015, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient015 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetMaxBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient015 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient016
 * @tc.desc: test GetMinBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient016, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient016 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetMinBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient016 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient017
 * @tc.desc: test AdjustBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient017, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient017 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t ADJUST_BRIGHTNESS = 200;
    auto ret = client.AdjustBrightness(OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID,
        ADJUST_BRIGHTNESS);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient017 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient018
 * @tc.desc: test AutoAdjustBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient018, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient018 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.AutoAdjustBrightness(true);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient018 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient019
 * @tc.desc: test IsAutoAdjustBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient019, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient019 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.IsAutoAdjustBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient019 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient020
 * @tc.desc: test BoostBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient020, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient020 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t BOOST_TIMEOUT_MS = 200;
    auto ret = client.BoostBrightness(BOOST_TIMEOUT_MS);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient020 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient021
 * @tc.desc: test CancelBoostBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient021, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient021 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.CancelBoostBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient021 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient022
 * @tc.desc: test GetDeviceBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient022, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient022 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetDeviceBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient022 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient023
 * @tc.desc: test SetCoordinated() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient023, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient023 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.SetCoordinated(true);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient023 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient024
 * @tc.desc: test SetLightBrightnessThreshold() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient024, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient024 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    std::vector<int32_t> threshold = {};
    auto ret = client.SetLightBrightnessThreshold(threshold, nullptr);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient024 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient025
 * @tc.desc: test RegisterCallback() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient025, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient025 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    sptr<IDisplayPowerCallback> callback = new DisplayPowerMgrServiceTest::DisplayPowerMgrTestCallback();
    auto ret = client.RegisterCallback(callback);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient025 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient026
 * @tc.desc: test SetScreenOnBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient026, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient026 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.SetScreenOnBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient026 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient027
 * @tc.desc: test NotifyBrightnessManagerScreenPowerStatus() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrClient027, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient027 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.NotifyBrightnessManagerScreenPowerStatus(
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID,
        static_cast<uint32_t>(DisplayState::DISPLAY_ON));
    client.proxy_ = proxy;
    static constexpr int32_t DEFAULT_VALUE = -1;
    EXPECT_EQ(ret, DEFAULT_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient027 function end!");
}
} // namespace
