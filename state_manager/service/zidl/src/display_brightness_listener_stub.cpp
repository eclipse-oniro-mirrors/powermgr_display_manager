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

#include "display_brightness_listener_stub.h"

#include "display_brightness_listener_ipc_interface_code.h"
#include "display_common.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "display_power_info.h"
#include "errors.h"
#include "idisplay_brightness_listener.h"
#include "ipc_object_stub.h"
#include "message_option.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include <message_parcel.h>

namespace OHOS {
namespace DisplayPowerMgr {
int32_t DisplayBrightnessListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    DISPLAY_HILOGD(
        COMP_SVC, "DisplayBrightnessListenerStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descripter = DisplayBrightnessListenerStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(COMP_SVC, "descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    const int DFX_DELAY_S = 60;
    const int INVALID_ID = 0;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "DisplayBrightnessListenerStub", DFX_DELAY_S, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_LOG);
    if (id <= INVALID_ID) {
        DISPLAY_HILOGE(COMP_SVC, "SetTimer failed");
    }
    int32_t ret = ERR_OK;
    if (code == static_cast<uint32_t>(DisplayBrightnessListenerInterfaceCode::ON_DATA_CHANGED)) {
        OnDataChanged(data.ReadString());
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id); // cancel to prevent freeze, even when id <= INVALID_ID
    return ret;
}

void DisplayBrightnessListenerStub::OnDataChanged(const std::string& params) {}

} // namespace DisplayPowerMgr
} // namespace OHOS
