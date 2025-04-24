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

#include <gtest/gtest.h>
#include "display_log.h"
#include "mock_brightness_manager_interface.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
using namespace std;

namespace {
    const double NO_DISCOUNT = 1.00;
    const uint32_t MAX_DEFAULT_BRIGHTNESS_LEVEL = 255;
    const uint32_t MIN_DEFAULT_BRIGHTNESS_LEVEL = 1;
    const uint32_t TEST_MODE = 1;
    const uint32_t NORMAL_MODE = 2;
    const uint32_t DEFAULT_WAITING_TIME = 1200000;
    const uint32_t INIT_WAITING_TIME = 3000000;
}

BrightnessManagerExt g_BrightnessManagerExt;

class BrightnessManagerExtTest : public Test {};

namespace {
HWTEST_F(BrightnessManagerExtTest, NoInit, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerNoInit001: fun is start");
    bool retBool = g_BrightnessManagerExt.AutoAdjustBrightness(true);
    EXPECT_FALSE(retBool);

    retBool = g_BrightnessManagerExt.SetBrightness(90);
    EXPECT_FALSE(retBool);

    retBool = g_BrightnessManagerExt.DiscountBrightness(0.8);
    EXPECT_FALSE(retBool);

    double discount = g_BrightnessManagerExt.GetDiscount();
    EXPECT_EQ(1.0, discount);

    g_BrightnessManagerExt.SetScreenOnBrightness();

    uint32_t value = g_BrightnessManagerExt.GetScreenOnBrightness();
    EXPECT_EQ(0, value);

    g_BrightnessManagerExt.ClearOffset();

    retBool = g_BrightnessManagerExt.OverrideBrightness(255);
    usleep(DEFAULT_WAITING_TIME);
    EXPECT_FALSE(retBool);

    retBool = g_BrightnessManagerExt.RestoreBrightness();
    usleep(DEFAULT_WAITING_TIME);
    EXPECT_FALSE(retBool);

    retBool = g_BrightnessManagerExt.BoostBrightness(10000);
    EXPECT_FALSE(retBool);

    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.CancelBoostBrightness();
    EXPECT_FALSE(retBool);

    uint32_t brightness = g_BrightnessManagerExt.GetBrightness();
    EXPECT_EQ(0, brightness);

    usleep(DEFAULT_WAITING_TIME);
    uint32_t deviceBrightness = g_BrightnessManagerExt.GetDeviceBrightness();
    EXPECT_EQ(0, value);

    retBool = g_BrightnessManagerExt.IsBrightnessOverridden();
    EXPECT_FALSE(retBool);

    retBool = g_BrightnessManagerExt.IsBrightnessBoosted();
    EXPECT_FALSE(retBool);

    uint32_t currentDisplayId = g_BrightnessManagerExt.GetCurrentDisplayId(0);
    EXPECT_EQ(0, currentDisplayId);

    retBool = g_BrightnessManagerExt.SetBrightness(100);
    EXPECT_FALSE(retBool);

    retBool = g_BrightnessManagerExt.SetMaxBrightnessNit(100);
    EXPECT_FALSE(retBool);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerNoInit001: fun is end");
}

HWTEST_F(BrightnessManagerExtTest, MockInit001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerMockInit001: fun is start");
    // 打桩
    MockInitBrightnessManagerExt(g_BrightnessManagerExt);
    MockClearValues();

    g_BrightnessManagerExt.SetDisplayState(0, DisplayState::DISPLAY_OFF, 0);

    MockSetValue(1.0);
    usleep(DEFAULT_WAITING_TIME);
    bool retBool = g_BrightnessManagerExt.AutoAdjustBrightness(true);
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(2.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.SetBrightness(0, 0, true);
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(3.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.DiscountBrightness(91.0);
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(4.0);
    usleep(DEFAULT_WAITING_TIME);
    auto ret = g_BrightnessManagerExt.GetDiscount();
    MockClearValues();
    EXPECT_EQ(ret, 4.0);

    g_BrightnessManagerExt.SetScreenOnBrightness();

    MockSetValue(5.0);
    usleep(DEFAULT_WAITING_TIME);
    int onBrightness= g_BrightnessManagerExt.GetScreenOnBrightness();
    MockClearValues();
    EXPECT_EQ(onBrightness, 5);

    g_BrightnessManagerExt.ClearOffset();

    MockSetValue(6.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.OverrideBrightness(0, 0);
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(7.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.RestoreBrightness(0);
    MockClearValues();
    EXPECT_TRUE(retBool);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerMockInit001: fun is end");
}

HWTEST_F(BrightnessManagerExtTest, MockInit002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerMockInit002: fun is start");
    MockSetValue(8.0);
    usleep(DEFAULT_WAITING_TIME);
    bool retBool = g_BrightnessManagerExt.BoostBrightness(0, 0);
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(9.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.CancelBoostBrightness(0);
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(10.0);
    usleep(DEFAULT_WAITING_TIME);
    int brightness= g_BrightnessManagerExt.GetBrightness();
    MockClearValues();
    EXPECT_EQ(brightness, 10);

    MockSetValue(11.0);
    usleep(DEFAULT_WAITING_TIME);
    int deviceBrightness= g_BrightnessManagerExt.GetDeviceBrightness();
    MockClearValues();
    EXPECT_EQ(deviceBrightness, 11);

    MockSetValue(12.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.IsBrightnessOverridden();
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(13.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.IsBrightnessBoosted();
    MockClearValues();
    EXPECT_TRUE(retBool);

    MockSetValue(14.0);
    usleep(DEFAULT_WAITING_TIME);
    int currentDisplayId= g_BrightnessManagerExt.GetCurrentDisplayId(1);
    MockClearValues();
    EXPECT_EQ(currentDisplayId, 14);

    MockSetValue(15.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.SetMaxBrightness(1.0);
    MockClearValues();
    EXPECT_TRUE(retBool);

    g_BrightnessManagerExt.SetDisplayId(1);

    MockSetValue(16.0);
    usleep(DEFAULT_WAITING_TIME);
    retBool = g_BrightnessManagerExt.SetMaxBrightnessNit(1.0);
    MockClearValues();
    EXPECT_TRUE(retBool);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerMockInit002: fun is end");
}

HWTEST_F(BrightnessManagerExtTest, Init, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerExtInit001: fun is start");
    MockDeInitBrightnessManagerExt(g_BrightnessManagerExt);
    MockClearValues();
    g_BrightnessManagerExt.Init(MIN_DEFAULT_BRIGHTNESS_LEVEL, MAX_DEFAULT_BRIGHTNESS_LEVEL);
    usleep(INIT_WAITING_TIME);
    double discount = g_BrightnessManagerExt.GetDiscount();
    EXPECT_EQ(1.0, discount);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerExtInit001: fun is end");
}

} // namespace