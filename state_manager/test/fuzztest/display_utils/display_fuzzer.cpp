/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "display_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "message_parcel.h"
#include "securec.h"
#include "iremote_object.h"
#include "idisplay_power_mgr.h"
#include "display_brightness_callback_stub.h"

using namespace OHOS::DisplayPowerMgr;
using namespace OHOS::PowerMgr;
using namespace OHOS;

namespace {
const int32_t REWIND_READ_DATA = 0;
const int32_t SLEEP_TIME = 5;
} // namespace

DisplayFuzzerTest::DisplayFuzzerTest()
{
    service_ = DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    service_->Init();
}

DisplayFuzzerTest::~DisplayFuzzerTest()
{
    if (service_ != nullptr) {
        sleep(SLEEP_TIME);
        service_->Deinit();
        service_->Reset();
    }
    service_ = nullptr;
}

void DisplayPowerMgrTestCallback::OnDisplayStateChanged(
    uint32_t displayId, DisplayPowerMgr::DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(LABEL_TEST, "DisplayPowerMgrTestCallback::OnDisplayStateChangedStub");
}

void DisplayFuzzerTest::TestDisplayServiceStub(const uint32_t code, const uint8_t* data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(DisplayPowerMgrService::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(REWIND_READ_DATA);
    if (code == static_cast<uint32_t>(IDisplayPowerMgrIpcCode::COMMAND_NOTIFY_SCREEN_POWER_STATUS)) {
        constexpr uint32_t displayId = 0;
        constexpr uint32_t screenPowerStatus = 3; // 3 represent the POWER_STATUS_OFF
        datas.WriteUint32(displayId);
        datas.WriteUint32(screenPowerStatus);
    }
    if (code == static_cast<uint32_t>(IDisplayPowerMgrIpcCode::COMMAND_REGISTER_CALLBACK)) {
        sptr<IDisplayPowerCallback> obj = new DisplayPowerMgrTestCallback();
        datas.WriteRemoteObject(obj->AsObject());
    }
    if (code == static_cast<uint32_t>(IDisplayPowerMgrIpcCode::COMMAND_SET_LIGHT_BRIGHTNESS_THRESHOLD)) {
        sptr<IDisplayBrightnessCallback> obj = new DisplayBrightnessCallbackStub();
        datas.WriteRemoteObject(obj->AsObject());
    }
    MessageParcel reply;
    MessageOption option;
    service_->OnRemoteRequest(code, datas, reply, option);
}
