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

#include <gtest/gtest.h>

#include "ohos.brightness.proj.hpp"
#include "ohos.brightness.impl.hpp"
#include "ohos.brightness.user.hpp"
#include "taihe/runtime.hpp"
#include "display_power_mgr_client.h"
#include "display_log.h"
#include "display_mgr_errors.h"

using namespace taihe;
using namespace ohos::brightness;
using namespace OHOS::DisplayPowerMgr;

using namespace testing;
using namespace testing::ext;

namespace {
DisplayErrors g_error = DisplayErrors::ERR_OK;
bool g_pass = false;
bool g_ret = false;
}

namespace taihe {
void set_business_error(int32_t err_code, taihe::string_view msg)
{
    (void)err_code;
    (void)msg;
}
}

namespace OHOS::DisplayPowerMgr {
bool DisplayPowerMgrClient::SetBrightness(uint32_t value, uint32_t displayId, bool continuous)
{
    g_pass = true;
    return g_ret;
}

DisplayErrors DisplayPowerMgrClient::GetError()
{
    return g_error;
}
} // OHOS::PowerMgr

namespace {
class BrightnessTaiheNativeTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown()
    {
        g_pass = false;
        g_ret = false;
    }
};

/**
 * @tc.name: BrightnessTaiheNativeTest_001
 * @tc.desc: test brightness taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(BrightnessTaiheNativeTest, BrightnessTaiheNativeTest_001, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessTaiheNativeTest_001 start");
    constexpr int32_t value = 255;
    SetValueInt(value);
    EXPECT_TRUE(g_pass);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessTaiheNativeTest_001 end");
}

/**
 * @tc.name: BrightnessTaiheNativeTest_002
 * @tc.desc: test brightness taihe native
 * @tc.type: FUNC
 * @tc.require: issue#ICAK9Z
 */
HWTEST_F(BrightnessTaiheNativeTest, BrightnessTaiheNativeTest_002, TestSize.Level1)
{
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessTaiheNativeTest_002 start");
    constexpr int32_t value = 255;
    constexpr bool continuous = true;
    SetValueContinuous(value, continuous);

    g_error = DisplayErrors::ERR_SYSTEM_API_DENIED;
    SetValueContinuous(value, continuous);

    g_ret = true;
    SetValueContinuous(value, continuous);
    EXPECT_TRUE(g_pass);
    DISPLAY_HILOGI(LABEL_TEST, "BrightnessTaiheNativeTest_002 end");
}
}
