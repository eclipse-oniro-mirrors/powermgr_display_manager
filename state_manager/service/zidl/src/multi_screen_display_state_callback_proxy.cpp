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

#include "multi_screen_display_state_callback_proxy.h"

#include "display_common.h"
#include "display_log.h"
#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "multi_screen_display_state_callback_ipc_interface_code.h"

namespace OHOS {
namespace DisplayPowerMgr {
void MultiScreenDisplayStateCallbackProxy::OnMultiScreenDisplayStateChanged(uint64_t screenId,
    const std::string& screenName, DisplayState state, MultiScreenStateChangeReason reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(MultiScreenDisplayStateCallbackProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint64, screenId);
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, String, screenName);
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, static_cast<uint32_t>(state));
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, static_cast<uint32_t>(reason));

    auto id = static_cast<int>(MultiScreenDisplayStateCallbackInterfaceCode::ON_MULTI_SCREEN_DISPLAY_STATE_CHANGED);
    int ret = remote->SendRequest(id, data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %{public}d", ret);
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
