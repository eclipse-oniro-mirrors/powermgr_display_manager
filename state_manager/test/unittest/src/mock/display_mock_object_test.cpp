/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "display_mock_object_test.h"
#include "display_log.h"
#include "display_power_mgr_client.h"
#include "display_power_mgr_proxy.h"
#include "mock_display_remote_object.h"
#include "power_state_machine_info.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS::DisplayPowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static constexpr uint32_t BRIGHTNESS_SETTING_VALUE = 50;
static constexpr uint32_t BRIGHTNESS_OVERRIDE_VALUE = 100;
static constexpr uint32_t BRIGHTNESS_DURATION = 0;
static constexpr uint32_t BRIGHTNESS_NONE_VALUE = 0;
static constexpr uint32_t DISPLAY_ID = 0;
static constexpr uint32_t MAIN_ID_PROXY = 0;
static constexpr uint32_t REASON = 0;
static constexpr int32_t BRIGHTNESS_ADJUST_VALUE = 150;
static constexpr int32_t TIMEOUT_MS = 500;
static constexpr double DISCOUNT_VALUE = 0.30;
} // namespace

void DisplayMockObjectTest::DisplayPowerMgrTestCallback::OnDisplayStateChanged(
    uint32_t displayId, DisplayPowerMgr::DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrTestCallback::OnDisplayStateChangedStub");
}

namespace {
/**
 * @tc.name: DisplayMockObjectTest_001
 * @tc.desc: test DisplayPowerMgrProxy function(IRemoteObject is mock, "SendRequest" function return false)
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayMockObjectTest, DisplayMockObjectTest_001, TestSize.Level0)
{
    sptr<MockDisplayRemoteObject> sptrRemoteObj = new MockDisplayRemoteObject();
    auto sptrDisplayProxy = std::make_shared<DisplayPowerMgr::DisplayPowerMgrProxy>(sptrRemoteObj);
    EXPECT_FALSE(sptrDisplayProxy->SetDisplayState(DISPLAY_ID, DisplayPowerMgr::DisplayState::DISPLAY_ON, REASON));
    EXPECT_NE(DisplayPowerMgr::DisplayState::DISPLAY_ON, sptrDisplayProxy->GetDisplayState(DISPLAY_ID));
    std::vector<uint32_t> result;
    result.push_back(DISPLAY_ID);
    EXPECT_NE(result, sptrDisplayProxy->GetDisplayIds());
    EXPECT_EQ(MAIN_ID_PROXY, sptrDisplayProxy->GetMainDisplayId());
    EXPECT_FALSE(sptrDisplayProxy->SetBrightness(BRIGHTNESS_SETTING_VALUE, DISPLAY_ID, false));
    EXPECT_FALSE(sptrDisplayProxy->DiscountBrightness(DISCOUNT_VALUE, DISPLAY_ID));
    EXPECT_FALSE(sptrDisplayProxy->OverrideBrightness(BRIGHTNESS_OVERRIDE_VALUE, DISPLAY_ID));
    EXPECT_FALSE(sptrDisplayProxy->RestoreBrightness(DISPLAY_ID));
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, sptrDisplayProxy->GetBrightness(DISPLAY_ID));
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, sptrDisplayProxy->GetDeviceBrightness(DISPLAY_ID));
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, sptrDisplayProxy->GetDefaultBrightness());
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, sptrDisplayProxy->GetMaxBrightness());
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, sptrDisplayProxy->GetMinBrightness());
    EXPECT_FALSE(sptrDisplayProxy->AutoAdjustBrightness(true));
    EXPECT_FALSE(sptrDisplayProxy->AdjustBrightness(DISPLAY_ID, BRIGHTNESS_ADJUST_VALUE, BRIGHTNESS_DURATION));
    EXPECT_FALSE(sptrDisplayProxy->AutoAdjustBrightness(false));
    EXPECT_FALSE(sptrDisplayProxy->IsAutoAdjustBrightness());
    EXPECT_FALSE(sptrDisplayProxy->BoostBrightness(TIMEOUT_MS, DISPLAY_ID));
    EXPECT_FALSE(sptrDisplayProxy->CancelBoostBrightness(DISPLAY_ID));
}
} //namespace
