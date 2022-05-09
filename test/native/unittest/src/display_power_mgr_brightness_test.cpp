/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "display_power_mgr_client.h"
#include "hilog_wrapper.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

class DisplayPowerMgrBrightnessTest : public Test {
public:
    void SetUp()
    {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    }

    void TearDown()
    {
        DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        sleep(1); // wait for gradual animation
    }
};

namespace {
/**
 * @tc.name: DisplayPowerMgrBrightness001
 * @tc.desc: Test SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness001: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(value, 100);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBrightness002
 * @tc.desc: Test OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBrightness002, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness002: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(value, 255);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness002: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBrightness003
 * @tc.desc: Test RestoreBrightness after OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBrightness003, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness003: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(value, 255);
    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness(); // restore
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    value = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(value, 100);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness003: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBrightness004
 * @tc.desc: Test SetBrightness after OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBrightness004, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness004: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_FALSE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(value, 255);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness004: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBrightness005
 * @tc.desc: Test RestoreBrightness after not override
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBrightness005, TestSize.Level0)
{
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness005: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(value, 100);
    DISPLAY_HILOGI(MODULE_SERVICE, "DisplayPowerMgrBrightness005: fun is end");
}
}