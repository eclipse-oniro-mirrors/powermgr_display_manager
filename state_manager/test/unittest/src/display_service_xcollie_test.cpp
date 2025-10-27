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
#include "display_service_xcollie_test.h"

#include "display_log.h"
#include "display_xcollie.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;

void DisplayPowerMgrXcollieTest::SetUp()
{
}

void DisplayPowerMgrXcollieTest::TearDown()
{
}

namespace {
/**
 * @tc.name: DisplayXCollie001
 * @tc.desc: Test functions DisplayXCollie default
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrXcollieTest, DisplayXCollie001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie001 function start!");
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDisplayState");
    EXPECT_FALSE(displayXCollie.isCanceled_.load());
    displayXCollie.CancelDisplayXCollie();
    EXPECT_TRUE(displayXCollie.isCanceled_.load());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie001 function end!");
}

/**
 * @tc.name: DisplayXCollie002
 * @tc.desc: Test functions DisplayXCollie isRecovery = true
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrXcollieTest, DisplayXCollie002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie002 function start!");
    DisplayXCollie displayXCollie("DisplayPowerMgrService::GetDisplayState", true);
    EXPECT_FALSE(displayXCollie.isCanceled_.load());
    displayXCollie.CancelDisplayXCollie();
    EXPECT_TRUE(displayXCollie.isCanceled_.load());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie002 function end!");
}

/**
 * @tc.name: DisplayXCollie003
 * @tc.desc: Test functions DisplayXCollie empty logtag
 * @tc.type: FUNC
 */
HWTEST_F(DisplayPowerMgrXcollieTest, DisplayXCollie003, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie003 function start!");
    std::string emptyString;
    DisplayXCollie displayXCollie(emptyString, true);
    EXPECT_FALSE(displayXCollie.isCanceled_.load());
    displayXCollie.CancelDisplayXCollie();
    EXPECT_TRUE(displayXCollie.isCanceled_.load());
    DISPLAY_HILOGI(LABEL_TEST, "DisplayXCollie003 function end!");
}
} // namespace
