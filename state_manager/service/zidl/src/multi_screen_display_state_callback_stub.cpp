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

#include "multi_screen_display_state_callback_stub.h"

#include "display_common.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "errors.h"
#include "imulti_screen_display_state_callback.h"
#include "ipc_object_stub.h"
#include "message_option.h"
#include "message_parcel.h"
#include "multi_screen_display_state_callback_ipc_interface_code.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace DisplayPowerMgr {
int32_t MultiScreenDisplayStateCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply,
    MessageOption& option)
{
    DISPLAY_HILOGD(COMP_SVC,
        "MultiScreenDisplayStateCallbackStub::OnRemoteRequest, cmd = %{public}d, flags = %{public}d",
        code, option.GetFlags());
    std::u16string descripter = MultiScreenDisplayStateCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(COMP_SVC, "descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    const int32_t DFX_DELAY_S = 3;
    int32_t xcollieId = HiviewDFX::XCollie::GetInstance().SetTimer(
        "MultiScreenDisplayStateCallbackStub", DFX_DELAY_S, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    int32_t ret = ERR_OK;
    if (code == static_cast<uint32_t>(
        MultiScreenDisplayStateCallbackInterfaceCode::ON_MULTI_SCREEN_DISPLAY_STATE_CHANGED)) {
        ret = OnMultiScreenDisplayStateChangedStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(xcollieId);
    return ret;
}

int32_t MultiScreenDisplayStateCallbackStub::OnMultiScreenDisplayStateChangedStub(
    MessageParcel& data, MessageParcel& reply)
{
    uint64_t screenId = 0;
    std::string screenName;
    uint32_t state = 0;
    uint32_t reason = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint64, screenId, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, String, screenName, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);

    OnMultiScreenDisplayStateChanged(screenId, screenName, static_cast<DisplayState>(state),
        static_cast<MultiScreenStateChangeReason>(reason));
    return ERR_OK;
}

void MultiScreenDisplayStateCallbackStub::OnMultiScreenDisplayStateChanged(uint64_t screenId,
    const std::string& screenName, DisplayState state, MultiScreenStateChangeReason reason) {}
} // namespace DisplayPowerMgr
} // namespace OHOS
