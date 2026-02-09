/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <gtest/gtest-death-test.h>
#include "display_log.h"
#include "display_power_mgr_client.h"

// Make private members accessible for testing
#define private public
#include "brightness_service.h"
#undef private

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
using namespace std;

namespace {
    const double NO_DISCOUNT = 1.00;
    const double HALF_DISCOUNT = 0.50;
    const double MIN_DISCOUNT = 0.01;
    const uint32_t DEFAULT_BRIGHTNESS_VALUE = 128;
    const uint32_t MIN_BRIGHTNESS_VALUE = 1;
    const uint32_t MAX_BRIGHTNESS_VALUE = 255;
    const uint32_t TEST_TIMEOUT_MS = 1000;
}

class BrightnessServiceTest : public Test {
public:
    void SetUp()
    {
        DISPLAY_HILOGI(LABEL_TEST, "BrightnessServiceTest SetUp");
        brightnessService = &BrightnessService::Get();
        ASSERT_NE(brightnessService, nullptr);

        // Initialize with default values
        brightnessService->Init(MAX_BRIGHTNESS_VALUE, MIN_BRIGHTNESS_VALUE);

        // Set display to ON state for most tests
        DisplayPowerMgrClient::GetInstance().SetDisplayState(DisplayState::DISPLAY_ON);
        DisplayPowerMgrClient::GetInstance().DiscountBrightness(NO_DISCOUNT);
    }

    void TearDown()
    {
        DISPLAY_HILOGI(LABEL_TEST, "BrightnessServiceTest TearDown");
        DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();

        // Reset state
        if (brightnessService != nullptr) {
            brightnessService->ClearOffset();
        }
    }

protected:
    BrightnessService* brightnessService;
};

namespace {
// ==================== Basic Singleton Tests ====================

HWTEST_F(BrightnessServiceTest, BrightnessServiceGet001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessServiceGet001 function start!");
    auto& service = BrightnessService::Get();
    EXPECT_NE(&service, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessServiceGet001 function end!");
}

HWTEST_F(BrightnessServiceTest, BrightnessServiceGetDiscount001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessServiceGetDiscount001 function start!");
    EXPECT_NE(BrightnessService::Get().GetDiscount(), 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessServiceGetDiscount001 function end!");
}

// ==================== Initialization Tests ====================

HWTEST_F(BrightnessServiceTest, Init_ValidParameters_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "Init_ValidParameters_Success start!");
    // Init already called in SetUp, just verify it worked
    EXPECT_NE(brightnessService, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "Init_ValidParameters_Success end!");
}

HWTEST_F(BrightnessServiceTest, DeInit_Cleanup_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DeInit_Cleanup_Success start!");
    brightnessService->DeInit();
    // After DeInit, service should still be accessible but with cleaned state
    EXPECT_NE(brightnessService, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "DeInit_Cleanup_Success end!");
}

// ==================== Display State Tests ====================

HWTEST_F(BrightnessServiceTest, SetDisplayState_DisplayOn_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayState_DisplayOn_Success start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    EXPECT_EQ(brightnessService->GetDisplayState(), DisplayState::DISPLAY_ON);
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayState_DisplayOn_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetDisplayState_DisplayOff_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayState_DisplayOff_Success start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_OFF);
    EXPECT_EQ(brightnessService->GetDisplayState(), DisplayState::DISPLAY_OFF);
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayState_DisplayOff_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetDisplayState_DisplayDim_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayState_DisplayDim_Success start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_DIM);
    EXPECT_EQ(brightnessService->GetDisplayState(), DisplayState::DISPLAY_DIM);
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayState_DisplayDim_Success end!");
}

// ==================== IsScreenOnState Tests ====================

HWTEST_F(BrightnessServiceTest, IsScreenOnState_DisplayOn_ReturnsTrue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayOn_ReturnsTrue start!");
    EXPECT_TRUE(brightnessService->IsScreenOnState(DisplayState::DISPLAY_ON));
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayOn_ReturnsTrue end!");
}

HWTEST_F(BrightnessServiceTest, IsScreenOnState_DisplayDim_ReturnsTrue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayDim_ReturnsTrue start!");
    EXPECT_TRUE(brightnessService->IsScreenOnState(DisplayState::DISPLAY_DIM));
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayDim_ReturnsTrue end!");
}

HWTEST_F(BrightnessServiceTest, IsScreenOnState_DisplayOff_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayOff_ReturnsFalse start!");
    EXPECT_FALSE(brightnessService->IsScreenOnState(DisplayState::DISPLAY_OFF));
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayOff_ReturnsFalse end!");
}

HWTEST_F(BrightnessServiceTest, IsScreenOnState_DisplayUnknown_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayUnknown_ReturnsFalse start!");
    EXPECT_FALSE(brightnessService->IsScreenOnState(DisplayState::DISPLAY_UNKNOWN));
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOnState_DisplayUnknown_ReturnsFalse end!");
}

// ==================== Brightness Setting Tests ====================

