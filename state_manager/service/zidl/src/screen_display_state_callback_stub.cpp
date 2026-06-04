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

#include "screen_display_state_callback_stub.h"

#include <message_parcel.h>
#include "errors.h"
#include "display_common.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "screen_display_state_callback_ipc_interface_code.h"
#include "ipc_object_stub.h"
#include "message_option.h"

namespace OHOS {
namespace DisplayPowerMgr {
int32_t ScreenDisplayStateCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DISPLAY_HILOGD(COMP_SVC, "ScreenDisplayStateCallbackStub::OnRemoteRequest, cmd = %d, flags= %d",
        code, option.GetFlags());
    std::u16string descripter = ScreenDisplayStateCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(COMP_SVC, "descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    int32_t ret = ERR_OK;
    if (code == static_cast<uint32_t>(PowerMgr::ScreenDisplayStateCallbackInterfaceCode::ON_DISPLAY_STATE_CHANGED)) {
        ret = OnDisplayStateChangedStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return ret;
}

int32_t ScreenDisplayStateCallbackStub::OnDisplayStateChangedStub(MessageParcel& data, MessageParcel& reply)
{
    uint64_t displayId = 0;
    uint32_t state = 0;
    uint32_t reason = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint64, displayId, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);

    OnDisplayStateChanged(displayId, state, reason);
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
