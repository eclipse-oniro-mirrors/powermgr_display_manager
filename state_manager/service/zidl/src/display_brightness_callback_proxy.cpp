/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "display_brightness_callback_proxy.h"

#include "display_brightness_callback_ipc_interface_code.h"
#include "display_common.h"
#include "display_log.h"
#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace DisplayPowerMgr {
void DisplayBrightnessCallbackProxy::OnNotifyApsLightBrightnessChange(uint32_t type, bool state)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(DisplayBrightnessCallbackProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Uint32, type);
    RETURN_IF_WRITE_PARCEL_FAILED_NO_RET(data, Bool, state);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayBrightnessCallbackInterfaceCode::ON_NOTIFY_APS_LIGHT_BRIGHTNESS_CHANGE), data,
        reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return;
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS