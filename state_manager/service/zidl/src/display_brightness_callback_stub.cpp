/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "display_brightness_callback_stub.h"

#include "display_brightness_callback_ipc_interface_code.h"
#include "display_common.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "display_power_info.h"
#include "errors.h"
#include "idisplay_brightness_callback.h"
#include "ipc_object_stub.h"
#include "message_option.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"
#include <message_parcel.h>

namespace OHOS {
namespace DisplayPowerMgr {
int32_t DisplayBrightnessCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    DISPLAY_HILOGD(
        COMP_SVC, "DisplayBrightnessCallbackStub::OnRemoteRequest, cmd = %d, flags= %d", code, option.GetFlags());
    std::u16string descripter = DisplayBrightnessCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(COMP_SVC, "descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer(
        "DisplayBrightnessCallbackStub", DFX_DELAY_MS, nullptr, nullptr, HiviewDFX::XCOLLIE_FLAG_NOOP);
    int32_t ret = ERR_OK;
    if (code ==
        static_cast<uint32_t>(
            PowerMgr::DisplayBrightnessCallbackInterfaceCode::ON_NOTIFY_APS_LIGHT_BRIGHTNESS_CHANGE)) {
        ret = OnNotifyApsLightBrightnessChangeStub(data, reply);
    } else {
        ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t DisplayBrightnessCallbackStub::OnNotifyApsLightBrightnessChangeStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t type = 0;
    bool state = false;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, type, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, state, E_READ_PARCEL_ERROR);

    OnNotifyApsLightBrightnessChange(type, state);
    return ERR_OK;
}

void DisplayBrightnessCallbackStub::OnNotifyApsLightBrightnessChange(uint32_t type, bool state) {}

} // namespace DisplayPowerMgr
} // namespace OHOS
