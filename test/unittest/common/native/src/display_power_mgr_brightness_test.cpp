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
#include "power_setting_helper.h"
#include "system_ability_definition.h"
#include "display_log.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

namespace {
const std::string SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
const double NO_DISCOUNT = 1.00;
}

class DisplayPowerMgrBrightnessTest : public Test {
public:
    void SetUp()
    {
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
        DisplayPowerMgrClient::GetInstance().DiscountBrightness(NO_DISCOUNT);
    }

    void TearDown()
    {
        DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
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
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness001: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(ret);
    uint32_t deviceBrightness = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(deviceBrightness, 100);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrSettingBrightness001
 * @tc.desc: Test setting brightness value changed when SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSettingBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrSettingBrightness001: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(223);
    EXPECT_TRUE(ret);
    sleep(1); // wait for setting update
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(brightness, 223);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrSettingBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrSettingBrightness002
 * @tc.desc: Test change setting brightness value then system brightness changed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrSettingBrightness002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrSettingBrightness002: fun is start");
    PowerMgr::PowerSettingHelper& helper = PowerMgr::PowerSettingHelper::GetInstance(DISPLAY_MANAGER_SERVICE_ID);
    ErrCode code = helper.PutIntValue(SETTING_BRIGHTNESS_KEY, 233);
    if (code != ERR_OK) {
        DISPLAY_HILOGW(LABEL_TEST, "put setting brightness failed, ret=%{public}d", code);
        FAIL();
    }
    sleep(1); // wait for setting update

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrSettingBrightness002: value is %{public}u", value);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrSettingBrightness002: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessNormal
 * @tc.desc: Test DiscountBrightness the normal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessNormal, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessNormal: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_NORMAL = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_NORMAL);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(SET_BRIGHTNESS * DISCOUNT_NORMAL));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessNormal: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessNormal
 * @tc.desc: Test DiscountBrightness when screen is off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessScreenOff, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessScreenOff: fun is start");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    const double DISCOUNT_VALUE = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessScreenOff: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessBoundary001
 * @tc.desc: Test DiscountBrightness the boundary test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessBoundary001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessBoundary001: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_LOWER_BOUNDARY = 0.01;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_LOWER_BOUNDARY);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    uint32_t minBrightness = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_EQ(value, minBrightness);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessBoundary001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessBoundary002
 * @tc.desc: Test DiscountBrightness the boundary test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessBoundary002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessBoundary002: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_UPPER_BOUNDARY = 1.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_UPPER_BOUNDARY);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(SET_BRIGHTNESS * DISCOUNT_UPPER_BOUNDARY));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessBoundary002: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessAbnormal001
 * @tc.desc: Test DiscountBrightness the abnormal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessAbnormal001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessAbnormal001: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_BEYOND_LOWER_BOUNDARY = 0.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_BEYOND_LOWER_BOUNDARY);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    uint32_t minBrightness = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_EQ(value, minBrightness);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessAbnormal001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessAbnormal002
 * @tc.desc: Test DiscountBrightness the abnormal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessAbnormal002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessAbnormal002: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_BEYOND_UPPER_BOUNDARY = 2.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_BEYOND_UPPER_BOUNDARY);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    const double DISCOUNT_MAX_VALUE = 1.0;
    EXPECT_EQ(value, static_cast<uint32_t>(SET_BRIGHTNESS * DISCOUNT_MAX_VALUE));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessAbnormal002: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightnessAbnormal003
 * @tc.desc: Test DiscountBrightness the abnormal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightnessAbnormal003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessAbnormal003: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_NEGATIVE_VALUE = -1.0;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_NEGATIVE_VALUE);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    uint32_t minBrightness = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_EQ(value, minBrightness);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightnessAbnormal003: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness001
 * @tc.desc: Test OverrideBrightness after DiscountBrightness, then RestoreBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness001: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_VALUE = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);

    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));

    const uint32_t SET_OVERRIDE_BRIGHTNESS = 200;
    DisplayPowerMgrClient::GetInstance().OverrideBrightness(SET_OVERRIDE_BRIGHTNESS);
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_OVERRIDE_BRIGHTNESS));

    DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness002
 * @tc.desc: Test DiscountBrightness after OverrideBrightness, then RestoreBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness002: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const uint32_t SET_OVERRIDE_BRIGHTNESS = 200;
    DisplayPowerMgrClient::GetInstance().OverrideBrightness(SET_OVERRIDE_BRIGHTNESS);

    const double DISCOUNT_VALUE = 0.8;
    double ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_OVERRIDE_BRIGHTNESS));

    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_TRUE(ret);

    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness002: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness003
 * @tc.desc: Test DisconutBrightness after BoostBrightness and CancelBoostBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness003: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const uint32_t SET_BOOST_BRIGHTNESS = 10000;
    DisplayPowerMgrClient::GetInstance().BoostBrightness(SET_BOOST_BRIGHTNESS);

    const double DISCOUNT_VALUE = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);

    uint32_t maxBrightness = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * maxBrightness));

    DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();

    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness003: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness004
 * @tc.desc: Test BoostBrightness after DiscountBrightness, then CancelBoostBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness004: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_VALUE = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);

    const uint32_t SET_BOOST_BRIGHTNESS = 10000;
    DisplayPowerMgrClient::GetInstance().BoostBrightness(SET_BOOST_BRIGHTNESS);
    uint32_t maxBrightness = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * maxBrightness));

    DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();

    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, static_cast<uint32_t>(DISCOUNT_VALUE * SET_BRIGHTNESS));
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness004: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDiscountBrightness005
 * @tc.desc: Test DisconutBrightness after SetBrightness, then GetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDiscountBrightness005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness005: fun is start");
    const uint32_t SET_BRIGHTNESS = 100;
    DisplayPowerMgrClient::GetInstance().SetBrightness(SET_BRIGHTNESS);
    const double DISCOUNT_VALUE = 0.8;
    bool ret = DisplayPowerMgrClient::GetInstance().DiscountBrightness(DISCOUNT_VALUE);
    EXPECT_TRUE(ret);

    uint32_t beforeBrightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    EXPECT_EQ(SET_BRIGHTNESS, beforeBrightness);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDiscountBrightness005: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness001
 * @tc.desc: Test OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness002: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, 255);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness002: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness002
 * @tc.desc: Test RestoreBrightness after OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness003: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, 255);
    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness(); // restore
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, 100);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness003: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness003
 * @tc.desc: Test SetBrightness after OverrideBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness004: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_FALSE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, 255);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness004: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness004
 * @tc.desc: Test RestoreBrightness after not override
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness005: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    ret = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_FALSE(ret);
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, 100);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness005: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrOverrideBrightness005
 * @tc.desc: Test OverrideBrightness off Disable the call
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrOverrideBrightness005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness006: fun is start");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    bool isBoost = DisplayPowerMgrClient::GetInstance().OverrideBrightness(100);
    EXPECT_FALSE(isBoost);
    bool isRestore = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_FALSE(isRestore);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBrightness006: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrMaxBrightness001
 * @tc.desc: Test GetMaxBrightness less equals 255
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrMaxBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrMaxBrightness001: fun is start");
    uint32_t max = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
    EXPECT_LE(max, 255);
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(max + 100);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, max);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrMaxBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrMinBrightness001
 * @tc.desc: Test GetMinBrightness greater equals 0
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrMinBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrMinBrightness001: fun is start");
    uint32_t min = DisplayPowerMgrClient::GetInstance().GetMinBrightness();
    EXPECT_GE(min, 0);
    bool ret = DisplayPowerMgrClient::GetInstance().SetBrightness(0);
    EXPECT_TRUE(ret);
    sleep(1); // wait for gradual animation
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(value, min);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrMinBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrDefaultBrightness001
 * @tc.desc: Test GetDefaultnBrightness greater equals 0 and less equals 255
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrDefaultBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDefaultBrightness001: fun is start");
    uint32_t value = DisplayPowerMgrClient::GetInstance().GetDefaultBrightness();
    EXPECT_GE(value, 0);
    EXPECT_LE(value, 255);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrDefaultBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessNormal
 * @tc.desc: Test BoostBrightness the normal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessNormal, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessNormal: fun is start");
    bool isSucc = DisplayPowerMgrClient::GetInstance().BoostBrightness(1000);
    EXPECT_TRUE(isSucc);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessNormal: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessAbnormal
 * @tc.desc: Test BoostBrightness the abnormal value
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessAbnormal, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessAbnormal: fun is start");
    bool isSucc = DisplayPowerMgrClient::GetInstance().BoostBrightness(-1);
    EXPECT_FALSE(isSucc);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessAbnormal: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrCancelBoostBrightnessNormal
 * @tc.desc: Test CancelBoostBrightness the normal test
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrCancelBoostBrightnessNormal, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrCancelBoostBrightnessNormal: fun is start");
    bool isSucc = DisplayPowerMgrClient::GetInstance().BoostBrightness(50000);
    EXPECT_TRUE(isSucc);
    sleep(1);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_TRUE(isCancel);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrCancelBoostBrightnessNormal: fun is end");
}

