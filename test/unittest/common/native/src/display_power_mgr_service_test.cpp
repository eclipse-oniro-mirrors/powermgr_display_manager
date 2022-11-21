/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

void DisplayPowerMgrServiceTest::TearDown()
{
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    if (state != DisplayState::DISPLAY_ON) {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
        return;
    }
    return;
}

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
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayPowerMgrService007
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
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
    DISPLAY_HILOGI(LABEL_TEST, "fun is start");
    DisplayErrors error = DisplayPowerMgrClient::GetInstance().GetError();
    DISPLAY_HILOGI(LABEL_TEST, "get error %{public}d", static_cast<int32_t>(error));
    EXPECT_TRUE(error == DisplayErrors::ERR_OK);
}
}
