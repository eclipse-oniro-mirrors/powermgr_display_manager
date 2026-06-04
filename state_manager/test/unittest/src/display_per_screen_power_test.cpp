/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "display_per_screen_power_test.h"

#include <vector>

#include "display_power_mgr_client.h"
#include "display_log.h"
#include "screen_controller.h"
#include "screen_power_adapter.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

#ifdef ENABLE_PER_SCREEN_POWER

namespace {
class TestScreenDisplayStateCallback : public IScreenDisplayStateCallback {
public:
    void OnDisplayStateChanged(uint64_t displayId, uint32_t state, uint32_t reason) override
    {
        lastDisplayId_ = displayId;
        lastState_ = state;
        lastReason_ = reason;
        callCount_++;
    }

    uint64_t lastDisplayId_ {0};
    uint32_t lastState_ {0};
    uint32_t lastReason_ {0};
    uint32_t callCount_ {0};
};

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower001: GetDisplayStateById basic query");
    uint64_t displayId = 0;
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayStateById(displayId);
    EXPECT_TRUE(state == DisplayState::DISPLAY_ON || state == DisplayState::DISPLAY_OFF ||
        state == DisplayState::DISPLAY_UNKNOWN);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower002: GetDisplayStateById with invalid displayId");
    uint64_t invalidId = 99999;
    DisplayState state = DisplayPowerMgrClient::GetInstance().GetDisplayStateById(invalidId);
    EXPECT_EQ(state, DisplayState::DISPLAY_UNKNOWN);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower003: SetDisplayStateById invalid state");
    uint64_t displayId = 0;
    uint32_t invalidState = 99;
    uint32_t reason = 0;
    DisplayErrors ret = DisplayPowerMgrClient::GetInstance().SetDisplayStateById(displayId, invalidState, reason);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower004: SetDisplayStateById invalid displayId");
    uint64_t invalidId = 99999;
    uint32_t state = static_cast<uint32_t>(DisplayState::DISPLAY_ON);
    uint32_t reason = 0;
    DisplayErrors ret = DisplayPowerMgrClient::GetInstance().SetDisplayStateById(invalidId, state, reason);
    EXPECT_EQ(ret, DisplayErrors::ERR_PARAM_INVALID);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower005: RegisterScreenDisplayStateCallback null");
    bool ret = DisplayPowerMgrClient::GetInstance().RegisterScreenDisplayStateCallback(nullptr);
    EXPECT_FALSE(ret);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower006: Register and UnregisterScreenDisplayStateCallback");
    auto callback = sptr<TestScreenDisplayStateCallback>::MakeSptr();
    ASSERT_TRUE(callback != nullptr);
    bool ret = DisplayPowerMgrClient::GetInstance().RegisterScreenDisplayStateCallback(callback);
    EXPECT_TRUE(ret);
    ret = DisplayPowerMgrClient::GetInstance().UnregisterScreenDisplayStateCallback();
    EXPECT_TRUE(ret);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower007: ScreenController SetState");
    uint32_t id = 0;
    auto controller = std::make_shared<ScreenController>(id);
    ASSERT_TRUE(controller != nullptr);
    controller->SetState(DisplayState::DISPLAY_OFF);
    EXPECT_EQ(controller->GetState(), DisplayState::DISPLAY_OFF);
    controller->SetState(DisplayState::DISPLAY_ON);
    EXPECT_EQ(controller->GetState(), DisplayState::DISPLAY_ON);
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower008, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower008: SetDisplayStateById same state");
    uint64_t displayId = 0;
    DisplayState currentState = DisplayPowerMgrClient::GetInstance().GetDisplayStateById(displayId);
    if (currentState != DisplayState::DISPLAY_UNKNOWN) {
        uint32_t sameState = static_cast<uint32_t>(currentState);
        uint32_t reason = 0;
        DisplayErrors ret = DisplayPowerMgrClient::GetInstance().SetDisplayStateById(displayId, sameState, reason);
        EXPECT_EQ(ret, DisplayErrors::ERR_OK);
    }
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower009, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower009: Callback receives state change");
    auto callback = sptr<TestScreenDisplayStateCallback>::MakeSptr();
    ASSERT_TRUE(callback != nullptr);
    bool regRet = DisplayPowerMgrClient::GetInstance().RegisterScreenDisplayStateCallback(callback);
    EXPECT_TRUE(regRet);

