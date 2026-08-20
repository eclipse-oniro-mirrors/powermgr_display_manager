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
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
#include "multi_screen_display_state_callback_stub.h"
#endif
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
#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
    // When isProxy=true, Register accepts it as proxy for full IPC loopback
    class TestMultiScreenCallback : public OHOS::DisplayPowerMgr::MultiScreenDisplayStateCallbackStub {
    public:
        explicit TestMultiScreenCallback(bool isProxy = false) : isProxy_(isProxy) {}
        ~TestMultiScreenCallback() override = default;
        bool IsProxyObject() const override { return isProxy_; }
        void OnMultiScreenDisplayStateChanged(
            uint64_t screenId, const std::string& screenName,
            OHOS::DisplayPowerMgr::DisplayState state,
            OHOS::DisplayPowerMgr::MultiScreenStateChangeReason reason) override;

        uint64_t lastScreenId_ {0};
        std::string lastScreenName_;
        OHOS::DisplayPowerMgr::DisplayState lastState_ {OHOS::DisplayPowerMgr::DisplayState::DISPLAY_UNKNOWN};
        uint32_t lastReason_ {0};
        int callCount_ {0};
    private:
        bool isProxy_ {false};
    };
#endif

#ifdef ENABLE_SCREEN_POWER_OFF_STRATEGY
class MockRemoteObject : public IRemoteObject {
public:
    MockRemoteObject() : IRemoteObject(u"") {};
    virtual ~MockRemoteObject() {};
    bool IsProxyObject() const
    {
        return isProxyObject_;
    };

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int Dump(int fd, const std::vector<std::u16string> &args)
    {
        (void) args;
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        return 0;
    }

    bool AddDeathRecipient(const sptr <DeathRecipient> &recipient)
    {
        (void) recipient;
        return false;
    }

    bool RemoveDeathRecipient(const sptr <DeathRecipient> &recipient)
    {
        (void) recipient;
        return false;
    }

    bool isProxyObject_ = false;
};
#endif
} // namespace PowerMgr
} // namespace OHOS
#endif // DISPLAY_SERVICE_TEST_H
