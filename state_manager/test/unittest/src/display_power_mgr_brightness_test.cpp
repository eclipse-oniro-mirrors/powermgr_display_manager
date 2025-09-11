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

#include <gtest/gtest.h>
#include "display_brightness_callback_stub.h"
#include "display_log.h"
#include "display_manager_test_base.h"
#include "display_power_mgr_client.h"
#include "setting_provider.h"
#include "system_ability_definition.h"
#include "power_mgr_client.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

namespace {
const std::string SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
const double NO_DISCOUNT = 1.00;
const uint32_t MAX_DEFAULT_BRIGHTNESS_LEVEL = 255;
const uint32_t TEST_MODE = 1;
const uint32_t NORMAL_MODE = 2;
constexpr int32_t CMD_EXECUTION_DELAY = 15 * 1000; // 15ms delay before executing command
constexpr int32_t SET_AUTO_DELAY = 50 * 1000;      // 50ms delay for datashare callback
}

class DisplayPowerMgrBrightnessTest : public OHOS::PowerMgr::TestBase {
public:
    void SetUp()
    {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
        DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        DisplayPowerMgrClient::GetInstance().DiscountBrightness(NO_DISCOUNT);
        uint32_t maxBrightness = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
        double value = (double)maxBrightness / MAX_DEFAULT_BRIGHTNESS_LEVEL;
        DisplayPowerMgrClient::GetInstance().SetMaxBrightness(value, TEST_MODE);

        auto& powerMgrClient = PowerMgr::PowerMgrClient::GetInstance();
        PowerMgr::PowerMode mode = powerMgrClient.GetDeviceMode();
        if (mode == PowerMgr::PowerMode::POWER_SAVE_MODE || mode == PowerMgr::PowerMode::EXTREME_POWER_SAVE_MODE) {
            powerMgrClient.SetDeviceMode(PowerMgr::PowerMode::NORMAL_MODE);
        }

        WaitDimmingDone();
        DISPLAY_HILOGI(LABEL_TEST, "%{public}s: function start!", mName.c_str());
    }

    void TearDown()
    {
        WaitDimmingDone();
        DISPLAY_HILOGI(LABEL_TEST, "%{public}s: function end!", mName.c_str());
        DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
        uint32_t maxBrightness = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
        double value = (double)maxBrightness / MAX_DEFAULT_BRIGHTNESS_LEVEL;
        DisplayPowerMgrClient::GetInstance().SetMaxBrightness(value, NORMAL_MODE);
    }

    void WaitDimmingDone()
    {
        usleep(CMD_EXECUTION_DELAY);
        DisplayPowerMgrClient::GetInstance().WaitDimmingDone();
        usleep(SET_AUTO_DELAY);
    }
};