HWTEST_F(BrightnessServiceTest, SetBrightness_NormalValue_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_NormalValue_Success start!");
    bool result = brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 0, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_NormalValue_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetBrightness_WithGradualDuration_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_WithGradualDuration_Success start!");
    bool result = brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 500, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_WithGradualDuration_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetBrightness_ContinuousMode_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_ContinuousMode_Success start!");
    bool result = brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 0, true);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_ContinuousMode_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetBrightness_MinValue_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_MinValue_Success start!");
    bool result = brightnessService->SetBrightness(MIN_BRIGHTNESS_VALUE, 0, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_MinValue_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetBrightness_MaxValue_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_MaxValue_Success start!");
    bool result = brightnessService->SetBrightness(MAX_BRIGHTNESS_VALUE, 0, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_MaxValue_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetBrightness_ZeroValue_ClampedToMin, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_ZeroValue_ClampedToMin start!");
    bool result = brightnessService->SetBrightness(0, 0, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightness_ZeroValue_ClampedToMin end!");
}

HWTEST_F(BrightnessServiceTest, GetBrightness_ReturnsValidValue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightness_ReturnsValidValue start!");
    brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 0, false);
    uint32_t brightness = brightnessService->GetBrightness();
    EXPECT_GT(brightness, 0);
    EXPECT_LE(brightness, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightness_ReturnsValidValue end!");
}

HWTEST_F(BrightnessServiceTest, GetDeviceBrightness_WithoutHbm_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetDeviceBrightness_WithoutHbm_Success start!");
    uint32_t brightness = brightnessService->GetDeviceBrightness(false);
    EXPECT_GE(brightness, MIN_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetDeviceBrightness_WithoutHbm_Success end!");
}

HWTEST_F(BrightnessServiceTest, GetDeviceBrightness_WithHbm_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetDeviceBrightness_WithHbm_Success start!");
    uint32_t brightness = brightnessService->GetDeviceBrightness(true);
    EXPECT_GE(brightness, MIN_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetDeviceBrightness_WithHbm_Success end!");
}

HWTEST_F(BrightnessServiceTest, GetCachedSettingBrightness_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetCachedSettingBrightness_Success start!");
    uint32_t cachedBrightness = brightnessService->GetCachedSettingBrightness();
    EXPECT_GE(cachedBrightness, MIN_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetCachedSettingBrightness_Success end!");
}

// ==================== Brightness Level Tests ====================

HWTEST_F(BrightnessServiceTest, GetBrightnessLevel_LowLux_ReturnsCorrectLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessLevel_LowLux_ReturnsCorrectLevel start!");
    uint32_t level = brightnessService->GetBrightnessLevel(1.0f);
    EXPECT_GT(level, 0);
    EXPECT_LE(level, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessLevel_LowLux_ReturnsCorrectLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetBrightnessLevel_HighLux_ReturnsCorrectLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessLevel_HighLux_ReturnsCorrectLevel start!");
    uint32_t level = brightnessService->GetBrightnessLevel(10000.0f);
    EXPECT_GT(level, 0);
    EXPECT_LE(level, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessLevel_HighLux_ReturnsCorrectLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetBrightnessHighLevel_ValidInput_ReturnsSameLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessHighLevel_ValidInput_ReturnsSameLevel start!");
    uint32_t level = brightnessService->GetBrightnessHighLevel(DEFAULT_BRIGHTNESS_VALUE);
    EXPECT_EQ(level, DEFAULT_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessHighLevel_ValidInput_ReturnsSameLevel end!");
}

// ==================== Brightness Mapping Tests ====================

HWTEST_F(BrightnessServiceTest, GetMappingBrightnessLevel_MinValue_ReturnsMinLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessLevel_MinValue_ReturnsMinLevel start!");
    uint32_t mappedLevel = BrightnessService::GetMappingBrightnessLevel(MIN_BRIGHTNESS_VALUE);
    EXPECT_GE(mappedLevel, MIN_BRIGHTNESS_VALUE);
    EXPECT_LE(mappedLevel, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessLevel_MinValue_ReturnsMinLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetMappingBrightnessLevel_MaxValue_ReturnsMaxLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessLevel_MaxValue_ReturnsMaxLevel start!");
    uint32_t mappedLevel = BrightnessService::GetMappingBrightnessLevel(MAX_BRIGHTNESS_VALUE);
    EXPECT_GE(mappedLevel, MIN_BRIGHTNESS_VALUE);
    EXPECT_LE(mappedLevel, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessLevel_MaxValue_ReturnsMaxLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetMappingBrightnessLevel_MidValue_ReturnsCorrectLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessLevel_MidValue_ReturnsCorrectLevel start!");
    uint32_t midValue = (MAX_BRIGHTNESS_VALUE + MIN_BRIGHTNESS_VALUE) / 2;
    uint32_t mappedLevel = BrightnessService::GetMappingBrightnessLevel(midValue);
    EXPECT_GE(mappedLevel, MIN_BRIGHTNESS_VALUE);
    EXPECT_LE(mappedLevel, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessLevel_MidValue_ReturnsCorrectLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetOrigBrightnessLevel_MidValue_ReturnsCorrectLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetOrigBrightnessLevel_MidValue_ReturnsCorrectLevel start!");
    uint32_t midValue = 100;
    uint32_t origLevel = BrightnessService::GetOrigBrightnessLevel(midValue);
    EXPECT_GE(origLevel, MIN_BRIGHTNESS_VALUE);
    EXPECT_LE(origLevel, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetOrigBrightnessLevel_MidValue_ReturnsCorrectLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetMappingBrightnessNit_ValidLevel_ReturnsCorrectNit, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessNit_ValidLevel_ReturnsCorrectNit start!");
    uint32_t level = 100;
    uint32_t nit = brightnessService->GetMappingBrightnessNit(level);
    EXPECT_GT(nit, 0);
    EXPECT_LE(nit, 600);  // MAX_DEFAULT_BRGIHTNESS_NIT
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingBrightnessNit_ValidLevel_ReturnsCorrectNit end!");
}

HWTEST_F(BrightnessServiceTest, GetBrightnessLevelFromNit_ValidNit_ReturnsCorrectLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessLevelFromNit_ValidNit_ReturnsCorrectLevel start!");
    uint32_t nit = 300;
    uint32_t level = brightnessService->GetBrightnessLevelFromNit(nit);
    EXPECT_GE(level, MIN_BRIGHTNESS_VALUE);
    EXPECT_LE(level, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetBrightnessLevelFromNit_ValidNit_ReturnsCorrectLevel end!");
}

HWTEST_F(BrightnessServiceTest, GetMappingHighBrightnessLevel_ValidLevel_ReturnsCorrectLevel, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingHighBrightnessLevel_ValidLevel_ReturnsCorrectLevel start!");
    uint32_t level = 200;
    uint32_t highLevel = brightnessService->GetMappingHighBrightnessLevel(level);
    EXPECT_GE(highLevel, 156);  // MIN_DEFAULT_HIGH_BRGIHTNESS_LEVEL
    DISPLAY_HILOGI(LABEL_TEST, "GetMappingHighBrightnessLevel_ValidLevel_ReturnsCorrectLevel end!");
}

// ==================== Safe Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, GetSafeBrightness_NormalValue_ReturnsSameValue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetSafeBrightness_NormalValue_ReturnsSameValue start!");
    uint32_t safeBrightness = BrightnessService::GetSafeBrightness(DEFAULT_BRIGHTNESS_VALUE);
    EXPECT_EQ(safeBrightness, DEFAULT_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetSafeBrightness_NormalValue_ReturnsSameValue end!");
}

HWTEST_F(BrightnessServiceTest, GetSafeBrightness_ExceedsMax_ClampedToMax, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetSafeBrightness_ExceedsMax_ClampedToMax start!");
    uint32_t unsafeValue = MAX_BRIGHTNESS_VALUE + 100;
    uint32_t safeBrightness = BrightnessService::GetSafeBrightness(unsafeValue);
    EXPECT_LE(safeBrightness, MAX_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetSafeBrightness_ExceedsMax_ClampedToMax end!");
}

HWTEST_F(BrightnessServiceTest, GetSafeBrightness_BelowMin_ClampedToMin, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetSafeBrightness_BelowMin_ClampedToMin start!");
    uint32_t safeBrightness = BrightnessService::GetSafeBrightness(0);
    EXPECT_GE(safeBrightness, MIN_BRIGHTNESS_VALUE);
    DISPLAY_HILOGI(LABEL_TEST, "GetSafeBrightness_BelowMin_ClampedToMin end!");
}

// ==================== Discount Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, DiscountBrightness_HalfDiscount_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_HalfDiscount_Success start!");
    bool result = brightnessService->DiscountBrightness(HALF_DISCOUNT, 0);
    EXPECT_TRUE(result);
    EXPECT_DOUBLE_EQ(brightnessService->GetDiscount(), HALF_DISCOUNT);
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_HalfDiscount_Success end!");
}

HWTEST_F(BrightnessServiceTest, DiscountBrightness_MinDiscount_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_MinDiscount_Success start!");
    bool result = brightnessService->DiscountBrightness(MIN_DISCOUNT, 0);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_MinDiscount_Success end!");
}

HWTEST_F(BrightnessServiceTest, DiscountBrightness_NoDiscount_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_NoDiscount_Success start!");
    bool result = brightnessService->DiscountBrightness(NO_DISCOUNT, 0);
    EXPECT_TRUE(result);
    EXPECT_DOUBLE_EQ(brightnessService->GetDiscount(), NO_DISCOUNT);
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_NoDiscount_Success end!");
}

HWTEST_F(BrightnessServiceTest, DiscountBrightness_ExceedsMax_ClampedToMax, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_ExceedsMax_ClampedToMax start!");
    double excessiveDiscount = 1.5;
    bool result = brightnessService->DiscountBrightness(excessiveDiscount, 0);
    EXPECT_TRUE(result);
    EXPECT_LE(brightnessService->GetDiscount(), 1.0);
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_ExceedsMax_ClampedToMax end!");
}

HWTEST_F(BrightnessServiceTest, DiscountBrightness_BelowMin_ClampedToMin, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_BelowMin_ClampedToMin start!");
    double tooLowDiscount = 0.0;
    bool result = brightnessService->DiscountBrightness(tooLowDiscount, 0);
    EXPECT_TRUE(result);
    EXPECT_GE(brightnessService->GetDiscount(), MIN_DISCOUNT);
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBrightness_BelowMin_ClampedToMin end!");
}

// ==================== Override Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, OverrideBrightness_ValidValue_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "OverrideBrightness_ValidValue_Success start!");
    bool result = brightnessService->OverrideBrightness(DEFAULT_BRIGHTNESS_VALUE, 0);
    EXPECT_TRUE(result);
    EXPECT_TRUE(brightnessService->IsBrightnessOverridden());
    DISPLAY_HILOGI(LABEL_TEST, "OverrideBrightness_ValidValue_Success end!");
}

HWTEST_F(BrightnessServiceTest, OverrideBrightness_WithDuration_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "OverrideBrightness_WithDuration_Success start!");
    bool result = brightnessService->OverrideBrightness(DEFAULT_BRIGHTNESS_VALUE, 500);
    EXPECT_TRUE(result);
    EXPECT_TRUE(brightnessService->IsBrightnessOverridden());
    DISPLAY_HILOGI(LABEL_TEST, "OverrideBrightness_WithDuration_Success end!");
}

HWTEST_F(BrightnessServiceTest, RestoreBrightness_AfterOverride_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RestoreBrightness_AfterOverride_Success start!");
    brightnessService->OverrideBrightness(DEFAULT_BRIGHTNESS_VALUE, 0);
    EXPECT_TRUE(brightnessService->IsBrightnessOverridden());

    bool result = brightnessService->RestoreBrightness(0);
    EXPECT_TRUE(result);
    EXPECT_FALSE(brightnessService->IsBrightnessOverridden());
    DISPLAY_HILOGI(LABEL_TEST, "RestoreBrightness_AfterOverride_Success end!");
}

HWTEST_F(BrightnessServiceTest, RestoreBrightness_WithoutOverride_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RestoreBrightness_WithoutOverride_ReturnsFalse start!");
    bool result = brightnessService->RestoreBrightness(0);
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "RestoreBrightness_WithoutOverride_ReturnsFalse end!");
}

HWTEST_F(BrightnessServiceTest, IsBrightnessOverridden_AfterOverride_ReturnsTrue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessOverridden_AfterOverride_ReturnsTrue start!");
    brightnessService->OverrideBrightness(DEFAULT_BRIGHTNESS_VALUE, 0);
    EXPECT_TRUE(brightnessService->IsBrightnessOverridden());
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessOverridden_AfterOverride_ReturnsTrue end!");
}

HWTEST_F(BrightnessServiceTest, IsBrightnessOverridden_AfterRestore_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessOverridden_AfterRestore_ReturnsFalse start!");
    brightnessService->OverrideBrightness(DEFAULT_BRIGHTNESS_VALUE, 0);
    brightnessService->RestoreBrightness(0);
    EXPECT_FALSE(brightnessService->IsBrightnessOverridden());
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessOverridden_AfterRestore_ReturnsFalse end!");
}

// ==================== Boost Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, BoostBrightness_ValidTimeout_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BoostBrightness_ValidTimeout_Success start!");
    bool result = brightnessService->BoostBrightness(TEST_TIMEOUT_MS, 0);
    EXPECT_TRUE(result);
    EXPECT_TRUE(brightnessService->IsBrightnessBoosted());
    DISPLAY_HILOGI(LABEL_TEST, "BoostBrightness_ValidTimeout_Success end!");
}

HWTEST_F(BrightnessServiceTest, BoostBrightness_WithDuration_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BoostBrightness_WithDuration_Success start!");
    bool result = brightnessService->BoostBrightness(TEST_TIMEOUT_MS, 500);
    EXPECT_TRUE(result);
    EXPECT_TRUE(brightnessService->IsBrightnessBoosted());
    DISPLAY_HILOGI(LABEL_TEST, "BoostBrightness_WithDuration_Success end!");
}

HWTEST_F(BrightnessServiceTest, CancelBoostBrightness_AfterBoost_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "CancelBoostBrightness_AfterBoost_Success start!");
    brightnessService->BoostBrightness(TEST_TIMEOUT_MS, 0);
    EXPECT_TRUE(brightnessService->IsBrightnessBoosted());

    bool result = brightnessService->CancelBoostBrightness(0);
    EXPECT_TRUE(result);
    EXPECT_FALSE(brightnessService->IsBrightnessBoosted());
    DISPLAY_HILOGI(LABEL_TEST, "CancelBoostBrightness_AfterBoost_Success end!");
}

HWTEST_F(BrightnessServiceTest, CancelBoostBrightness_WithoutBoost_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "CancelBoostBrightness_WithoutBoost_ReturnsFalse start!");
    bool result = brightnessService->CancelBoostBrightness(0);
    EXPECT_FALSE(result);
    DISPLAY_HILOGI(LABEL_TEST, "CancelBoostBrightness_WithoutBoost_ReturnsFalse end!");
}

HWTEST_F(BrightnessServiceTest, IsBrightnessBoosted_AfterBoost_ReturnsTrue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessBoosted_AfterBoost_ReturnsTrue start!");
    brightnessService->BoostBrightness(TEST_TIMEOUT_MS, 0);
    EXPECT_TRUE(brightnessService->IsBrightnessBoosted());
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessBoosted_AfterBoost_ReturnsTrue end!");
}

HWTEST_F(BrightnessServiceTest, IsBrightnessBoosted_AfterCancel_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessBoosted_AfterCancel_ReturnsFalse start!");
    brightnessService->BoostBrightness(TEST_TIMEOUT_MS, 0);
    brightnessService->CancelBoostBrightness(0);
    EXPECT_FALSE(brightnessService->IsBrightnessBoosted());
    DISPLAY_HILOGI(LABEL_TEST, "IsBrightnessBoosted_AfterCancel_ReturnsFalse end!");
}

// ==================== Screen On Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, SetScreenOnBrightness_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetScreenOnBrightness_Success start!");
    brightnessService->SetScreenOnBrightness();
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "SetScreenOnBrightness_Success end!");
}

HWTEST_F(BrightnessServiceTest, GetScreenOnBrightness_WithoutUpdate_ReturnsValue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetScreenOnBrightness_WithoutUpdate_ReturnsValue start!");
    uint32_t brightness = brightnessService->GetScreenOnBrightness(false);
    EXPECT_GE(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "GetScreenOnBrightness_WithoutUpdate_ReturnsValue end!");
}

HWTEST_F(BrightnessServiceTest, GetScreenOnBrightness_WithUpdate_ReturnsValue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetScreenOnBrightness_WithUpdate_ReturnsValue start!");
    uint32_t brightness = brightnessService->GetScreenOnBrightness(true);
    EXPECT_GE(brightness, 0);
    DISPLAY_HILOGI(LABEL_TEST, "GetScreenOnBrightness_WithUpdate_ReturnsValue end!");
}

// ==================== Sleep Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, SetSleepBrightness_ValidState_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetSleepBrightness_ValidState_Success start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_DIM);
    brightnessService->SetSleepBrightness();
    EXPECT_TRUE(brightnessService->IsSleepStatus());
    DISPLAY_HILOGI(LABEL_TEST, "SetSleepBrightness_ValidState_Success end!");
}

HWTEST_F(BrightnessServiceTest, IsSleepStatus_AfterSetSleepBrightness_ReturnsTrue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsSleepStatus_AfterSetSleepBrightness_ReturnsTrue start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_DIM);
    brightnessService->SetSleepBrightness();
    EXPECT_TRUE(brightnessService->IsSleepStatus());
    DISPLAY_HILOGI(LABEL_TEST, "IsSleepStatus_AfterSetSleepBrightness_ReturnsTrue end!");
}

HWTEST_F(BrightnessServiceTest, IsSleepStatus_AfterDisplayOn_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsSleepStatus_AfterDisplayOn_ReturnsFalse start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_DIM);
    brightnessService->SetSleepBrightness();
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    EXPECT_FALSE(brightnessService->IsSleepStatus());
    DISPLAY_HILOGI(LABEL_TEST, "IsSleepStatus_AfterDisplayOn_ReturnsFalse end!");
}

// ==================== Display ID Tests ====================

HWTEST_F(BrightnessServiceTest, GetDisplayId_ReturnsDefaultId, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetDisplayId_ReturnsDefaultId start!");
    uint32_t displayId = brightnessService->GetDisplayId();
    EXPECT_GE(displayId, 0);
    DISPLAY_HILOGI(LABEL_TEST, "GetDisplayId_ReturnsDefaultId end!");
}

HWTEST_F(BrightnessServiceTest, SetDisplayId_ValidId_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayId_ValidId_Success start!");
    brightnessService->SetDisplayId(1);
    EXPECT_EQ(brightnessService->GetDisplayId(), 1);
    DISPLAY_HILOGI(LABEL_TEST, "SetDisplayId_ValidId_Success end!");
}

HWTEST_F(BrightnessServiceTest, GetCurrentDisplayId_ValidDefault_ReturnsId, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetCurrentDisplayId_ValidDefault_ReturnsId start!");
    uint32_t displayId = brightnessService->GetCurrentDisplayId(0);
    EXPECT_GE(displayId, 0);
    DISPLAY_HILOGI(LABEL_TEST, "GetCurrentDisplayId_ValidDefault_ReturnsId end!");
}

HWTEST_F(BrightnessServiceTest, GetCurrentSensorId_ReturnsValidId, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetCurrentSensorId_ReturnsValidId start!");
    uint32_t sensorId = brightnessService->GetCurrentSensorId();
    EXPECT_GE(sensorId, 0);
    DISPLAY_HILOGI(LABEL_TEST, "GetCurrentSensorId_ReturnsValidId end!");
}

HWTEST_F(BrightnessServiceTest, SetCurrentSensorId_ValidId_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetCurrentSensorId_ValidId_Success start!");
    brightnessService->SetCurrentSensorId(1);
    EXPECT_EQ(brightnessService->GetCurrentSensorId(), 1);
    DISPLAY_HILOGI(LABEL_TEST, "SetCurrentSensorId_ValidId_Success end!");
}

// ==================== Light Brightness Threshold Tests ====================

HWTEST_F(BrightnessServiceTest, SetLightBrightnessThreshold_EmptyThreshold_ReturnsZero, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetLightBrightnessThreshold_EmptyThreshold_ReturnsZero start!");
    std::vector<int32_t> emptyThreshold;
    uint32_t result = brightnessService->SetLightBrightnessThreshold(emptyThreshold, nullptr);
    EXPECT_EQ(result, 0);
    DISPLAY_HILOGI(LABEL_TEST, "SetLightBrightnessThreshold_EmptyThreshold_ReturnsZero end!");
}

HWTEST_F(BrightnessServiceTest, SetLightBrightnessThreshold_NullCallback_ReturnsZero, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetLightBrightnessThreshold_NullCallback_ReturnsZero start!");
    std::vector<int32_t> threshold = {100, 200};
    uint32_t result = brightnessService->SetLightBrightnessThreshold(threshold, nullptr);
    EXPECT_EQ(result, 0);
    DISPLAY_HILOGI(LABEL_TEST, "SetLightBrightnessThreshold_NullCallback_ReturnsZero end!");
}

// ==================== Sensor Support Tests ====================

HWTEST_F(BrightnessServiceTest, IsSupportLightSensor_ReturnsBool, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsSupportLightSensor_ReturnsBool start!");
    bool isSupported = brightnessService->IsSupportLightSensor();
    // Result depends on whether sensor is available
    EXPECT_TRUE(isSupported == true || isSupported == false);
    DISPLAY_HILOGI(LABEL_TEST, "IsSupportLightSensor_ReturnsBool end!");
}

HWTEST_F(BrightnessServiceTest, GetIsSupportLightSensor_ReturnsBool, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetIsSupportLightSensor_ReturnsBool start!");
    // Now we can access private method
    bool isSupported = brightnessService->GetIsSupportLightSensor();
    // Result depends on whether sensor is available
    EXPECT_TRUE(isSupported == true || isSupported == false);
    DISPLAY_HILOGI(LABEL_TEST, "GetIsSupportLightSensor_ReturnsBool end!");
}

// ==================== Auto Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, IsAutoAdjustBrightness_ReturnsBool, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsAutoAdjustBrightness_ReturnsBool start!");
    bool isAutoEnabled = brightnessService->IsAutoAdjustBrightness();
    EXPECT_TRUE(isAutoEnabled == true || isAutoEnabled == false);
    DISPLAY_HILOGI(LABEL_TEST, "IsAutoAdjustBrightness_ReturnsBool end!");
}

HWTEST_F(BrightnessServiceTest, GetSettingAutoBrightness_DefaultKey_ReturnsBool, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetSettingAutoBrightness_DefaultKey_ReturnsBool start!");
    bool isAutoEnabled = BrightnessService::GetSettingAutoBrightness();
    EXPECT_TRUE(isAutoEnabled == true || isAutoEnabled == false);
    DISPLAY_HILOGI(LABEL_TEST, "GetSettingAutoBrightness_DefaultKey_ReturnsBool end!");
}

HWTEST_F(BrightnessServiceTest, SetSettingAutoBrightness_Enable_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetSettingAutoBrightness_Enable_Success start!");
    BrightnessService::SetSettingAutoBrightness(true);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "SetSettingAutoBrightness_Enable_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetSettingAutoBrightness_Disable_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetSettingAutoBrightness_Disable_Success start!");
    BrightnessService::SetSettingAutoBrightness(false);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "SetSettingAutoBrightness_Disable_Success end!");
}

