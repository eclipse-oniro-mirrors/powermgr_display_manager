/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "display_power_mgr_service_test.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <vector>

#include "display_power_mgr_client.h"
#include "display_log.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

void DisplayPowerMgrServiceTest::SetUp()
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
}

void DisplayPowerMgrServiceTest::TearDown()
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
}

namespace {
/**
 * @tc.name: DisplayPowerMgrService001
 * @tc.desc: Test DisplayPowerMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService01 is start");
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sam != nullptr) << "DisplayPowerMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    EXPECT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
}

/**
 * @tc.name: DisplayPowerMgrService002
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService002 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService003
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService003 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService004
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService004 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService005
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService005 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService006
 * @tc.desc: Test set screen state unknown
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService006 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_UNKNOWN);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService007
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService007 is start");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_ON);
}

/**
 * @tc.name: DisplayPowerMgrService008
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService008, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService008 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_DIM);
}

/**
 * @tc.name: DisplayPowerMgrService009
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService009, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService009 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_OFF);
}

/**
 * @tc.name: DisplayPowerMgrService010
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService010, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService010 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_SUSPEND);
}

/**
 * @tc.name: DisplayPowerMgrService011
 * @tc.desc: Test set screen state unknown
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService011, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService011 is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_UNKNOWN);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_UNKNOWN);
}

/**
 * @tc.name: DisplayPowerMgrService012
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService012 is start");
    std::vector<uint32_t> ret = DisplayPowerMgrClient::GetInstance().GetDisplayIds();
    EXPECT_TRUE(ret.size() != 0);
}

/**
 * @tc.name: DisplayPowerMgrService013
 * @tc.desc: Test GetMainDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService013 is start");
    int32_t ret = DisplayPowerMgrClient::GetInstance().GetMainDisplayId();
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: DisplayPowerMgrService014
 * @tc.desc: Test AutoAdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService014, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService014 is start");
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    if (ret) {
        DISPLAY_HILOGI(LABEL_TEST, "AutoAdjustBrightness: is supported");
        ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        EXPECT_TRUE(ret);
    } else {
        DISPLAY_HILOGI(LABEL_TEST, "AutoAdjustBrightness: is not supported");
        EXPECT_FALSE(ret);
    }
}

/**
 * @tc.name: DisplayPowerMgrService015
 * @tc.desc: Test GetError
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService015, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService015 is start");
    DisplayErrors error = DisplayPowerMgrClient::GetInstance().GetError();
    DISPLAY_HILOGI(LABEL_TEST, "get error %{public}d", static_cast<int32_t>(error));
    EXPECT_TRUE(error == DisplayErrors::ERR_OK);
}

/**
 * @tc.name: DisplayPowerMgrService016
 * @tc.desc: Test RegisterCallback(Input callback is nullptr, function return false)
 * @tc.type: FUNC
 * @tc.require: issueI63PTF
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService016, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService016 is start");
    bool result = DisplayPowerMgrClient::GetInstance().RegisterCallback(nullptr);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: DisplayPowerMgrService017
 * @tc.desc: Test OverrideDisplayOffDelay para
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService017, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService017 is start");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(0);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService018
 * @tc.desc: Test prohibitting writing delay screen off time in screen off state
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService018, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService018 is start");
    DisplayState State = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    if (State != DisplayState::DISPLAY_OFF) {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    }
    uint32_t delayTime = 10; //delay time is 10ms
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(delayTime);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService019
 * @tc.desc: Test srceen delay off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService019, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService019 is start");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    uint32_t delayTime = 10; //delay time is 10ms
    DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(delayTime);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    DisplayState delayState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(delayState == DisplayState::DISPLAY_DELAY_OFF);
    //Callback for waiting window
    int sleepTime = 2; // sleep time is 2s
    sleep(sleepTime);
    DisplayState offState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(offState == DisplayState::DISPLAY_OFF);
}

/**
 * @tc.name: DisplayPowerMgrService020
 * @tc.desc: Test srceen delay off interrupt
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService020, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService020 is start");
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
}

/**
 * @tc.name: DisplayPowerMgrService021
 * @tc.desc: Test set coordinated and set display state
 * @tc.type: FUNC
 * @tc.require: issueI8JBT4
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService021, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService021 is start");
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
        DisplayState::DISPLAY_ON, PowerMgr::StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    EXPECT_TRUE(ret);
    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(
        DisplayState::DISPLAY_OFF, PowerMgr::StateChangeReason::STATE_CHANGE_REASON_TIMEOUT);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService022
 * @tc.desc: Test set display state in Pre-light the screen
 * @tc.type: FUNCs
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService022, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService022 is start");
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
}

/**
 * @tc.name: DisplayPowerMgrService023
 * @tc.desc: Test set display state in Pre-light the screen and auth success
 * @tc.type: FUNC
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService023, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService023 is start");
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
}

/**
 * @tc.name: DisplayPowerMgrService024
 * @tc.desc: Test set display state in Pre-light the screen and auth failed but screen on
 * @tc.type: FUNC
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService024, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService024 is start");
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
}

/**
 * @tc.name: DisplayPowerMgrService025
 * @tc.desc: Test set display state in Pre-light the screen and auth failed but screen on
 * @tc.type: FUNC
 * @tc.require: issue#I9AJ1S
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService025, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService025 is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    DisplayState currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);

    ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_PRE_BRIGHT_AUTH_FAIL_SCREEN_OFF);
    EXPECT_FALSE(ret);
    currentState = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(currentState == DisplayState::DISPLAY_OFF);
}

} // namespace
