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
#include "xcollie.h"

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

    const int DFX_DELAY_MS = 10000;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("DisplayPowerMgrStub", DFX_DELAY_MS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_NOOP);
    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(IDisplayPowerMgr::SET_DISPLAY_STATE):
            ret = SetDisplayStateStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::GET_DISPLAY_STATE):
            ret = GetDisplayStateStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::GET_DISPLAY_IDS):
            ret = GetDisplayIdsStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::GET_MAIN_DISPLAY_ID):
            ret = GetMainDisplayIdStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::SET_BRIGHTNESS):
            ret = SetBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::OVERRIDE_BRIGHTNESS):
            ret = OverrideBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::RESTORE_BRIGHTNESS):
            ret = RestoreBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::GET_BRIGHTNESS):
            ret = GetBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::ADJUST_BRIGHTNESS):
            ret = AdjustBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::AUTO_ADJUST_BRIGHTNESS):
            ret = AutoAdjustBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(IDisplayPowerMgr::SET_STATE_CONFIG):
            ret = SetStateConfigStub(data, reply);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }

    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t DisplayPowerMgrStub::SetDisplayStateStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    uint32_t state = 0;
    uint32_t reason = 0;

    READ_PARCEL_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);

    bool ret = SetDisplayState(id, static_cast<DisplayState>(state), reason);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetDisplayStateStub return value");
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
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write GetDisplayStateStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetDisplayIdsStub(MessageParcel& data, MessageParcel& reply)
{
    std::vector<uint32_t> result = GetDisplayIds();
    if (!reply.WriteUint32(static_cast<uint32_t>(result.size()))) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write GetDisplayIdsStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    for (uint32_t i = 0; i < result.size(); i++) {
        if (!reply.WriteUint32(static_cast<uint32_t>(result[i]))) {
            DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write GetDisplayIdsStub");
        }
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetMainDisplayIdStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t result = GetMainDisplayId();
    if (!reply.WriteUint32(result)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write GetMainDisplayIdStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t value = 0;
    uint32_t displayId = 0;

    READ_PARCEL_WITH_RET(data, Uint32, value, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    bool ret = SetBrightness(value, displayId);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::OverrideBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t value = 0;
    uint32_t displayId = 0;

    READ_PARCEL_WITH_RET(data, Uint32, value, E_READ_PARCEL_ERROR);
    READ_PARCEL_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    bool ret = OverrideBrightness(value, displayId);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write OverrideBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::RestoreBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;

    READ_PARCEL_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    bool ret = RestoreBrightness(displayId);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write RestoreBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;

    READ_PARCEL_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    uint32_t ret = GetBrightness(displayId);
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write GetBrightness return value");
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
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write AdjustBrightnessStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::AutoAdjustBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    bool enable = 0;

    READ_PARCEL_WITH_RET(data, Bool, enable, E_READ_PARCEL_ERROR);

    bool ret = AutoAdjustBrightness(enable);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write AutoAdjustBrightnessStub return value");
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
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to write SetStateConfigStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::RegisterCallbackStub(MessageParcel& data, MessageParcel& reply)
{
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IDisplayPowerCallback> callback = iface_cast<IDisplayPowerCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    RegisterCallback(callback);
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