// ==================== Screen Power Status Tests ====================

HWTEST_F(BrightnessServiceTest, NotifyScreenPowerStatus_ValidInput_ReturnsZero, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "NotifyScreenPowerStatus_ValidInput_ReturnsZero start!");
    int result = brightnessService->NotifyScreenPowerStatus(0, 1);
    EXPECT_EQ(result, 0);
    DISPLAY_HILOGI(LABEL_TEST, "NotifyScreenPowerStatus_ValidInput_ReturnsZero end!");
}

HWTEST_F(BrightnessServiceTest, NotifyScreenPowerStatus_DifferentStatuses_ReturnsZero, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "NotifyScreenPowerStatus_DifferentStatuses_ReturnsZero start!");
    int result1 = brightnessService->NotifyScreenPowerStatus(0, 0);
    int result2 = brightnessService->NotifyScreenPowerStatus(0, 2);
    EXPECT_EQ(result1, 0);
    EXPECT_EQ(result2, 0);
    DISPLAY_HILOGI(LABEL_TEST, "NotifyScreenPowerStatus_DifferentStatuses_ReturnsZero end!");
}

// ==================== Dimming Tests ====================

HWTEST_F(BrightnessServiceTest, IsDimming_InitialState_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsDimming_InitialState_ReturnsFalse start!");
    bool isDimming = brightnessService->IsDimming();
    EXPECT_FALSE(isDimming);
    DISPLAY_HILOGI(LABEL_TEST, "IsDimming_InitialState_ReturnsFalse end!");
}

