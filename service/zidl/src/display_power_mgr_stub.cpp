/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "display_power_mgr_stub.h"

#include <message_parcel.h>

#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
int32_t DisplayPowerMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DISPLAY_HILOGD(MODULE_SERVICE, "DisplayPowerMgrStub::OnRemoteRequest, cmd = %d, flags= %d",
        code, option.GetFlags());
    std::u16string descripter = DisplayPowerMgrStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(MODULE_SERVICE, "DisplayPowerMgrStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    switch (code) {
        case static_cast<int32_t>(IDisplayPowerMgr::SET_DISPLAY_STATE):
            return SetDisplayStateStub(data, reply);
        case static_cast<int32_t>(IDisplayPowerMgr::GET_DISPLAY_STATE):
            return GetDisplayStateStub(data, reply);
        case static_cast<int32_t>(IDisplayPowerMgr::SET_BRIGHTNESS):
            return SetBrightnessStub(data, reply);
        case static_cast<int32_t>(IDisplayPowerMgr::ADJUST_BRIGHTNESS):
            return AdjustBrightnessStub(data, reply);
        case static_cast<int32_t>(IDisplayPowerMgr::SET_STATE_CONFIG):
            return SetStateConfigStub(data, reply);
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t DisplayPowerMgrStub::SetDisplayStateStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    uint32_t state = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);

    bool ret = SetDisplayState(id, static_cast<DisplayState>(state));
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetScreenState return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetDisplayStateStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);

    DisplayState ret = GetDisplayState(id);
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetScreenState return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}


int32_t DisplayPowerMgrStub::SetBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    int32_t value = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, value, E_READ_PARCEL_ERROR);

    bool ret = SetBrightness(id, value);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::AdjustBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    int32_t value = 0;
    uint32_t duration = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, value, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, duration, E_READ_PARCEL_ERROR);

    bool ret = AdjustBrightness(id, value, duration);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetStateConfigStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    uint32_t state = 0;
    int32_t value = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Int32, value, E_READ_PARCEL_ERROR);

    bool ret = SetStateConfig(id, static_cast<DisplayState>(state), value);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
