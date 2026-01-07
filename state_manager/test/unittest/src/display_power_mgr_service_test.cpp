/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "screen_manager_lite.h"
#include "screen_action.h"
#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
#include "miscellaneous_display_power_strategy.h"
#endif


using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
namespace {
bool g_powerState = false;
}

void DisplayPowerMgrServiceTest::SetUp()
{
}

void DisplayPowerMgrServiceTest::TearDown()
{
}

namespace OHOS::Rosen {
bool ScreenManagerLite::SetScreenPowerForAll(ScreenPowerState state, PowerStateChangeReason reason)
{
    return g_powerState;
}

bool ScreenManagerLite::SetSpecifiedScreenPower(ScreenId screenId,
    ScreenPowerState state, PowerStateChangeReason reason)
{
    return g_powerState;
}
} // namespace OHOS::Rosen

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
 * @tc.desc: Test GetDisplayIds
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService002 function start!");
    std::vector<uint32_t> ret = DisplayPowerMgrClient::GetInstance().GetDisplayIds();
    EXPECT_TRUE(ret.size() != 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService002 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService003
 * @tc.desc: Test AutoAdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService003 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    if (ret) {
        DISPLAY_HILOGI(LABEL_TEST, "AutoAdjustBrightness: is supported");
        ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        EXPECT_TRUE(ret);
    } else {
        DISPLAY_HILOGI(LABEL_TEST, "AutoAdjustBrightness: is not supported");
        EXPECT_FALSE(ret);
    }
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService003 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService004
 * @tc.desc: Test GetError
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService004 function start!");
    DisplayErrors error = DisplayPowerMgrClient::GetInstance().GetError();
    DISPLAY_HILOGI(LABEL_TEST, "get error %{public}d", static_cast<int32_t>(error));
    EXPECT_TRUE(error == DisplayErrors::ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService004 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService005
 * @tc.desc: Test RegisterCallback(Input callback is nullptr, function return false)
 * @tc.type: FUNC
 * @tc.require: issueI63PTF
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService005 function start!");
    bool result = DisplayPowerMgrClient::GetInstance().RegisterCallback(nullptr);
    EXPECT_EQ(result, false);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService005 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService006
 * @tc.desc: Test OverrideDisplayOffDelay para
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService006 function start!");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(0);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService006 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService007
 * @tc.desc: Test prohibitting writing delay screen off time in screen off state
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService007 function start!");
    DisplayState State = DisplayPowerMgrClient::GetInstance().GetDisplayState();
    if (State != DisplayState::DISPLAY_OFF) {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    }
    uint32_t delayTime = 10; //delay time is 10ms
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideDisplayOffDelay(delayTime);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService007 function end!");
}

/**
 * @tc.name: DisplayPowerMgrService008
 * @tc.desc: Test set screen power fail
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService008, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService008 function start!");
    uint32_t displayId = 0;
    uint32_t reason = 0;
    std::shared_ptr<ScreenAction> action = std::make_shared<ScreenAction>(displayId);
    bool ret = action->SetDisplayPower(DisplayPowerMgr::DisplayState::DISPLAY_ON, reason);
    EXPECT_EQ(ret, false);
    g_powerState = true;
    ret = action->SetDisplayPower(DisplayPowerMgr::DisplayState::DISPLAY_ON, reason);
    EXPECT_EQ(ret, true);
    g_powerState = false;
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService008 function end!");
}

#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
/**
 * @tc.name: DisplayPowerMgrService009
 * @tc.desc: Test set screen power with specific strategy
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrServiceTest, DisplayPowerMgrService009, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService009 function start!");
    uint32_t displayId = 0;
    uint32_t reason = 0;
    std::shared_ptr<ScreenAction> action = std::make_shared<ScreenAction>(displayId);
    MiscellaneousDisplayPowerStrategy::GetInstance().SetStrategy(PowerOffStrategy::STRATEGY_SPECIFIC,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_WIRED_APPCAST);
    bool ret = action->SetDisplayPower(DisplayPowerMgr::DisplayState::DISPLAY_ON, reason);
    EXPECT_EQ(ret, false);
    ret = action->SetDisplayPower(DisplayPowerMgr::DisplayState::DISPLAY_OFF, reason);
    EXPECT_EQ(ret, false);

    MiscellaneousDisplayPowerStrategy::GetInstance().SetStrategy(PowerOffStrategy::STRATEGY_ALL,
        PowerMgr::StateChangeReason::STATE_CHANGE_REASON_UNKNOWN);
    ret = action->SetDisplayPower(DisplayPowerMgr::DisplayState::DISPLAY_ON, reason);
    EXPECT_EQ(ret, false);
    ret = action->SetDisplayPower(DisplayPowerMgr::DisplayState::DISPLAY_OFF, reason);
    EXPECT_EQ(ret, false);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrService009 function end!");
}
#endif
} // namespace