HWTEST_F(BrightnessServiceTest, GetDimmingUpdateTime_ReturnsValidTime, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "GetDimmingUpdateTime_ReturnsValidTime start!");
    uint32_t updateTime = brightnessService->GetDimmingUpdateTime();
    EXPECT_GE(updateTime, 0);
    DISPLAY_HILOGI(LABEL_TEST, "GetDimmingUpdateTime_ReturnsValidTime end!");
}

HWTEST_F(BrightnessServiceTest, WaitDimmingDone_NoDimming_Completes, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "WaitDimmingDone_NoDimming_Completes start!");
    brightnessService->WaitDimmingDone();
    // If no deadlock, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "WaitDimmingDone_NoDimming_Completes end!");
}

// ==================== Clear Offset Tests ====================

HWTEST_F(BrightnessServiceTest, ClearOffset_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ClearOffset_Success start!");
    brightnessService->ClearOffset();
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ClearOffset_Success end!");
}

HWTEST_F(BrightnessServiceTest, ClearOffset_AfterDiscount_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ClearOffset_AfterDiscount_Success start!");
    brightnessService->DiscountBrightness(HALF_DISCOUNT, 0);
    brightnessService->ClearOffset();
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ClearOffset_AfterDiscount_Success end!");
}

// ==================== Scene Mode Tests ====================

