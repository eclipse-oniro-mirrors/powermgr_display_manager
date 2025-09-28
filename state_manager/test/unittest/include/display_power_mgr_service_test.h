/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DISPLAYMGR_DISPLAY_MGR_SERVICE_TEST_H
#define DISPLAYMGR_DISPLAY_MGR_SERVICE_TEST_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "display_power_callback_stub.h"
#include "screen_action.h"

class DisplayPowerMgrServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp();
    void TearDown();
    class DisplayPowerMgrTestCallback : public OHOS::DisplayPowerMgr::DisplayPowerCallbackStub {
    public:
        DisplayPowerMgrTestCallback() {};
        virtual ~DisplayPowerMgrTestCallback() {};
        virtual void OnDisplayStateChanged(
            uint32_t displayId, OHOS::DisplayPowerMgr::DisplayState state, uint32_t reason) override;
    };
    class ScreenActionMock : public OHOS::DisplayPowerMgr::ScreenAction {
    public:
        ScreenActionMock(uint32_t displayId) : OHOS::DisplayPowerMgr::ScreenAction(displayId) {}
        ~ScreenActionMock() = default;
        MOCK_METHOD0(GetDefaultDisplayId, uint32_t());
        MOCK_METHOD0(GetDisplayState, OHOS::DisplayPowerMgr::DisplayState());
        MOCK_METHOD2(SetDisplayState, bool(OHOS::DisplayPowerMgr::DisplayState,
            const std::function<void(OHOS::DisplayPowerMgr::DisplayState)>&));
    };
};
#endif // DISPLAYMGR_DISPLAY_MGR_SERVICE_TEST_H
