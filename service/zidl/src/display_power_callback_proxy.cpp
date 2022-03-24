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

#include "display_power_callback_proxy.h"

#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
void DisplayPowerCallbackProxy::OnDisplayStateChanged(uint32_t displayId, DisplayState state)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerCallbackProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK,
            "DisplayPowerCallbackProxy::%{public}s write descriptor failed!", __func__);
        return;
    }

    WRITE_PARCEL_NO_RET(data, Uint32, displayId);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(state));

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerCallback::ON_DISPLAY_STATE_CHANGED),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK,
            "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d", __func__, ret);
        return;
    }

    return;
}
} // namespace DisplayPowerMgr
} // namespace OHOS