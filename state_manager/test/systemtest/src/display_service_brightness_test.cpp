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

#include "display_service_brightness_test.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "display_power_mgr_client.h"
#include "ipc_object_stub.h"
#include "power_state_machine_info.h"
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
#include <hisysevent.h>
#endif
#include <memory>
#include <unistd.h>
#include "display_brightness_callback_stub.h"
#include "display_power_mgr_service.h"
#include "screen_manager_lite.h"
#include "permission.h"

using namespace testing;
using namespace testing::ext;
#ifdef HAS_HIVIEWDFX_HISYSEVENT_PART
using namespace OHOS::HiviewDFX;
#endif
using namespace OHOS::PowerMgr;
using namespace OHOS::DisplayPowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static constexpr uint32_t BRIGHTNESS_SETTING_VALUE = 50;
static constexpr uint32_t BRIGHTNESS_OVERRIDE_VALUE = 100;
static constexpr uint32_t BRIGHTNESS_DURATION = 0;
static constexpr uint32_t BRIGHTNESS_NONE_VALUE = 0;
static constexpr uint32_t DISPLAY_MAIN_ID = 0;
static constexpr uint32_t DISPLAY_INVALID_ID = 211;
static constexpr uint32_t REASON = 0;
static constexpr int32_t BRIGHTNESS_ADJUST_VALUE = 150;
static constexpr int32_t TIMEOUT_MS = 500;
static constexpr double DISCOUNT_VALUE = 0.30;
static constexpr uint32_t SET_SUCC = 1;
static constexpr uint32_t MAX_BRIGHTNESS_SET = 125;
static constexpr uint32_t TEST_MODE = 1;
static constexpr uint32_t DEFAULT_DURATION = 500;
static constexpr uint32_t BRIGHTNESS_OFF = 0;
static constexpr uint32_t DEFAULT_WAITING_TIME = 1200000;
sptr<DisplayPowerMgrService> g_service;
OHOS::Rosen::ScreenPowerState g_powerState = OHOS::Rosen::ScreenPowerState::POWER_ON;
bool g_isPermissionGranted = true;
} // namespace

namespace OHOS::PowerMgr {
bool Permission::IsSystem()
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest IsSystem, g_isPermissionGranted: %{pubilc}d",
        g_isPermissionGranted);
    return g_isPermissionGranted;
}
} // namespace OHOS::PowerMgr

namespace OHOS::Rosen {
bool DisplayManagerLite::SetScreenPowerById(
    Rosen::ScreenId screenId, Rosen::ScreenPowerState status, Rosen::PowerStateChangeReason reason)
{
    if (reason == Rosen::PowerStateChangeReason::STATE_CHANGE_REASON_UNKNOWN) {
        return false;
    }
    return true;
}
} // namespace OHOS::Rosen

void DisplayPowerServiceBrightnessTest::SetUpTestCase()
{
    g_service = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    g_service->Init();
}

void DisplayPowerServiceBrightnessTest::TearDownTestCase()
{
    g_service->Deinit();
    g_service->Reset();
}

namespace OHOS::Rosen {
bool DisplayManagerLite::SetDisplayState(DisplayState state, DisplayStateCallback callback)
{
    return true;
}

ScreenPowerState ScreenManagerLite::GetScreenPower()
{
    return g_powerState;
}
} //namespace OHOS::Rosen

