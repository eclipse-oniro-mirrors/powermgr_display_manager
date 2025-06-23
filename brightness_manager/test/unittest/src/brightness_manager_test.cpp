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
#include "display_log.h"
#include "display_power_mgr_client.h"
#include "brightness_manager.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayPowerMgr;
using namespace std;

namespace {
    const double NO_DISCOUNT = 1.00;
}

class BrightnessManagerTest : public Test {
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
HWTEST_F(BrightnessManagerTest, BrightnessManagerGet001, TestSize.Level0)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerGet001 function start!");
    auto& brightnessManager = BrightnessManager::Get();
    EXPECT_NE(&brightnessManager, nullptr);
    const int sleepTime = 100000;
    usleep(sleepTime);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerGet001 function end!");
}

HWTEST_F(BrightnessManagerTest, BrightnessManagerGetDiscount001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerGetDiscount001 function start!");
    EXPECT_NE(BrightnessManager::Get().GetDiscount(), 0);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessManagerGetDiscount001 function end!");
}
} // namespace
