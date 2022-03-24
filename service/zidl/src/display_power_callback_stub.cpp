/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "display_power_callback_stub.h"

#include <message_parcel.h>

#include "display_common.h"
#include "xcollie.h"

namespace OHOS {
namespace DisplayPowerMgr {
int32_t DisplayPowerCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DISPLAY_HILOGD(COMP_SVC, "DisplayPowerCallbackStub::OnRemoteRequest, cmd = %d, flags= %d",
        code, option.GetFlags());
    std::u16string descripter = DisplayPowerCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(COMP_SVC,
            "DisplayPowerCallbackStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("DisplayPowerCallbackStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);
    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(IDisplayPowerCallback::ON_DISPLAY_STATE_CHANGED):
            ret = OnDisplayStateChangedStub(data, reply);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t DisplayPowerCallbackStub::OnDisplayStateChangedStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    uint32_t state = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);

    OnDisplayStateChanged(id, static_cast<DisplayState>(state));
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