namespace {
/**
 * @tc.name: DisplayPowerServiceBrightnessTest001
 * @tc.desc: test SetLightBrightnessThresholdInner function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest001 function start!");
    EXPECT_TRUE(g_service != nullptr);
    std::vector<int32_t> threshold = {200, 200, 20};
    sptr<DisplayBrightnessCallbackStub> callback = new DisplayBrightnessCallbackStub();
    uint32_t result = g_service->SetLightBrightnessThresholdInner(threshold, callback);
    EXPECT_EQ(result, SET_SUCC);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest001 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest002
 * @tc.desc: test SetMaxBrightness function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest002 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    int32_t errCode = 0;
    uint32_t result = g_service->SetMaxBrightness(MAX_BRIGHTNESS_SET, TEST_MODE, ret, errCode);
    uint32_t maxBrightness = 0;
    g_service->GetMaxBrightness(maxBrightness);
    EXPECT_TRUE(ret);
    EXPECT_NE(maxBrightness, MAX_BRIGHTNESS_SET);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest002 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest003
 * @tc.desc: test SetBrightness function SetBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest003 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    int32_t errCode = 0;
    g_service->SetDisplayState(DISPLAY_MAIN_ID, static_cast<uint32_t>(DisplayPowerMgr::DisplayState::DISPLAY_ON),
        REASON, ret);
    g_service->SetBrightness(BRIGHTNESS_SETTING_VALUE, DISPLAY_MAIN_ID, false, ret, errCode);
    EXPECT_TRUE(ret);
    EXPECT_EQ(errCode, ERR_OK);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest003 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest004
 * @tc.desc: test DiscountBrightness function
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest004, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest004 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->DiscountBrightness(DISCOUNT_VALUE, DISPLAY_MAIN_ID, ret);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest004 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest005
 * @tc.desc: test DisplayPowerMgrService function RestoreBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest005, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest005 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->RestoreBrightness(DISPLAY_MAIN_ID, DEFAULT_DURATION, ret);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest005 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest006
 * @tc.desc: test DisplayPowerMgrService function GetBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest006, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest006 function start!");
    EXPECT_TRUE(g_service != nullptr);
    uint32_t brightness = 0;
    g_service->GetBrightness(DISPLAY_MAIN_ID, brightness);
    EXPECT_NE(brightness, BRIGHTNESS_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest006 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest007
 * @tc.desc: test DisplayPowerMgrService function GetDefaultBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest007, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest007 function start!");
    EXPECT_TRUE(g_service != nullptr);
    uint32_t brightness = 0;
    g_service->GetDefaultBrightness(brightness);
    EXPECT_EQ(brightness, DisplayPowerMgrService::BRIGHTNESS_DEFAULT);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest007 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest008
 * @tc.desc: test DisplayPowerMgrService function GetMinBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest008, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest008 function start!");
    EXPECT_TRUE(g_service != nullptr);
    uint32_t brightness = 0;
    g_service->GetMinBrightness(brightness);
    EXPECT_EQ(brightness, DisplayPowerMgrService::BRIGHTNESS_MIN);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest008 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest009
 * @tc.desc: test DisplayPowerMgrService function AdjustBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest009, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest009 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->AdjustBrightness(DISPLAY_MAIN_ID, BRIGHTNESS_ADJUST_VALUE, BRIGHTNESS_DURATION, ret);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest009 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest010
 * @tc.desc: test DisplayPowerMgrService function IsAutoAdjustBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest010, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest010 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->AutoAdjustBrightness(false, ret);
    g_service->IsAutoAdjustBrightness(ret);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest010 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest011
 * @tc.desc: test DisplayPowerMgrService function BoostBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest011, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest011 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->BoostBrightness(TIMEOUT_MS, DISPLAY_MAIN_ID, ret);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest011 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest012
 * @tc.desc: test DisplayPowerMgrService function CancelBoostBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest012, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest012 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->CancelBoostBrightness(DISPLAY_MAIN_ID, ret);
    EXPECT_TRUE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest012 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest013
 * @tc.desc: test DisplayPowerMgrService function GetDeviceBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest013, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest013 function start!");
    EXPECT_TRUE(g_service != nullptr);
    uint32_t brightness = 0;
    g_service->GetDeviceBrightness(DISPLAY_MAIN_ID, brightness);
    EXPECT_NE(brightness, BRIGHTNESS_NONE_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest013 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest014
 * @tc.desc: test DisplayPowerMgrService function SetLightBrightnessThreshold
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest014, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest014 function start!");
    EXPECT_TRUE(g_service != nullptr);
    uint32_t ret = 0;
    std::vector<int32_t> threshold = {200, 200, 20};
    sptr<DisplayBrightnessCallbackStub> callback = new DisplayBrightnessCallbackStub();
    g_service->SetLightBrightnessThreshold(threshold, callback, ret);
    EXPECT_EQ(ret, SET_SUCC);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest014 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest015
 * @tc.desc: test DisplayPowerMgrService function SetMaxBrightnessNit
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest015, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest015 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    int32_t errCode = 0;
    uint32_t result = g_service->SetMaxBrightnessNit(MAX_BRIGHTNESS_SET, TEST_MODE, ret, errCode);
    EXPECT_TRUE(ret);
    uint32_t maxBrightness = 0;
    g_service->GetMaxBrightness(maxBrightness);
    EXPECT_NE(maxBrightness, MAX_BRIGHTNESS_SET);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest015 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest016
 * @tc.desc: test DisplayPowerMgrService function SetBootCompletedBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest016, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest016 function start!");
    EXPECT_TRUE(g_service != nullptr);
    const int sleepTime = DEFAULT_WAITING_TIME;
    usleep(sleepTime);
    uint32_t brightnessTemp = BRIGHTNESS_OFF;
    g_service->GetBrightness(DISPLAY_MAIN_ID, brightnessTemp);
    bool enable = g_service->GetSettingAutoBrightness();
    g_service->SetBootCompletedBrightness();
    g_service->SetBootCompletedAutoBrightness();
    g_service->RegisterSettingObservers();
    uint32_t brightness = BRIGHTNESS_OFF;
    g_service->GetBrightness(DISPLAY_MAIN_ID, brightness);
    EXPECT_EQ(brightness, brightnessTemp);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest016 function end!");
}

/**
 * @tc.name: DisplayPowerServiceBrightnessTest017
 * @tc.desc: test DisplayPowerMgrService function OverrideBrightness
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DisplayPowerServiceBrightnessTest, DisplayPowerServiceBrightnessTest017, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest017 function start!");
    EXPECT_TRUE(g_service != nullptr);
    bool ret = false;
    g_service->OverrideBrightness(BRIGHTNESS_OVERRIDE_VALUE, DISPLAY_INVALID_ID, DEFAULT_DURATION, ret);
    EXPECT_FALSE(ret);
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerServiceBrightnessTest017 function end!");
}
} // namespace
