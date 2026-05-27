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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gtest/gtest-death-test.h>

#ifdef DISPLAYMGR_GTEST
#define private   public
#define protected public
#endif

#include "display_log.h"
#include "display_power_mgr_client.h"

// Make private members accessible for testing
#define private public
#include "brightness_service.h"
#undef private
#include "light_lux_manager.h"

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

class BrightnessLightLuxBufferTest : public Test {
public:
    void SetUp()
    {
        DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest SetUp");
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
        DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest TearDown");
        DisplayPowerMgrClient::GetInstance().RestoreBrightness();
        DisplayPowerMgrClient::GetInstance().CancelBoostBrightness();

        // Reset state
        if (brightnessService != nullptr) {
            brightnessService->ClearOffset();
        }
    }

    static void TearDownTestCase()
    {
        DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest TearDownTestCase");
        BrightnessService::Get().DeInit(); // for ffrt queue destruct
        // After DeInit, service should still be accessible but with cleaned state
        EXPECT_NE(&BrightnessService::Get(), nullptr);
    }

protected:
    BrightnessService* brightnessService;
};

namespace {
/**
 * @tc.name: BrightnessLightLuxBufferTest001
 * @tc.desc: test LightLuxBuffer LightLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest001 function start!");
    LightLuxBuffer lightLuxManager{0};
    EXPECT_EQ(lightLuxManager.mCapacity, 32);
    LightLuxBuffer lightLuxManager1{512};
    EXPECT_EQ(lightLuxManager1.mCapacity, 32);
    LightLuxBuffer lightLuxManager2{32};
    EXPECT_EQ(lightLuxManager2.mCapacity, 32);
    LightLuxBuffer lightLuxManager3{523};
    EXPECT_EQ(lightLuxManager3.mCapacity, 32);
    LightLuxBuffer lightLuxManager4{-1};
    EXPECT_EQ(lightLuxManager4.mCapacity, 32);
    LightLuxBuffer lightLuxManager5{23};
    EXPECT_EQ(lightLuxManager5.mCapacity, 23);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest001 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest002
 * @tc.desc: test LightLuxBuffer ~LightLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest002, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest002 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest002 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest003
 * @tc.desc: test LightLuxBuffer ~LightLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest003, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest003 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    delete[] lightLuxManager.mBufferData;
    lightLuxManager.mBufferData = nullptr;
    EXPECT_EQ(lightLuxManager.mBufferData, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest003 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest004
 * @tc.desc: test LightLuxBuffer ~LightLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest004, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest004 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest004 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest005
 * @tc.desc: test LightLuxBuffer ~LightLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest005, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest005 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    delete[] lightLuxManager.mBufferTime;
    lightLuxManager.mBufferTime = nullptr;
    EXPECT_EQ(lightLuxManager.mBufferTime, nullptr);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest005 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest006
 * @tc.desc: test LightLuxBuffer LuxBufferCheck
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest006, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest006 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    
    int ret = lightLuxManager.LuxBufferCheck();
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest006 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest007
 * @tc.desc: test LightLuxBuffer LuxBufferCheck
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest007, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest007 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    delete[] lightLuxManager.mBufferData;
    lightLuxManager.mBufferData = nullptr;
    int ret = lightLuxManager.LuxBufferCheck();
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest007 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest008
 * @tc.desc: test LightLuxBuffer LuxBufferCheck
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest008, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest008 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    delete[] lightLuxManager.mBufferData;
    lightLuxManager.mBufferData = nullptr;

    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);
    delete[] lightLuxManager.mBufferTime;
    lightLuxManager.mBufferTime = nullptr;
    int ret = lightLuxManager.LuxBufferCheck();
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest008 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest009
 * @tc.desc: test LightLuxBuffer LuxBufferCheck
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest009, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest009 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);

    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);
    delete[] lightLuxManager.mBufferTime;
    lightLuxManager.mBufferTime = nullptr;
    int ret = lightLuxManager.LuxBufferCheck();
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest009 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest011
 * @tc.desc: test LightLuxBuffer CopyLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest011, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest011 function start!");
    LightLuxBuffer lightLuxManager{0};
    int newSize = -1;
    int ret = lightLuxManager.CopyLuxBuffer(newSize);
    EXPECT_EQ(ret, -1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest011 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest012
 * @tc.desc: test LightLuxBuffer CopyLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest012, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest012 function start!");
    LightLuxBuffer lightLuxManager{0};

    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    lightLuxManager.mBufferData[0] = 12;

    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);
    lightLuxManager.mBufferTime[0] = 12;

    lightLuxManager.mStart = 2;
    int newSize = 64;
    int ret = lightLuxManager.CopyLuxBuffer(newSize);
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest012 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest013
 * @tc.desc: test LightLuxBuffer CopyLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest013, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest013 function start!");
    LightLuxBuffer lightLuxManager{0};

    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    lightLuxManager.mBufferData[0] = 12;

    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);
    lightLuxManager.mBufferTime[0] = 12;

    lightLuxManager.mStart = 0;
    int newSize = 64;
    int ret = lightLuxManager.CopyLuxBuffer(newSize);
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest013 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest014
 * @tc.desc: test LightLuxBuffer CopyLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest014, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest014 function start!");
    LightLuxBuffer lightLuxManager{0};

    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);

    lightLuxManager.mStart = 0;
    int newSize = 23;
    int ret = lightLuxManager.CopyLuxBuffer(newSize);
    EXPECT_EQ(ret, -1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest014 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest015
 * @tc.desc: test LightLuxBuffer CopyLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest015, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest015 function start!");
    LightLuxBuffer lightLuxManager{0};

    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);

    lightLuxManager.mStart = 12;
    int newSize = 10;
    int ret = lightLuxManager.CopyLuxBuffer(newSize);
    EXPECT_EQ(ret, -1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest015 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest016
 * @tc.desc: test LightLuxBuffer CopyLuxBuffer
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest016, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest016 function start!");
    LightLuxBuffer lightLuxManager{0};

    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    EXPECT_NE(lightLuxManager.mBufferData, nullptr);
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    EXPECT_NE(lightLuxManager.mBufferTime, nullptr);

    lightLuxManager.mStart = 12;
    int newSize = 23;
    int ret = lightLuxManager.CopyLuxBuffer(newSize);
    EXPECT_EQ(ret, -1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest016 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest017
 * @tc.desc: test LightLuxBuffer Push
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest017, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest017 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    delete[] lightLuxManager.mBufferData;
    lightLuxManager.mBufferData = nullptr;
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];

    lightLuxManager.Push(123, 233);
    EXPECT_EQ(capacity, 32);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest017 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest018
 * @tc.desc: test LightLuxBuffer Push
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest018, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest018 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mEnd = 0;
    lightLuxManager.mCount = 0;
    lightLuxManager.Push(123, 233);
    EXPECT_EQ(lightLuxManager.mBufferTime[0], 123);
    EXPECT_EQ(lightLuxManager.mBufferData[0], 233);
    EXPECT_EQ(lightLuxManager.mEnd, 1);
    EXPECT_EQ(lightLuxManager.mCount, 1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest018 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest019
 * @tc.desc: test LightLuxBuffer Push
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest019, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest019 function start!");
    LightLuxBuffer lightLuxManager{0};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mEnd = 0;
    lightLuxManager.mCount = 0;
    for (int i = 0; i < capacity; i++) {
        lightLuxManager.Push(123, 233);
    }
    lightLuxManager.Push(123, 233);
    EXPECT_EQ(lightLuxManager.mBufferTime[32], 123);
    EXPECT_EQ(lightLuxManager.mBufferData[32], 233);
    EXPECT_EQ(lightLuxManager.mEnd, 33);
    EXPECT_EQ(lightLuxManager.mStart, 0);
    EXPECT_EQ(lightLuxManager.mCount, 33);
    EXPECT_EQ(lightLuxManager.mCapacity, 64);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest019 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest020
 * @tc.desc: test LightLuxBuffer Push
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest020, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest020 function start!");
    LightLuxBuffer lightLuxManager{392};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mEnd = 0;
    lightLuxManager.mCount = 0;
    for (int i = 0; i < capacity; i++) {
        lightLuxManager.Push(123, 233);
    }
    lightLuxManager.Push(123, 233);
    EXPECT_EQ(lightLuxManager.mBufferTime[391], 123);
    EXPECT_EQ(lightLuxManager.mBufferData[391], 233);
    EXPECT_EQ(lightLuxManager.mEnd, 0);
    EXPECT_EQ(lightLuxManager.mStart, 0);
    EXPECT_EQ(lightLuxManager.mCount, 392);
    EXPECT_EQ(lightLuxManager.mCapacity, 392);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest020 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest021
 * @tc.desc: test LightLuxBuffer Push
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest021, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest021 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mBufferData[31] = 2;
    lightLuxManager.mBufferTime[31] = 12;
    lightLuxManager.mEnd = capacity;
    lightLuxManager.mStart = capacity;
    lightLuxManager.mCount = capacity;

    lightLuxManager.Push(123, 233);
    EXPECT_EQ(lightLuxManager.mBufferData[31], 2);
    EXPECT_EQ(lightLuxManager.mBufferTime[31], 12);
    EXPECT_EQ(lightLuxManager.mEnd, 32);
    EXPECT_EQ(lightLuxManager.mStart, 32);
    EXPECT_EQ(lightLuxManager.mCount, 32);
    EXPECT_EQ(lightLuxManager.mCapacity, 32);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest021 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest022
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest022, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest022 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mCount = 0;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mCount, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest022 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest023
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest023, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest023 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    if (lightLuxManager.mBufferTime != nullptr) {
        delete[] lightLuxManager.mBufferTime;
        lightLuxManager.mBufferTime = nullptr;
    }
    lightLuxManager.mCount = 0;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mCount, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest023 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest024
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest024, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest024 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    if (lightLuxManager.mBufferTime != nullptr) {
        delete[] lightLuxManager.mBufferTime;
        lightLuxManager.mBufferTime = nullptr;
    }
    lightLuxManager.mCount = 2;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mCount, 2);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest024 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest025
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest025, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest025 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mBufferTime[1] = 23;

    lightLuxManager.mCount = 2;
    lightLuxManager.mStart = 0;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mBufferTime[1], 100);
    EXPECT_EQ(lightLuxManager.mStart, 1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest025 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest026
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest026, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest026 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mBufferTime[1] = 123;

    lightLuxManager.mCount = 2;
    lightLuxManager.mStart = 0;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mBufferTime[1], 123);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest026 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest027
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest027, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest027 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mBufferTime[1] = 23;

    lightLuxManager.mCount = 2;
    lightLuxManager.mStart = 0;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mBufferTime[1], 100);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest027 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest028
 * @tc.desc: test LightLuxBuffer Prune
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest028, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest028 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mBufferTime[31] = 23;
    lightLuxManager.mBufferTime[0] = 12;

    lightLuxManager.mCount = 3;
    lightLuxManager.mStart = 30;

    lightLuxManager.Prune(100);
    EXPECT_EQ(lightLuxManager.mStart, 0);
    EXPECT_EQ(lightLuxManager.mBufferTime[31], 23);
    EXPECT_EQ(lightLuxManager.mBufferTime[0], 100);
    EXPECT_EQ(lightLuxManager.mCount, 1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest028 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest029
 * @tc.desc: test LightLuxBuffer Clear
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest029, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest029 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];

    lightLuxManager.mCount = 3;
    lightLuxManager.mStart = 1;
    lightLuxManager.mEnd = 31;

    lightLuxManager.Clear();
    EXPECT_EQ(lightLuxManager.mStart, 0);
    EXPECT_EQ(lightLuxManager.mEnd, 0);
    EXPECT_EQ(lightLuxManager.mCount, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest029 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest030
 * @tc.desc: test LightLuxBuffer GetData
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest030, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest030 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    if (lightLuxManager.mBufferData != nullptr) {
        delete[] lightLuxManager.mBufferData;
        lightLuxManager.mBufferData = nullptr;
    }

    float ret = lightLuxManager.GetData(1);
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest030 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest031
 * @tc.desc: test LightLuxBuffer GetData
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest031, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest031 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mCount = 23;

    float ret = lightLuxManager.GetData(23);
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest031 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest032
 * @tc.desc: test LightLuxBuffer GetData
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest032, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest032 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mCount = 23;

    float ret = lightLuxManager.GetData(24);
    EXPECT_EQ(ret, 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest032 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest033
 * @tc.desc: test LightLuxBuffer GetData
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest033, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest033 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mCount = 23;
    lightLuxManager.mStart = 2;
    lightLuxManager.mBufferData[21] = 100;

    float ret = lightLuxManager.GetData(19);
    EXPECT_EQ(ret, 100);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest033 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest034
 * @tc.desc: test LightLuxBuffer GetSize
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest034, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest034 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mCount = 23;

    float ret = lightLuxManager.GetSize();
    EXPECT_EQ(ret, 23);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest034 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest035
 * @tc.desc: test LightLuxBuffer OffsetOf
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest035, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest035 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mStart = 10;

    unsigned int ret = lightLuxManager.OffsetOf(23);
    EXPECT_EQ(ret, 1);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest035 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest036
 * @tc.desc: test LightLuxBuffer OffsetOf
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest036, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest036 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mStart = 1;

    unsigned int ret = lightLuxManager.OffsetOf(23);
    EXPECT_EQ(ret, 24);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest036 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest037
 * @tc.desc: test LightLuxBuffer ToString
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest037, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest037 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mCount = 9;

    for (int i = 0; i < capacity; i++) {
        lightLuxManager.mBufferData[i] = 123;
        lightLuxManager.mBufferTime[i] = 233;
    }

    std::string expectedCode = "[123/233, 123/233, 123/233, 123/233, 123/233, 123/233, 123/233, 123/233, 123/233, ]";
    std::string ret = lightLuxManager.ToString(24);
    EXPECT_EQ(ret, expectedCode);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest037 function end!");
}

/**
 * @tc.name: BrightnessLightLuxBufferTest038
 * @tc.desc: test LightLuxBuffer ToString
 * @tc.type: FUNC
 */
HWTEST_F(BrightnessLightLuxBufferTest, BrightnessLightLuxBufferTest038, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest038 function start!");
    LightLuxBuffer lightLuxManager{32};
    unsigned int capacity = lightLuxManager.mCapacity;
    lightLuxManager.mBufferData = new(std::nothrow) float[capacity];
    lightLuxManager.mBufferTime = new(std::nothrow) int64_t[capacity];
    lightLuxManager.mCount = 9;

    for (int i = 0; i < capacity; i++) {
        lightLuxManager.mBufferData[i] = 123;
        lightLuxManager.mBufferTime[i] = 233;
    }

    std::string expectedCode = "[123/233, 123/233, 123/233, 123/233, 123/233, 123/233, 123/233, 123/233, 123/233, ]";
    std::string ret = lightLuxManager.ToString(9);
    EXPECT_EQ(ret, expectedCode);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessLightLuxBufferTest038 function end!");
}
} // namespace