    uint64_t displayId = 0;
    DisplayState originalState = DisplayPowerMgrClient::GetInstance().GetDisplayStateById(displayId);
    if (originalState != DisplayState::DISPLAY_UNKNOWN) {
        uint32_t targetState = (originalState == DisplayState::DISPLAY_ON)
            ? static_cast<uint32_t>(DisplayState::DISPLAY_OFF)
            : static_cast<uint32_t>(DisplayState::DISPLAY_ON);
        uint32_t reason = 1;
        DisplayErrors setRet = DisplayPowerMgrClient::GetInstance().SetDisplayStateById(displayId, targetState, reason);
        EXPECT_EQ(setRet, DisplayErrors::ERR_OK);

        DisplayPowerMgrClient::GetInstance().SetDisplayStateById(displayId,
            static_cast<uint32_t>(originalState), reason);
    }

    DisplayPowerMgrClient::GetInstance().UnregisterScreenDisplayStateCallback();
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower010, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower010: ScreenPowerAdapter WakeUpBegin call tracking");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
    uint64_t displayId = 0;
    Rosen::PowerStateChangeReason reason = Rosen::PowerStateChangeReason::REASON_APPLICATION;
    bool ret = ScreenPowerAdapter::GetInstance().WakeUpBegin(displayId, reason);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ScreenPowerAdapter::GetInstance().HasCall("WakeUpBegin", displayId));
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower011, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower011: ScreenPowerAdapter SuspendBegin call tracking");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
    uint64_t displayId = 0;
    Rosen::PowerStateChangeReason reason = Rosen::PowerStateChangeReason::REASON_APPLICATION;
    bool ret = ScreenPowerAdapter::GetInstance().SuspendBegin(displayId, reason);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ScreenPowerAdapter::GetInstance().HasCall("SuspendBegin", displayId));
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower012, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower012: ScreenPowerAdapter SetScreenOnBrightness call tracking");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
    uint64_t displayId = 0;
    bool ret = ScreenPowerAdapter::GetInstance().SetScreenOnBrightness(displayId);
    EXPECT_TRUE(ret);
    EXPECT_TRUE(ScreenPowerAdapter::GetInstance().HasCall("SetScreenOnBrightness", displayId));
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower013, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower013: ScreenPowerAdapter call order for ON flow");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
    auto& adapter = ScreenPowerAdapter::GetInstance();
    uint64_t displayId = 0;
    Rosen::PowerStateChangeReason reason = Rosen::PowerStateChangeReason::REASON_APPLICATION;

    adapter.WakeUpBegin(displayId, reason);
    adapter.SetScreenPowerById(displayId, Rosen::ScreenPowerState::POWER_ON, reason);
    adapter.SetScreenOnBrightness(displayId);
    adapter.WakeUpEnd(displayId);

    auto records = adapter.GetCallRecords();
    EXPECT_EQ(records.size(), static_cast<size_t>(4));
    EXPECT_EQ(records[0].methodName, "WakeUpBegin");
    EXPECT_EQ(records[1].methodName, "SetScreenPowerById");
    EXPECT_EQ(records[2].methodName, "SetScreenOnBrightness");
    EXPECT_EQ(records[3].methodName, "WakeUpEnd");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
}

HWTEST_F(DisplayPerScreenPowerTest, PerScreenPower014, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "PerScreenPower014: ScreenPowerAdapter call order for OFF flow");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
    auto& adapter = ScreenPowerAdapter::GetInstance();
    uint64_t displayId = 0;
    Rosen::PowerStateChangeReason reason = Rosen::PowerStateChangeReason::REASON_APPLICATION;

    adapter.SuspendBegin(displayId, reason);
    adapter.SetScreenPowerById(displayId, Rosen::ScreenPowerState::POWER_OFF, reason);
    adapter.SuspendEnd(displayId);

    auto records = adapter.GetCallRecords();
    EXPECT_EQ(records.size(), static_cast<size_t>(3));
    EXPECT_EQ(records[0].methodName, "SuspendBegin");
    EXPECT_EQ(records[1].methodName, "SetScreenPowerById");
    EXPECT_EQ(records[2].methodName, "SuspendEnd");
    ScreenPowerAdapter::GetInstance().ClearCallRecords();
}

#endif // ENABLE_PER_SCREEN_POWER
