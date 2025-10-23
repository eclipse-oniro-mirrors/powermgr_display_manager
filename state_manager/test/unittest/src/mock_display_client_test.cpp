/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "mock_display_client_test.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>
#include <vector>

#include "display_power_mgr_client.h"
#include "display_log.h"
#include "mock_display_power_mgr_proxy.h"
#include "mock_display_remote_object.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
namespace {
sptr<IRemoteObject> g_testRemoteObj;
sptr<MockDisplayPowerMgrProxy> g_mockProxy;
}

void DisplayPowerMgrClientMockTest::SetUp()
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
    g_testRemoteObj = sptr<PowerMgr::MockDisplayRemoteObject>::MakeSptr(u"DisplayPowerMgrClientMockTest");
    g_mockProxy = sptr<MockDisplayPowerMgrProxy>::MakeSptr(g_testRemoteObj);
}

void DisplayPowerMgrClientMockTest::TearDown()
{
    DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_OFF);
    g_testRemoteObj = nullptr;
    g_mockProxy = nullptr;
}

void DisplayPowerMgrClientMockTest::DisplayPowerMgrTestCallback::OnDisplayStateChanged(
    uint32_t displayId, DisplayPowerMgr::DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrTestCallback::OnDisplayStateChanged");
}

namespace {
/**
 * @tc.name: DisplayPowerMgrClient001
 * @tc.desc: test SetDisplayState() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient001 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    bool ret = client.SetDisplayState(DisplayState::DISPLAY_OFF);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient001 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient002
 * @tc.desc: test GetDisplayState() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient002 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto displayState = client.GetDisplayState();
    client.proxy_ = proxy;
    EXPECT_EQ(displayState, DisplayState::DISPLAY_UNKNOWN);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient002 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient003
 * @tc.desc: test GetDisplayIds() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient003 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ids = client.GetDisplayIds();
    client.proxy_ = proxy;
    EXPECT_TRUE(ids.size() == 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient003 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient004
 * @tc.desc: test GetMainDisplayId() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient004 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto id = client.GetMainDisplayId();
    client.proxy_ = proxy;
    EXPECT_EQ(id, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient004 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient005
 * @tc.desc: test SetBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient004 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_BRIGHTNESS = 150;
    auto result = client.SetBrightness(SET_BRIGHTNESS);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient005 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient006
 * @tc.desc: test SetMaxBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient006 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_MAX_BRIGHTNESS = 231;
    auto result = client.SetMaxBrightness(SET_MAX_BRIGHTNESS,
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
}

/**
 * @tc.name: DisplayPowerMgrClient007
 * @tc.desc: test SetMaxBrightnessNit() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient006 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_MAX_BRIGHTNESS = 231;
    auto result = client.SetMaxBrightnessNit(SET_MAX_BRIGHTNESS,
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient007 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient008
 * @tc.desc: test SetMaxBrightnessNit() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient008, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient008 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const double NO_DISCOUNT = 1.00;
    auto result = client.DiscountBrightness(NO_DISCOUNT);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient008 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient009
 * @tc.desc: test OverrideBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient009, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient009 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t SET_OVERRIDE_BRIGHTNESS = 202;
    auto result = client.OverrideBrightness(SET_OVERRIDE_BRIGHTNESS);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient009 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient010
 * @tc.desc: test OverrideDisplayOffDelay() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient010, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient010 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t OVERRIDE_DELAY_TIME = 200;
    auto result = client.OverrideDisplayOffDelay(OVERRIDE_DELAY_TIME);
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient010 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient011
 * @tc.desc: test RestoreBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient011, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient011 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto result = client.RestoreBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient011 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient012
 * @tc.desc: test GetBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient012 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient012 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient013
 * @tc.desc: test GetDefaultBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient013 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient013 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient014
 * @tc.desc: test GetDefaultBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient014, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient013 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetDefaultBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient014 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient015
 * @tc.desc: test GetMaxBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient015, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient015 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetMaxBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient015 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient016
 * @tc.desc: test GetMinBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient016, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient016 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetMinBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient016 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient017
 * @tc.desc: test AdjustBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient017, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient017 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t ADJUST_BRIGHTNESS = 200;
    auto ret = client.AdjustBrightness(OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID,
        ADJUST_BRIGHTNESS);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient017 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient018
 * @tc.desc: test AutoAdjustBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient018, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient018 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.AutoAdjustBrightness(true);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient018 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient019
 * @tc.desc: test IsAutoAdjustBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient019, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient019 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.IsAutoAdjustBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient019 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient020
 * @tc.desc: test BoostBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient020, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient020 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    const uint32_t BOOST_TIMEOUT_MS = 200;
    auto ret = client.BoostBrightness(BOOST_TIMEOUT_MS);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient020 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient021
 * @tc.desc: test CancelBoostBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient021, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient021 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.CancelBoostBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient021 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient022
 * @tc.desc: test GetDeviceBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient022, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient022 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto brightness = client.GetDeviceBrightness();
    client.proxy_ = proxy;
    EXPECT_EQ(brightness, OHOS::DisplayPowerMgr::DisplayPowerMgrClient::BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient022 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient023
 * @tc.desc: test SetCoordinated() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient023, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient023 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.SetCoordinated(true);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient023 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient024
 * @tc.desc: test SetLightBrightnessThreshold() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient024, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient024 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    std::vector<int32_t> threshold = {};
    auto ret = client.SetLightBrightnessThreshold(threshold, nullptr);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient024 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient025
 * @tc.desc: test RegisterCallback() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient025, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient025 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    sptr<IDisplayPowerCallback> callback = new DisplayPowerMgrClientMockTest::DisplayPowerMgrTestCallback();
    auto ret = client.RegisterCallback(callback);
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient025 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient026
 * @tc.desc: test SetScreenOnBrightness() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient026, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient026 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.SetScreenOnBrightness();
    client.proxy_ = proxy;
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient026 function end!");
}

/**
 * @tc.name: DisplayPowerMgrClient027
 * @tc.desc: test NotifyBrightnessManagerScreenPowerStatus() when proxy return fail
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerMgrClientMockTest, DisplayPowerMgrClient027, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient027 function start!");
    auto& client = DisplayPowerMgrClient::GetInstance();
    auto proxy = client.proxy_;
    client.proxy_ = g_mockProxy;
    auto ret = client.NotifyBrightnessManagerScreenPowerStatus(
        OHOS::DisplayPowerMgr::DisplayPowerMgrClient::DEFAULT_MAIN_DISPLAY_ID,
        static_cast<uint32_t>(DisplayState::DISPLAY_ON));
    client.proxy_ = proxy;
    static constexpr int32_t DEFAULT_VALUE = -1;
    EXPECT_EQ(ret, DEFAULT_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrClient027 function end!");
}
} // namespace