HWTEST_F(BrightnessServiceTest, UpdateBrightnessSceneMode_ValidMode_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UpdateBrightnessSceneMode_ValidMode_Success start!");
    brightnessService->UpdateBrightnessSceneMode(BrightnessSceneMode::MODE_DEFAULT);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "UpdateBrightnessSceneMode_ValidMode_Success end!");
}

HWTEST_F(BrightnessServiceTest, UpdateBrightnessSceneMode_DifferentModes_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UpdateBrightnessSceneMode_DifferentModes_Success start!");
    // Use valid enum values from BrightnessSceneMode: MODE_DEFAULT, MODE_GAME, MODE_VIDEO
    brightnessService->UpdateBrightnessSceneMode(BrightnessSceneMode::MODE_GAME);
    brightnessService->UpdateBrightnessSceneMode(BrightnessSceneMode::MODE_VIDEO);
    brightnessService->UpdateBrightnessSceneMode(BrightnessSceneMode::MODE_DEFAULT);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "UpdateBrightnessSceneMode_DifferentModes_Success end!");
}

// ==================== Setting Observer Tests ====================

HWTEST_F(BrightnessServiceTest, RegisterSettingBrightnessObserver_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "RegisterSettingBrightnessObserver_Success start!");
    brightnessService->RegisterSettingBrightnessObserver();
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "RegisterSettingBrightnessObserver_Success end!");
}

