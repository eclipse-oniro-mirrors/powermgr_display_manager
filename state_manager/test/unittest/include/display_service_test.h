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

#ifndef DISPLAY_SERVICE_TEST_H
#define DISPLAY_SERVICE_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "display_power_mgr_proxy.h"
#include "display_power_callback_stub.h"
#include "brightness_service.h"

namespace OHOS {
namespace PowerMgr {
class DisplayServiceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void DisplayServiceInnerTestFunc();
    class DisplayPowerMgrTestCallback : public OHOS::DisplayPowerMgr::DisplayPowerCallbackStub {
    public:
        DisplayPowerMgrTestCallback() {}
        virtual ~DisplayPowerMgrTestCallback() {}
        virtual void OnDisplayStateChanged(
            uint32_t displayId, OHOS::DisplayPowerMgr::DisplayState state, uint32_t reason) override;
    };

    class BrightnessServiceMock : public OHOS::DisplayPowerMgr::BrightnessService {
    public:
        BrightnessServiceMock() {}
        ~BrightnessServiceMock() = default;
        MOCK_METHOD0(SetScreenOnBrightness, void());
    };
};
} // namespace PowerMgr
} // namespace OHOS
#endif // DISPLAY_SERVICE_TEST_H
