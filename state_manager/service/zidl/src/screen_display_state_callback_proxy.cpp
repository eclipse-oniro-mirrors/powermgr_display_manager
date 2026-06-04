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

#include "screen_display_state_callback_proxy.h"

#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "display_log.h"
#include "display_common.h"
#include "screen_display_state_callback_ipc_interface_code.h"

namespace OHOS {
namespace DisplayPowerMgr {
void ScreenDisplayStateCallbackProxy::OnDisplayStateChanged(uint64_t displayId, uint32_t state, uint32_t reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(ScreenDisplayStateCallbackProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint64, displayId);
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, state);
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, reason);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::ScreenDisplayStateCallbackInterfaceCode::ON_DISPLAY_STATE_CHANGED),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return;
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