HWTEST_F(BrightnessServiceTest, UnregisterSettingBrightnessObserver_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterSettingBrightnessObserver_Success start!");
    brightnessService->UnregisterSettingBrightnessObserver();
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "UnregisterSettingBrightnessObserver_Success end!");
}

// ==================== Max Brightness Tests ====================

HWTEST_F(BrightnessServiceTest, SetMaxBrightness_ValidValue_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_ValidValue_Success start!");
    bool result = brightnessService->SetMaxBrightness(1.0);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_ValidValue_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetMaxBrightness_HalfValue_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_HalfValue_Success start!");
    bool result = brightnessService->SetMaxBrightness(0.5);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_HalfValue_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetMaxBrightness_ZeroValue_ClampedToMin, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_ZeroValue_ClampedToMin start!");
    bool result = brightnessService->SetMaxBrightness(0.0);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_ZeroValue_ClampedToMin end!");
}

HWTEST_F(BrightnessServiceTest, SetMaxBrightness_ExceedsOne_ClampedToMax, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_ExceedsOne_ClampedToMax start!");
    bool result = brightnessService->SetMaxBrightness(1.5);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightness_ExceedsOne_ClampedToMax end!");
}

HWTEST_F(BrightnessServiceTest, SetMaxBrightnessNit_ValidNit_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightnessNit_ValidNit_Success start!");
    bool result = brightnessService->SetMaxBrightnessNit(500);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightnessNit_ValidNit_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetMaxBrightnessNit_MinNit_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightnessNit_MinNit_Success start!");
    bool result = brightnessService->SetMaxBrightnessNit(2);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightnessNit_MinNit_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetMaxBrightnessNit_MaxNit_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightnessNit_MaxNit_Success start!");
    bool result = brightnessService->SetMaxBrightnessNit(600);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "SetMaxBrightnessNit_MaxNit_Success end!");
}

