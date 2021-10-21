/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "display_power_mgr_client.h"
#include "display_power_mgr_service.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

namespace {

/**
 * @tc.name: DisplayPowerMgrService01
 * @tc.desc: Test DisplayPowerMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService01, TestSize.Level0)
{
	#ifdef IPC_AVAILABLE
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sam != nullptr) << "DisplayPowerMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    EXPECT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
	#endif // IPC_AVAILABLE
	EXPECT_TRUE(true);
}

/**
 * @tc.name: DisplayPowerMgrService002
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService002, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService003
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService003, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService004
 * @tc.desc: Test set brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService004, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetBrightness(5);
    sleep(5);
    EXPECT_TRUE(ret);
    ret = DisplayPowerMgrClient::GetInstance().SetBrightness(255);
    sleep(5);
    EXPECT_TRUE(ret);
}


/**
 * @tc.name: DisplayPowerMgrService005
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService005, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    sleep(5);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_ON);
}

/**
 * @tc.name: DisplayPowerMgrService006
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService006, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    sleep(5);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_DIM);
}

/**
 * @tc.name: DisplayPowerMgrService007
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService007, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    sleep(5);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_OFF);
}

/**
 * @tc.name: DisplayPowerMgrService008
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService008, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    sleep(5);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_SUSPEND);
}

}