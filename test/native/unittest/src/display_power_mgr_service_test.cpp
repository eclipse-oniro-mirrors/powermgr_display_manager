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
#include <vector>

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
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService004, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService005
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService005, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService006
 * @tc.desc: Test set screen state unknown
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService006, TestSize.Level0)
{
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_UNKNOWN);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService007
 * @tc.desc: Test set brightness
 * @tc.type: FUNC
 */
#ifdef SHIELDING
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService007, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "SetBrightness: fun is start");
    DISPLAY_HILOGI(MODULE_SERVICE, "SetBrightness: set 5");
    auto ret = DisplayPowerMgrClient::GetInstance().SetBrightness(5);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "SetBrightness: set 155");
    ret = DisplayPowerMgrClient::GetInstance().SetBrightness(155);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "SetBrightness: set 255");
    ret = DisplayPowerMgrClient::GetInstance().SetBrightness(255);
    sleep(5);
    EXPECT_TRUE(ret);
}
#endif


/**
 * @tc.name: DisplayPowerMgrService008
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService008, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService008: fun is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    sleep(5);
    EXPECT_TRUE(ret);
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_ON);
}

/**
 * @tc.name: DisplayPowerMgrService009
 * @tc.desc: Test set screen state dim
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService009, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService009: fun is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService009: GetDisplayState");
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_DIM);
}

/**
 * @tc.name: DisplayPowerMgrService0010
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService010, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService010: fun is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService010: GetDisplayState");
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_OFF);
}

/**
 * @tc.name: DisplayPowerMgrService011
 * @tc.desc: Test set screen state suspend
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService011, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService011: fun is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_SUSPEND);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService011: GetDisplayState");
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_SUSPEND);
}

/**
 * @tc.name: DisplayPowerMgrService012
 * @tc.desc: Test set screen state unknow
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService012, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService012: fun is start");
    auto ret = DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_UNKNOWN);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService012: GetDisplayState");
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    EXPECT_TRUE(state == DisplayState::DISPLAY_UNKNOWN);
}

/**
 * @tc.name: DisplayPowerMgrService013
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService013, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService013: fun is start");
    std::vector<uint32_t> ret = DisplayPowerMgrClient::GetInstance().GetDisplayIds();
    sleep(5);
    EXPECT_TRUE(ret.size() != 0);
}

/**
 * @tc.name: DisplayPowerMgrService014
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService014, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService014: fun is start");
    int32_t ret = DisplayPowerMgrClient::GetInstance().GetMainDisplayId();
    sleep(5);
    EXPECT_TRUE(ret == 0);
}

/**
 * @tc.name: DisplayPowerMgrService015
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService015, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService015: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    sleep(5);
    if (ret) {
        DISPLAY_HILOGI(MODULE_SERVICE, "AutoAdjustBrightness: is supported");
        ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        EXPECT_TRUE(ret);
    } else {
        DISPLAY_HILOGI(MODULE_SERVICE, "AutoAdjustBrightness: is not supported");
        EXPECT_FALSE(ret);
    }
}

/**
 * @tc.name: DisplayPowerMgrService016
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService016, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService016: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().AdjustBrightness(0, 3000);
    sleep(5);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrService016: fun is end");
}
}