namespace {
/**
 * @tc.name: DisplayPowerMgrSetBrightness001
 * @tc.desc: Test SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetBrightness001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 90;
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t deviceBrightness = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(deviceBrightness, SET_BRIGHTNESS);
}

/**
 * @tc.name: DisplayPowerMgrSettingBrightness001
 * @tc.desc: Test setting brightness value changed when SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSettingBrightness001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS_VALUE = 223;
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS_VALUE);
    EXPECT_TRUE(ret);
    WaitDimmingDone(); // wait for setting update
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(brightness, SET_BRIGHTNESS_VALUE);
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessNormal
 * @tc.desc: Test DiscountBrightness the normal test
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessNormal, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 91;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_NORMAL = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_NORMAL);
    EXPECT_TRUE(ret);
    WaitDimmingDone(); // wait for setBrightness
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(SET_BRIGHTNESS * DISCOUNT_NORMAL));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessNormal
 * @tc.desc: Test DiscountBrightness when screen is off
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessScreenOff, TestSize.Level0)
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_VALUE = 0.7;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessBoundary001
 * @tc.desc: Test DiscountBrightness the boundary test
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessBoundary001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 92;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_LOWER_BOUNDARY = 0.01;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_LOWER_BOUNDARY);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    uint32_t minBrightness = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_EQ(value, minBrightness);
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessBoundary002
 * @tc.desc: Test DiscountBrightness the boundary test
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessBoundary002, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 93;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_UPPER_BOUNDARY = 1.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_UPPER_BOUNDARY);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(SET_BRIGHTNESS * DISCOUNT_UPPER_BOUNDARY));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessAbnormal001
 * @tc.desc: Test DiscountBrightness the abnormal test
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessAbnormal001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 94;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_BEYOND_LOWER_BOUNDARY = 0.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_BEYOND_LOWER_BOUNDARY);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    uint32_t minBrightness = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_EQ(value, minBrightness);
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessAbnormal002
 * @tc.desc: Test DiscountBrightness the abnormal test
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessAbnormal002, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 95;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_BEYOND_UPPER_BOUNDARY = 2.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_BEYOND_UPPER_BOUNDARY);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    const double DISCOUNT_MAX_VALUE = 1.0;
    EXPECT_EQ(value, static_cast<uint32_t>(SET_BRIGHTNESS * DISCOUNT_MAX_VALUE));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessAbnormal003
 * @tc.desc: Test DiscountBrightness the abnormal test
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessAbnormal003, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 96;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_NEGATIVE_VALUE = -1.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_NEGATIVE_VALUE);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    uint32_t minBrightness = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_EQ(value, minBrightness);
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness001
 * @tc.desc: Test OverrideBrightness after DiscountBrightness, then RestoreBrightness
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 97;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_VALUE = 0.7;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));

    const uint32_t SET_OVERRIDE_BRIGHTNESS = 202;
    DisplayPowerMgrClient::GetInstance().OverrideBrightness(SET_OVERRIDE_BRIGHTNESS);
    WaitDimmingDone();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_OVERRIDE_BRIGHTNESS));

    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    WaitDimmingDone();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness002
 * @tc.desc: Test DiscountBrightness after OverrideBrightness, then RestoreBrightness
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness002, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 150;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t SET_OVERRIDE_BRIGHTNESS = 202;
    DisplayPowerMgrClient::GetInstance().OverrideBrightness(SET_OVERRIDE_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);

    const double DISCOUNT_VALUE = 0.7;
    double ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_TRUE(value <= static_cast<uint32_t>(DISCOUNT_VALUE * SET_OVERRIDE_BRIGHTNESS));

    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness003
 * @tc.desc: Test DisconutBrightness after BoostBrightness and CancelBoostBrightness
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness003, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 99;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t SET_BOOST_BRIGHTNESS_TIME = 10000;
    DisplayPowerMgrClient::GetInstance().BoostBrightness(SET_BOOST_BRIGHTNESS_TIME);
    usleep(CMD_EXECUTION_DELAY);

    const double DISCOUNT_VALUE = 0.7;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t maxBrightness = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * maxBrightness));

    DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    WaitDimmingDone();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness004
 * @tc.desc: Test BoostBrightness after DiscountBrightness, then CancelBoostBrightness
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness004, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_VALUE = 0.7;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);
    usleep(CMD_EXECUTION_DELAY);

    const uint32_t SET_BOOST_BRIGHTNESS_TIME = 10000;
    DisplayPowerMgrClient::GetInstance().BoostBrightness(SET_BOOST_BRIGHTNESS_TIME);
    WaitDimmingDone();
    uint32_t maxBrightness = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * maxBrightness));

    DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    WaitDimmingDone();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness005
 * @tc.desc: Test DisconutBrightness after SetBrightness, then GetBrightness
 * @tc.type: FUNC
 * @tc.require: issueI5HWMN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness005, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 103;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const double DISCOUNT_VALUE = 0.7;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);
    usleep(CMD_EXECUTION_DELAY);
    uint32_t beforeBrightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, beforeBrightness);
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness001
 * @tc.desc: Test OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness001, TestSize.Level0)
{
    const uint32_t OVERRIDE_BRIGHTNESS = 255;
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BRIGHTNESS);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, OVERRIDE_BRIGHTNESS);
    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness002
 * @tc.desc: Test RestoreBrightness after OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness002, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 104;
    const uint32_t OVERRIDE_BRIGHTNESS = 255;
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_TRUE(ret);
    usleep(CMD_EXECUTION_DELAY);
    ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BRIGHTNESS);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, OVERRIDE_BRIGHTNESS);
    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness(); // restore
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, SET_BRIGHTNESS);
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness003
 * @tc.desc: Test SetBrightness after OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness003, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 105;
    const uint32_t OVERRIDE_BRIGHTNESS = 255;
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BRIGHTNESS);
    EXPECT_TRUE(ret);
    usleep(CMD_EXECUTION_DELAY);
    ret = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_FALSE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, OVERRIDE_BRIGHTNESS);
    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness004
 * @tc.desc: Test RestoreBrightness after not override
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness004, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 106;
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    EXPECT_TRUE(ret);
    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_FALSE(ret);
    WaitDimmingDone();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, SET_BRIGHTNESS);
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness005
 * @tc.desc: Test OverrideBrightness off Disable the call
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness005, TestSize.Level0)
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t OVERRIDE_BRIGHTNESS = 100;
    bool isBoost = DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BRIGHTNESS);
    EXPECT_FALSE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isRestore = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_FALSE(isRestore);
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness006
 * @tc.desc: Test device brightness when override brightness is 256
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness006, TestSize.Level0)
{
    uint32_t overrideValue = 256;
    uint32_t brightnessMax = 255;
    DisplayPowerMgrClient::GetInstance().OverrideBrightness(overrideValue);
    WaitDimmingDone();
    uint32_t deviceBrightness = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(brightnessMax, deviceBrightness);
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness007
 * @tc.desc: Test device brightness when override brightness is 0
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness007, TestSize.Level0)
{
    uint32_t overrideValue = 0;
    uint32_t brightnessMin = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_TRUE(DisplayPowerMgrClient::GetInstance().OverrideBrightness(overrideValue));
    WaitDimmingDone();
    uint32_t deviceBrightness = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(brightnessMin, deviceBrightness);
    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness008
 * @tc.desc: Test device brightness when override brightness is -1
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness008, TestSize.Level0)
{
    uint32_t overrideValue = -1;
    uint32_t brightnessMax = 255;
    EXPECT_TRUE(DisplayPowerMgrClient::GetInstance().OverrideBrightness(overrideValue));
    WaitDimmingDone();
    uint32_t deviceBrightness = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(brightnessMax, deviceBrightness);
    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness009
 * @tc.desc: Set brightness after override brightness, then exit override brightness, the
 *           restore brightness should be brightness value of thr current device
 * @tc.type: FUNC
 * @tc.require: issueI6ACLX
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness009, TestSize.Level0)
{
    const uint32_t OVERRIDE_BRIGHTNESS = 156;
    const uint32_t SET_BRIGHTNESS = 66;
    auto currentBrightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    bool ret1 = DisplayPowerMgrClient::GetInstance().SetBrightness(currentBrightness);
    EXPECT_TRUE(ret1);
    usleep(CMD_EXECUTION_DELAY);
    DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    bool ret2 = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_FALSE(ret2);
    usleep(CMD_EXECUTION_DELAY);
    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    WaitDimmingDone();
    uint32_t deviceBrightness = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(currentBrightness, deviceBrightness);
}

/**
 * @tc.name: DisplayPowerMgrMaxBrightness001
 * @tc.desc: Test GetMaxBrightness less equals 255
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrMaxBrightness001, TestSize.Level0)
{
    uint32_t max = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
    EXPECT_LE(max, 255);
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(max + 100);
    EXPECT_TRUE(ret);
    WaitDimmingDone(); // wait for setting update
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, max);
}

/**
 * @tc.name: DisplayPowerMgrMinBrightness001
 * @tc.desc: Test GetMinBrightness greater equals 0
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrMinBrightness001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS_VALUE = 0;
    uint32_t min = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_GE(min, 0);
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS_VALUE);
    EXPECT_TRUE(ret);
    WaitDimmingDone(); // wait for setting update
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, min);
}

/**
 * @tc.name: DisplayPowerMgrDefaultBrightness001
 * @tc.desc: Test GetDefaultnBrightness greater equals 0 and less equals 255
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDefaultBrightness001, TestSize.Level0)
{
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDefaultBrightness();
    EXPECT_GE(value, 0);
    EXPECT_LE(value, 255);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessNormal
 * @tc.desc: Test BoostBrightness the normal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessNormal, TestSize.Level0)
{
    const uint32_t BOOST_TIME = 100; // keep BoostBrightness 100ms
    bool isSucc = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_TIME);
    EXPECT_TRUE(isSucc);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessAbnormal
 * @tc.desc: Test BoostBrightness the abnormal value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessAbnormal, TestSize.Level0)
{
    const uint32_t BOOST_TIME = -1;
    bool isSucc = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_TIME);
    EXPECT_FALSE(isSucc);
}

/**
 * @tc.name: DisplayPowerMgrCancelBoostBrightnessNormal
 * @tc.desc: Test CancelBoostBrightness the normal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrCancelBoostBrightnessNormal, TestSize.Level0)
{
    const uint32_t BOOST_TIME = 50000;
    bool isSucc = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_TIME);
    EXPECT_TRUE(isSucc);
    usleep(CMD_EXECUTION_DELAY);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_TRUE(isCancel);
}

/**
 * @tc.name: BoostAndOverrideMutuallyExclusive1
 * @tc.desc: Test BoostBrightness and OverrideBrightness are mutually exclusive
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, BoostAndOverrideMutuallyExclusive1, TestSize.Level0)
{
    const uint32_t BOOST_TIME = 50000;
    const uint32_t OVERRIDE_BIRGHTNESS = 100;
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_TIME);
    EXPECT_TRUE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isOverride = DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BIRGHTNESS);
    EXPECT_FALSE(isOverride);
    usleep(CMD_EXECUTION_DELAY);
    bool isRestore = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_FALSE(isRestore);
}

/**
 * @tc.name: BoostAndOverrideMutuallyExclusive2
 * @tc.desc: Test BoostBrightness and OverrideBrightness are mutually exclusive
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, BoostAndOverrideMutuallyExclusive2, TestSize.Level0)
{
    const uint32_t BOOST_TIME = 50000;
    const uint32_t OVERRIDE_BIRGHTNESS = 255;
    bool isOverride = DisplayPowerMgrClient::GetInstance().OverrideBrightness(OVERRIDE_BIRGHTNESS);
    EXPECT_TRUE(isOverride);
    usleep(CMD_EXECUTION_DELAY);
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_TIME);
    EXPECT_FALSE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_FALSE(isCancel);
    usleep(CMD_EXECUTION_DELAY);
    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessTimeout1
 * @tc.desc: Test BoostBrightness timeout restore system brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessTimeout1, TestSize.Level0)
{
    uint32_t beforeValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    const uint32_t BOOST_BRIGHTNESS_TIME = 100;
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_BRIGHTNESS_TIME);
    EXPECT_TRUE(isBoost);
    const int sleepTime = (BOOST_BRIGHTNESS_TIME + 50) * 1000; // wait for BoostBrightness timeout
    usleep(sleepTime);
    WaitDimmingDone();
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(beforeValue, currentValue);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessTimeout2
 * @tc.desc: Test BoostBrightness timeout brightness adjustment can be called
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessTimeout2, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 106;
    const uint32_t BOOST_BRIGHTNESS_TIME = 100;
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_BRIGHTNESS_TIME);
    EXPECT_TRUE(isBoost);
    const int sleepTime = (BOOST_BRIGHTNESS_TIME + 50) * 1000; // wait for BoostBrightness timeout
    usleep(sleepTime);
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_TRUE(isSet);
    WaitDimmingDone();
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, currentValue);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessCancel1
 * @tc.desc: Test BoostBrightness Cancel restore system brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessCancel1, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 106;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t BOOST_BRIGHTNESS_TIME = 10000;
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_BRIGHTNESS_TIME);
    EXPECT_TRUE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_TRUE(isCancel);
    WaitDimmingDone();
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, currentValue) << "beforeValue: " << SET_BRIGHTNESS << " currentVal: " << currentValue;
    DISPLAY_HILOGI(LABEL_TEST, "beforeValue: %{public}d, currentVal: %{public}d", SET_BRIGHTNESS, currentValue);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessCancel2
 * @tc.desc: Test BoostBrightness Cancel brightness adjustment can be called
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessCancel2, TestSize.Level0)
{
    const uint32_t BOOST_BRIGHTNESS_TIME = 10000;
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_BRIGHTNESS_TIME);
    EXPECT_TRUE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_TRUE(isCancel);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t SET_BRIGHTNESS_VALUE = 111;
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS_VALUE);
    EXPECT_TRUE(isSet);
    WaitDimmingDone();
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(SET_BRIGHTNESS_VALUE, currentValue);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessNotAdjust
 * @tc.desc: Test BoostBrightness do not adjust brightness, but update cachedSettingBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessNotAdjust, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 107;
    const uint32_t BOOST_BRIGHTNESS_TIME = 10000;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_BRIGHTNESS_TIME);
    EXPECT_TRUE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_FALSE(isSet);
    usleep(CMD_EXECUTION_DELAY);
    DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    WaitDimmingDone();
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, currentValue);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessScreenOff
 * @tc.desc: Test BoostBrightness off Disable the call
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessScreenOff, TestSize.Level0)
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t BOOST_BRIGHTNESS_TIME = 10000;
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(BOOST_BRIGHTNESS_TIME);
    EXPECT_FALSE(isBoost);
    usleep(CMD_EXECUTION_DELAY);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_FALSE(isCancel);
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessMultipleCalls
 * @tc.desc: Test BoostBrightness multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessMultipleCalls, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 108;
    const uint32_t COUNT = 3;
    const uint32_t TIMEOUT = 100;
    for (uint32_t i = 0; i < COUNT; ++i) {
        bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(TIMEOUT);
        EXPECT_TRUE(isBoost);
        usleep(CMD_EXECUTION_DELAY);
    }
    // Brightness adjustment is not allowed
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    EXPECT_FALSE(isSet);
    const int sleepTime = TIMEOUT * 1000; // wait for BoostBrightness timeout
    usleep(sleepTime);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_FALSE(isCancel);
    usleep(CMD_EXECUTION_DELAY);
    const uint32_t SET_BRIGHTNESS_VALUE = 222;
    isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS_VALUE);
    EXPECT_TRUE(isSet);
    WaitDimmingDone();
    uint32_t currentVal = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(SET_BRIGHTNESS_VALUE, currentVal);
}

/**
 * @tc.name: DisplayPowerMgrAdjustBrightness001
 * @tc.desc: Test AdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrAdjustBrightness001, TestSize.Level0)
{
    bool ret = DisplayPowerMgrClient::GetInstance().AdjustBrightness(0, 3000);
    EXPECT_TRUE(ret);
    DisplayPowerMgrClient::GetInstance().AdjustBrightness(0, 5);
}

/**
 * @tc.name: DisplayPowerMgrAdjustBrightness002
 * @tc.desc: Test AutoAdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrAdjustBrightness002, TestSize.Level0)
{
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    usleep(SET_AUTO_DELAY); // waiting to find correct isSettingEnable value
    if (ret) {
        DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness002: is supported");
        ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        EXPECT_TRUE(ret);
        usleep(SET_AUTO_DELAY);
    } else {
        DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness002: is not supported");
        EXPECT_FALSE(ret);
    }
}

/**
 * @tc.name: DisplayPowerMgrAdjustBrightness003
 * @tc.desc: Test IsAutoAdjustBrightness
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrAdjustBrightness003, TestSize.Level0)
{
    bool ret = DisplayPowerMgrClient::GetInstance().IsAutoAdjustBrightness();
    DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
    usleep(SET_AUTO_DELAY);
    EXPECT_EQ(DisplayPowerMgrClient::GetInstance().IsAutoAdjustBrightness(), false);
    DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(ret);
}

/**
 * @tc.name: DisplayPowerMgrScreenBrightnessEquality001
 * @tc.desc: Test whether the screen brightness is equal after the screen is on and before the screen is off
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrScreenBrightnessEquality001, TestSize.Level0)
{
    DisplayPowerMgrClient::GetInstance().AdjustBrightness(0, 5);
    usleep(SET_AUTO_DELAY);
    WaitDimmingDone();
    uint32_t deviceBrightness1 = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    usleep(SET_AUTO_DELAY);
    DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
    usleep(SET_AUTO_DELAY);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    WaitDimmingDone();
    uint32_t deviceBrightness2 = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(deviceBrightness1, deviceBrightness2);
}

/**
 * @tc.name: DisplayPowerMgrSleepBrightness001
 * @tc.desc: Test after entering the dim mode, settings brightness to remain unchanged
 * @tc.type: FUNC
 * @tc.require: issueI5YZQR
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSleepBrightness001, TestSize.Level0)
{
    uint32_t deviceBrightness1 = DisplayPowerMgrClient::GetInstance().GetBrightness();
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_DIM);
    WaitDimmingDone();
    uint32_t deviceBrightness2 = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(deviceBrightness1, deviceBrightness2);
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
}

/**
 * @tc.name: DisplayPowerMgrSetLightBrightnessThreshold001
 * @tc.desc: Test the Function of setting Ambient Light Monitor (success is returned)
 * @tc.type: FUNC
 * @tc.require: issueI8ZHFN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetLightBrightnessThreshold001, TestSize.Level0)
{
    std::vector<int32_t> threshold = {200, 200, 20};
    uint32_t type = 1;
    int32_t state = 0;
    const uint32_t SUCCESS_RESULT = 1;
    sptr<DisplayBrightnessCallbackStub> callback = new DisplayBrightnessCallbackStub();
    callback->OnNotifyApsLightBrightnessChange(type, state);
    uint32_t result = DisplayPowerMgrClient::GetInstance().SetLightBrightnessThreshold(threshold, callback);
    EXPECT_EQ(result, SUCCESS_RESULT);
}

/**
 * @tc.name: DisplayPowerMgrSetLightBrightnessThreshold002
 * @tc.desc: Test the Function of setting Ambient Light Monitor (success is returned)
 * @tc.type: FUNC
 * @tc.require: issueI8ZHFN
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetLightBrightnessThreshold002, TestSize.Level0)
{
    std::vector<int32_t> threshold = {};
    uint32_t type = 1;
    int32_t state = 0;
    const uint32_t FAILD_RESULT = 0;
    sptr<DisplayBrightnessCallbackStub> callback = new DisplayBrightnessCallbackStub();
    callback->OnNotifyApsLightBrightnessChange(type, state);
    uint32_t result = DisplayPowerMgrClient::GetInstance().SetLightBrightnessThreshold(threshold, callback);
    EXPECT_EQ(result, FAILD_RESULT);
}

/**
 * @tc.name: DisplayPowerMgrSetMaxBrightnessNit001
 * @tc.desc: Test if SetMaxBrightness is less than the current screen brightness,
 *           then turn down the screen brightness to the set value.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetMaxBrightnessNit001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 150; // mapping nit = 350
    const uint32_t SET_BRIGHTNESS_NIT = 350;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);

    const uint32_t SET_MAX_BRIGHTNESS = 98; // mapping nit = 231
    const uint32_t SET_MAX_BRIGHTNESS_NIT = 231;
    bool ret = DisplayPowerMgrClient::GetInstance().SetMaxBrightnessNit(SET_MAX_BRIGHTNESS_NIT, 2);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_MAX_BRIGHTNESS, brightness);
    DisplayPowerMgrClient::GetInstance().SetMaxBrightnessNit(SET_BRIGHTNESS_NIT, 1);
}

/**
 * @tc.name: DisplayPowerMgrSetMaxBrightnessNit002
 * @tc.desc: Test if SetMaxBrightness is higher than the current screen brightness, just set the max range
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetMaxBrightnessNit002, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 150; // mapping nit = 350
    const uint32_t SET_BRIGHTNESS_NIT = 350;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);

    const uint32_t SET_MAX_BRIGHTNESS_NIT = 469; // mapping level = 200;
    bool ret = DisplayPowerMgrClient::GetInstance().SetMaxBrightnessNit(SET_MAX_BRIGHTNESS_NIT, 2);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, brightness);
    DisplayPowerMgrClient::GetInstance().SetMaxBrightnessNit(SET_BRIGHTNESS_NIT, 1);
}

/**
 * @tc.name: DisplayPowerMgrSetMaxBrightness001
 * @tc.desc: Test if SetMaxBrightness is less than the current screen brightness,
 *           then turn down the screen brightness to the set value.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetMaxBrightness001, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 150;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);

    const uint32_t SET_MAX_BRIGHTNESS = 98;
    bool ret = DisplayPowerMgrClient::GetInstance().SetMaxBrightness((double)SET_MAX_BRIGHTNESS / 255, 2);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_MAX_BRIGHTNESS, brightness);
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS, 1);
}

/**
 * @tc.name: DisplayPowerMgrSetMaxBrightness002
 * @tc.desc: Test if SetMaxBrightness is higher than the current screen brightness, just set the max range
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSetMaxBrightness002, TestSize.Level0)
{
    const uint32_t SET_BRIGHTNESS = 150;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    usleep(CMD_EXECUTION_DELAY);

    const uint32_t SET_MAX_BRIGHTNESS = 200;
    bool ret = DisplayPowerMgrClient::GetInstance().SetMaxBrightness((double)SET_MAX_BRIGHTNESS / 255, 2);
    EXPECT_TRUE(ret);
    WaitDimmingDone();
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, brightness);
    DisplayPowerMgrClient::GetInstance().SetMaxBrightness(SET_BRIGHTNESS, 1);
}

/**
*@tc.name: NotifyBrightnessManagerScreenPowerStatus001
*@tc.desc: Test if NotifyBrightnessManagerScreenPowerStatus is work when dms notify
*          brightness manager POWER_STATUS_ON
*@tc.type: FUNC
*/
HWTEST_F(DisplayPowerMgrBrightnessTest, NotifyBrightnessManagerScreenPowerStatus001, TestSize.Level0)
{
    uint32_t displayId = 0;
    uint32_t screenPowerStatus = 0; // 0 represent the POWER_STATUS_ON
    uint32_t result = DisplayPowerMgrClient::GetInstance().NotifyBrightnessManagerScreenPowerStatus(displayId,
        screenPowerStatus);
    const uint32_t FAILED_RESULT = 0;
    EXPECT_EQ(result, FAILED_RESULT);
}

/**
*@tc.name: NotifyBrightnessManagerScreenPowerStatus002
*@tc.desc: Test if NotifyBrightnessManagerScreenPowerStatus is work when dms notify
*          brightness manager POWER_STATUS_OFF
*@tc.type: FUNC
*/
HWTEST_F(DisplayPowerMgrBrightnessTest, NotifyBrightnessManagerScreenPowerStatus002, TestSize.Level0)
{
    uint32_t displayId = 0;
    uint32_t screenPowerStatus = 3; // 3 represent the POWER_STATUS_OFF
    uint32_t result = DisplayPowerMgrClient::GetInstance().NotifyBrightnessManagerScreenPowerStatus(displayId,
        screenPowerStatus);
    const uint32_t FAILED_RESULT = 0;
    EXPECT_EQ(result, FAILED_RESULT);
}
} // namespace