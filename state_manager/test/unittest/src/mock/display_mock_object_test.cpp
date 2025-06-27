/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
static constexpr uint32_t DEFAULT_DURATION = 500;
static constexpr int32_t ERR_OK = 0;
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
    DISPLAY_HILOGI(LABEL_TEST, "DisplayMockObjectTest_001 function start!");
    sptr<MockDisplayRemoteObject> sptrRemoteObj = new MockDisplayRemoteObject();
    auto sptrDisplayProxy = std::make_shared<DisplayPowerMgr::DisplayPowerMgrProxy>(sptrRemoteObj);
    bool bRet = false;
    EXPECT_NE(sptrDisplayProxy->SetDisplayState(DISPLAY_ID,
        static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON), REASON, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    int32_t state = 0;
    EXPECT_NE(sptrDisplayProxy->GetDisplayState(DISPLAY_ID, state), ERR_OK);
    EXPECT_NE(DisplayPowerMgr::DisplayState::DISPLAY_ON, DisplayPowerMgr::DisplayState(state));
    std::vector<uint32_t> result;
    result.push_back(DISPLAY_ID);
    std::vector<uint32_t> ids;
    EXPECT_NE(sptrDisplayProxy->GetDisplayIds(ids), ERR_OK);
    EXPECT_NE(result, ids);
    uint32_t id = 0;
    EXPECT_NE(sptrDisplayProxy->GetMainDisplayId(id), ERR_OK);
    EXPECT_EQ(MAIN_ID_PROXY, id);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayMockObjectTest_001 function end!");
}

/**
 * @tc.name: DisplayMockObjectTest_002
 * @tc.desc: test DisplayPowerMgrProxy function(IRemoteObject is mock, "SendRequest" function return false)
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayMockObjectTest, DisplayMockObjectTest_002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayMockObjectTest_002 function start!");
    sptr<MockDisplayRemoteObject> sptrRemoteObj = new MockDisplayRemoteObject();
    auto sptrDisplayProxy = std::make_shared<DisplayPowerMgr::DisplayPowerMgrProxy>(sptrRemoteObj);
    bool bRet = false;
    uint32_t brightness = 0;
    int32_t errCode = 0;
    EXPECT_NE(sptrDisplayProxy->SetBrightness(BRIGHTNESS_SETTING_VALUE, DISPLAY_ID, false, bRet, errCode), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->DiscountBrightness(DISCOUNT_VALUE, DISPLAY_ID, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->OverrideBrightness(BRIGHTNESS_OVERRIDE_VALUE, DISPLAY_ID, DEFAULT_DURATION, bRet),
        ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->RestoreBrightness(DISPLAY_ID, DEFAULT_DURATION, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->GetBrightness(DISPLAY_ID, brightness), ERR_OK);
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, brightness);
    EXPECT_NE(sptrDisplayProxy->GetDeviceBrightness(DISPLAY_ID, brightness), ERR_OK);
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, brightness);
    EXPECT_NE(sptrDisplayProxy->GetDefaultBrightness(brightness), ERR_OK);
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, brightness);
    EXPECT_NE(sptrDisplayProxy->GetMaxBrightness(brightness), ERR_OK);
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, brightness);
    EXPECT_NE(sptrDisplayProxy->GetMinBrightness(brightness), ERR_OK);
    EXPECT_EQ(BRIGHTNESS_NONE_VALUE, brightness);
    EXPECT_NE(sptrDisplayProxy->AdjustBrightness(DISPLAY_ID, BRIGHTNESS_ADJUST_VALUE, BRIGHTNESS_DURATION, bRet),
        ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->AutoAdjustBrightness(true, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->AutoAdjustBrightness(false, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->IsAutoAdjustBrightness(bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->BoostBrightness(TIMEOUT_MS, DISPLAY_ID, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    EXPECT_NE(sptrDisplayProxy->CancelBoostBrightness(DISPLAY_ID, bRet), ERR_OK);
    EXPECT_FALSE(bRet);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayMockObjectTest_002 function end!");
}
} //namespace