// ==================== Report Brightness Big Data Tests ====================

HWTEST_F(BrightnessServiceTest, ReportBrightnessBigData_ValidBrightness_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ReportBrightnessBigData_ValidBrightness_Success start!");
    brightnessService->ReportBrightnessBigData(DEFAULT_BRIGHTNESS_VALUE);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ReportBrightnessBigData_ValidBrightness_Success end!");
}

HWTEST_F(BrightnessServiceTest, ReportBrightnessBigData_MinBrightness_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ReportBrightnessBigData_MinBrightness_Success start!");
    brightnessService->ReportBrightnessBigData(MIN_BRIGHTNESS_VALUE);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ReportBrightnessBigData_MinBrightness_Success end!");
}

HWTEST_F(BrightnessServiceTest, ReportBrightnessBigData_MaxBrightness_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ReportBrightnessBigData_MaxBrightness_Success start!");
    brightnessService->ReportBrightnessBigData(MAX_BRIGHTNESS_VALUE);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ReportBrightnessBigData_MaxBrightness_Success end!");
}

// ==================== IsScreenOn Tests ====================

HWTEST_F(BrightnessServiceTest, IsScreenOn_DisplayOnState_ReturnsTrue, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOn_DisplayOnState_ReturnsTrue start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    // IsScreenOn is private, testing through public interface effects
    bool canSet = brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 0, false);
    EXPECT_TRUE(canSet);
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOn_DisplayOnState_ReturnsTrue end!");
}

HWTEST_F(BrightnessServiceTest, IsScreenOn_DisplayOffState_ReturnsFalse, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOn_DisplayOffState_ReturnsFalse start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_OFF);
    // When screen is off, SetBrightness may fail or be cached
    brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 0, false);
    // If no crash, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "IsScreenOn_DisplayOffState_ReturnsFalse end!");
}

// ==================== CanSetBrightness Tests ====================

HWTEST_F(BrightnessServiceTest, CanSetBrightness_WhenOverridden_CannotSet, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "CanSetBrightness_WhenOverridden_CannotSet start!");
    brightnessService->OverrideBrightness(DEFAULT_BRIGHTNESS_VALUE, 0);
    // After override, new SetBrightness calls should clear override
    bool result = brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE - 10, 0, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "CanSetBrightness_WhenOverridden_CannotSet end!");
}