/**
 * @tc.name: BoostAndOverrideMutuallyExclusive1
 * @tc.desc: Test BoostBrightness and OverrideBrightness are mutually exclusive
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, BoostAndOverrideMutuallyExclusive1, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BoostAndOverrideMutuallyExclusive: fun is start");
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(50000);
    EXPECT_TRUE(isBoost);
    sleep(1);
    bool isOverride = DisplayPowerMgrClient::GetInstance().OverrideBrightness(100);
    EXPECT_FALSE(isOverride);
    bool isRestore = DisplayPowerMgrClient::GetInstance().RestoreBrightness();
    EXPECT_FALSE(isRestore);
    DISPLAY_HILOGI(LABEL_TEST, "BoostAndOverrideMutuallyExclusive: fun is end");
}

/**
 * @tc.name: BoostAndOverrideMutuallyExclusive2
 * @tc.desc: Test BoostBrightness and OverrideBrightness are mutually exclusive
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, BoostAndOverrideMutuallyExclusive2, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BoostAndOverrideMutuallyExclusive2: fun is start");
    bool isOverride = DisplayPowerMgrClient::GetInstance().OverrideBrightness(255);
    EXPECT_TRUE(isOverride);
    sleep(1);
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(50000);
    EXPECT_FALSE(isBoost);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_FALSE(isCancel);
    DISPLAY_HILOGI(LABEL_TEST, "BoostAndOverrideMutuallyExclusive2: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessTimeout1
 * @tc.desc: Test BoostBrightness timeout restore system brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessTimeout1, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessTimeout1: fun is start");
    uint32_t beforeValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(1000);
    EXPECT_TRUE(isBoost);
    sleep(2);
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(beforeValue, currentValue);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessTimeout1: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessTimeout2
 * @tc.desc: Test BoostBrightness timeout brightness adjustment can be called
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessTimeout2, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessTimeout2: fun is start");
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(1000);
    EXPECT_TRUE(isBoost);
    sleep(2);
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_TRUE(isSet);
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(100, currentValue);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessTimeout2: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessCancel1
 * @tc.desc: Test BoostBrightness Cancel restore system brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessCancel1, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessCancel1: fun is start");
    uint32_t beforeValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(10000);
    EXPECT_TRUE(isBoost);
    sleep(1);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_TRUE(isCancel);
    sleep(1);
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(beforeValue, currentValue) << "beforeValue: " << beforeValue << " currentVal: " << currentValue;
    DISPLAY_HILOGI(LABEL_TEST, "BoostBrightnessCancel1: beforeValue: %{public}d, currentVal: %{public}d",
        beforeValue, currentValue);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessCancel1: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessCancel2
 * @tc.desc: Test BoostBrightness Cancel brightness adjustment can be called
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessCancel2, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessCancel2: fun is start");
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(10000);
    EXPECT_TRUE(isBoost);
    sleep(1);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_TRUE(isCancel);
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(111);
    EXPECT_TRUE(isSet);
    uint32_t currentValue = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(111, currentValue);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessCancel2: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessNotAdjust
 * @tc.desc: Test BoostBrightness do not adjust brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessNotAdjust, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessNotAdjust: fun is start");
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(10000);
    EXPECT_TRUE(isBoost);
    sleep(1);
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(100);
    EXPECT_FALSE(isSet);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessNotAdjust: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessScreenOff
 * @tc.desc: Test BoostBrightness off Disable the call
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessScreenOff, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessScreenOff: fun is start");
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(10000);
    EXPECT_FALSE(isBoost);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_FALSE(isCancel);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessScreenOff: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrBoostBrightnessMultipleCalls
 * @tc.desc: Test BoostBrightness multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrBoostBrightnessMultipleCalls, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessMultipleCalls: fun is start");
    const uint32_t COUNT = 3;
    const uint32_t TIMEOUT = 1000;
    for (uint32_t i = 0; i < COUNT; ++i) {
        bool isBoost = DisplayPowerMgrClient::GetInstance().BoostBrightness(TIMEOUT);
        EXPECT_TRUE(isBoost);
    }
    // Brightness adjustment is not allowed
    bool isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(123);
    EXPECT_FALSE(isSet);
    // Wait for a timeout
    sleep(COUNT + 1);
    bool isCancel = DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();
    EXPECT_FALSE(isCancel);
    isSet = DisplayPowerMgrClient::GetInstance().SetBrightness(222);
    EXPECT_TRUE(isSet);
    uint32_t currentVal = DisplayPowerMgrClient::GetInstance().GetDeviceBrightness();
    EXPECT_EQ(222, currentVal);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrBoostBrightnessMultipleCalls: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrAdjustBrightness001
 * @tc.desc: Test AdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrAdjustBrightness001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness001: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().AdjustBrightness(0, 3000);
    sleep(1);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness001: fun is end");
}

/**
 * @tc.name: DisplayPowerMgrAdjustBrightness002
 * @tc.desc: Test AutoAdjustBrightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrBrightnessTest, DisplayPowerMgrAdjustBrightness002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness002: fun is start");
    bool ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(true);
    sleep(1);
    if (ret) {
        DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness002: is supported");
        ret = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(false);
        EXPECT_TRUE(ret);
    } else {
        DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrAdjustBrightness002: is not supported");
        EXPECT_FALSE(ret);
    }
}
}