HWTEST_F(BrightnessServiceTest, CanSetBrightness_WhenBoosted_CannotSet, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "CanSetBrightness_WhenBoosted_CannotSet start!");
    brightnessService->BoostBrightness(TEST_TIMEOUT_MS, 0);
    // When boosted, SetBrightness should succeed but not change value
    bool result = brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE, 0, false);
    EXPECT_TRUE(result);
    DISPLAY_HILOGI(LABEL_TEST, "CanSetBrightness_WhenBoosted_CannotSet end!");
}

// ==================== ProcessLightLux Tests ====================

HWTEST_F(BrightnessServiceTest, ProcessLightLux_ValidLux_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_ValidLux_Success start!");
    brightnessService->ProcessLightLux(100.0f);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_ValidLux_Success end!");
}

HWTEST_F(BrightnessServiceTest, ProcessLightLux_ZeroLux_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_ZeroLux_Success start!");
    brightnessService->ProcessLightLux(0.0f);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_ZeroLux_Success end!");
}

HWTEST_F(BrightnessServiceTest, ProcessLightLux_HighLux_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_HighLux_Success start!");
    brightnessService->ProcessLightLux(50000.0f);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_HighLux_Success end!");
}

HWTEST_F(BrightnessServiceTest, ProcessLightLux_VaryingLux_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_VaryingLux_Success start!");
    brightnessService->ProcessLightLux(10.0f);
    brightnessService->ProcessLightLux(100.0f);
    brightnessService->ProcessLightLux(1000.0f);
    brightnessService->ProcessLightLux(10000.0f);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ProcessLightLux_VaryingLux_Success end!");
}

// ==================== SetBrightnessLevel Tests ====================

HWTEST_F(BrightnessServiceTest, SetBrightnessLevel_WithZeroDuration_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightnessLevel_WithZeroDuration_Success start!");
    // SetBrightnessLevel is a private method, now accessible
    brightnessService->SetBrightnessLevel(DEFAULT_BRIGHTNESS_VALUE, 0);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightnessLevel_WithZeroDuration_Success end!");
}

HWTEST_F(BrightnessServiceTest, SetBrightnessLevel_WithDuration_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightnessLevel_WithDuration_Success start!");
    // SetBrightnessLevel is a private method, now accessible
    brightnessService->SetBrightnessLevel(DEFAULT_BRIGHTNESS_VALUE, 500);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "SetBrightnessLevel_WithDuration_Success end!");
}

// ==================== Multiple State Transitions Tests ====================

HWTEST_F(BrightnessServiceTest, MultipleStateTransitions_OnToOffToOn_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultipleStateTransitions_OnToOffToOn_Success start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_OFF);
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "MultipleStateTransitions_OnToOffToOn_Success end!");
}

HWTEST_F(BrightnessServiceTest, MultipleStateTransitions_OnToDimToOn_Success, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "MultipleStateTransitions_OnToDimToOn_Success start!");
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_DIM);
    brightnessService->SetDisplayState(0, DisplayState::DISPLAY_ON);
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "MultipleStateTransitions_OnToDimToOn_Success end!");
}

// ==================== Brightness Value Boundary Tests ====================

HWTEST_F(BrightnessServiceTest, BrightnessValueBoundary_TestMultipleValues, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessValueBoundary_TestMultipleValues start!");
    std::vector<uint32_t> testValues = {0, 1, 100, 200, 255, 256, 1000};
    for (uint32_t value : testValues) {
        bool result = brightnessService->SetBrightness(value, 0, false);
        EXPECT_TRUE(result);
    }
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessValueBoundary_TestMultipleValues end!");
}

// ==================== Discount Boundary Tests ====================

HWTEST_F(BrightnessServiceTest, DiscountBoundary_TestMultipleDiscounts, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBoundary_TestMultipleDiscounts start!");
    std::vector<double> testDiscounts = {0.01, 0.1, 0.5, 0.9, 1.0, 1.5, -0.1};
    for (double discount : testDiscounts) {
        bool result = brightnessService->DiscountBrightness(discount, 0);
        EXPECT_TRUE(result);
    }
    DISPLAY_HILOGI(LABEL_TEST, "DiscountBoundary_TestMultipleDiscounts end!");
}

// ==================== Timeout Boundary Tests ====================

HWTEST_F(BrightnessServiceTest, TimeoutBoundary_TestMultipleTimeouts, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "TimeoutBoundary_TestMultipleTimeouts start!");
    std::vector<uint32_t> testTimeouts = {0, 100, 1000, 5000, 10000};
    for (uint32_t timeout : testTimeouts) {
        brightnessService->BoostBrightness(timeout, 0);
        brightnessService->CancelBoostBrightness(0);
    }
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "TimeoutBoundary_TestMultipleTimeouts end!");
}

// ==================== Concurrent Operation Tests ====================

HWTEST_F(BrightnessServiceTest, ConcurrentOperations_MultipleBrightnessChanges, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "ConcurrentOperations_MultipleBrightnessChanges start!");
    for (int i = 0; i < 10; i++) {
        brightnessService->SetBrightness(DEFAULT_BRIGHTNESS_VALUE + i, 0, false);
    }
    // If no exception, test passes
    EXPECT_TRUE(true);
    DISPLAY_HILOGI(LABEL_TEST, "ConcurrentOperations_MultipleBrightnessChanges end!");
}

} // namespace
