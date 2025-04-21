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

#include "display_power_mgr_stub.h"

#include <message_parcel.h>
#include "errors.h"
#include "ipc_object_stub.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "idisplay_power_callback.h"
#include "display_log.h"
#include "display_common.h"
#include "display_mgr_errors.h"
#include "display_power_info.h"
#include "display_power_mgr_ipc_interface_code.h"
#include "xcollie/xcollie.h"
#include "xcollie/xcollie_define.h"

namespace OHOS {
namespace DisplayPowerMgr {
int32_t DisplayPowerMgrStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DISPLAY_HILOGD(COMP_SVC, "DisplayPowerMgrStub::OnRemoteRequest, cmd = %d, flags= %d",
        code, option.GetFlags());
    std::u16string descripter = DisplayPowerMgrStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (descripter != remoteDescripter) {
        DISPLAY_HILOGE(COMP_SVC, "DisplayPowerMgrStub::OnRemoteRequest failed, descriptor is not matched!");
        return E_GET_POWER_SERVICE_FAILED;
    }

    constexpr int dfxDelayS = 60;
    int id = HiviewDFX::XCollie::GetInstance().SetTimer("DisplayPowerMgrStub", dfxDelayS, nullptr, nullptr,
        HiviewDFX::XCOLLIE_FLAG_LOG);

    int32_t ret = ProcessMessage(code, data, reply, option);
    HiviewDFX::XCollie::GetInstance().CancelTimer(id);
    return ret;
}

int32_t DisplayPowerMgrStub::ProcessMessage(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_DISPLAY_STATE):
            ret = SetDisplayStateStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DISPLAY_STATE):
            ret = GetDisplayStateStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DISPLAY_IDS):
            ret = GetDisplayIdsStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_MAIN_DISPLAY_ID):
            ret = GetMainDisplayIdStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_BRIGHTNESS):
            ret = SetBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::DISCOUNT_BRIGHTNESS):
            ret = DiscountBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::OVERRIDE_BRIGHTNESS):
            ret = OverrideBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::OVERRIDE_DISPLAY_OFF_DELAY):
            ret = OverrideDisplayOffDelayStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_APS_LIGHT_AND_BRIGHTNESS_THRESOLD):
            ret = SetLightBrightnessThresholdStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_MAX_BRIGHTNESS):
            ret = SetMaxBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_MAX_BRIGHTNESS_NIT):
            ret = SetMaxBrightnessNitStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::NOTIFY_DISPLAY_POWER_STATUS):
            ret = NotifyScreenPowerStatusStub(data, reply);
            break;
        default:
            ret = RemoteRequest(code, data, reply, option);
            break;
    }
    return ret;
}

int32_t DisplayPowerMgrStub::RemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    int32_t ret = ERR_OK;
    switch (code) {
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::RESTORE_BRIGHTNESS):
            ret = RestoreBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_BRIGHTNESS):
            ret = GetBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DEFAULT_BRIGHTNESS):
            ret = GetDefaultBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_MAX_BRIGHTNESS):
            ret = GetMaxBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_MIN_BRIGHTNESS):
            ret = GetMinBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::ADJUST_BRIGHTNESS):
            ret = AdjustBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::AUTO_ADJUST_BRIGHTNESS):
            ret = AutoAdjustBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::IS_AUTO_ADJUST_BRIGHTNESS):
            ret = IsAutoAdjustBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::REGISTER_CALLBACK):
            ret = RegisterCallbackStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::BOOST_BRIGHTNESS):
            ret = BoostBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::CANCEL_BOOST_BRIGHTNESS):
            ret = CancelBoostBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DEVICE_BRIGHTNESS):
            ret = GetDeviceBrightnessStub(data, reply);
            break;
        case static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_COORDINATED):
            ret = SetCoordinatedStub(data, reply);
            break;
        default:
            ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }

    return ret;
}

int32_t DisplayPowerMgrStub::SetDisplayStateStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    uint32_t state = 0;
    uint32_t reason = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, state, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, reason, E_READ_PARCEL_ERROR);

    bool ret = SetDisplayState(id, static_cast<DisplayState>(state), reason);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write SetDisplayStateStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetDisplayStateStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);

    DisplayState ret = GetDisplayState(id);
    if (!reply.WriteUint32(static_cast<uint32_t>(ret))) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetDisplayStateStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetDisplayIdsStub(MessageParcel& data, MessageParcel& reply)
{
    std::vector<uint32_t> result = GetDisplayIds();
    if (!reply.WriteUint32(static_cast<uint32_t>(result.size()))) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetDisplayIdsStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    for (uint32_t i = 0; i < result.size(); i++) {
        if (!reply.WriteUint32(static_cast<uint32_t>(result[i]))) {
            DISPLAY_HILOGE(COMP_SVC, "Failed to write GetDisplayIdsStub");
        }
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetMainDisplayIdStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t result = GetMainDisplayId();
    if (!reply.WriteUint32(result)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetMainDisplayIdStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t value = 0;
    uint32_t displayId = 0;
    bool continuous = false;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, value, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, continuous, E_READ_PARCEL_ERROR);

    bool ret = SetBrightness(value, displayId, continuous);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write SetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    int32_t error = static_cast<int32_t>(GetError());
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, error, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::DiscountBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    double discount = 0;
    uint32_t displayId = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Double, discount, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    bool ret = DiscountBrightness(discount, displayId);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to wirte DiscountBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::OverrideBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t value = 0;
    uint32_t displayId = 0;
    uint32_t duration = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, value, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, duration, E_READ_PARCEL_ERROR);

    bool ret = OverrideBrightness(value, displayId, duration);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write OverrideBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::OverrideDisplayOffDelayStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t delayMs = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, delayMs, E_READ_PARCEL_ERROR);

    bool ret = OverrideDisplayOffDelay(delayMs);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write OverrideDisplayOffDelay return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::RestoreBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;
    uint32_t duration = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, duration, E_READ_PARCEL_ERROR);

    bool ret = RestoreBrightness(displayId, duration);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write RestoreBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    uint32_t ret = GetBrightness(displayId);
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetDefaultBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t ret = GetDefaultBrightness();
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetDefaultBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetMaxBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t ret = GetMaxBrightness();
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetMaxBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetMinBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t ret = GetMinBrightness();
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetMinBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::AdjustBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t id = 0;
    int32_t value = 0;
    uint32_t duration = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, value, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, duration, E_READ_PARCEL_ERROR);

    bool ret = AdjustBrightness(id, value, duration);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write AdjustBrightnessStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::AutoAdjustBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    bool enable = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, enable, E_READ_PARCEL_ERROR);

    bool ret = AutoAdjustBrightness(enable);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write AutoAdjustBrightnessStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::IsAutoAdjustBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    bool ret = IsAutoAdjustBrightness();
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write IsAutoAdjustBrightnessStub return value");
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
    bool isSucc = RegisterCallback(callback);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Bool, isSucc, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::BoostBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    int32_t timeoutMs = -1;
    uint32_t id = 0;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32, timeoutMs, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, id, E_READ_PARCEL_ERROR);

    bool isScuu = BoostBrightness(timeoutMs, id);
    if (!reply.WriteBool(isScuu)) {
        DISPLAY_HILOGW(COMP_SVC, "Failed to write BoostBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::CancelBoostBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    bool isScuu = CancelBoostBrightness(displayId);
    if (!reply.WriteBool(isScuu)) {
        DISPLAY_HILOGW(COMP_SVC, "Failed to write CancelBoostBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::GetDeviceBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    uint32_t ret = GetDeviceBrightness(displayId);
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write GetDeviceBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetCoordinatedStub(MessageParcel& data, MessageParcel& reply)
{
    bool coordinated = false;
    uint32_t displayId = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Bool, coordinated, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);

    bool ret = SetCoordinated(coordinated, displayId);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write SetCoordinatedStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetLightBrightnessThresholdStub(MessageParcel& data, MessageParcel& reply)
{
    std::vector<int32_t> threshold;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Int32Vector, &threshold, E_READ_PARCEL_ERROR);
    sptr<IRemoteObject> obj = data.ReadRemoteObject();
    RETURN_IF_WITH_RET((obj == nullptr), E_READ_PARCEL_ERROR);
    sptr<IDisplayBrightnessCallback> callback = iface_cast<IDisplayBrightnessCallback>(obj);
    RETURN_IF_WITH_RET((callback == nullptr), E_READ_PARCEL_ERROR);
    uint32_t ret = SetLightBrightnessThreshold(threshold, callback);
    if (!reply.WriteUint32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write SetLightBrightnessThresholdStub return value");
        return E_WRITE_PARCEL_ERROR;
    }
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetMaxBrightnessStub(MessageParcel& data, MessageParcel& reply)
{
    double value = 0;
    uint32_t enterTestMode = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Double, value, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, enterTestMode, E_READ_PARCEL_ERROR);

    bool ret = SetMaxBrightness(value, enterTestMode);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write SetMaxBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    int32_t error = static_cast<int32_t>(GetError());
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, error, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::SetMaxBrightnessNitStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t value = 0;
    uint32_t enterTestMode = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, value, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, enterTestMode, E_READ_PARCEL_ERROR);

    bool ret = SetMaxBrightnessNit(value, enterTestMode);
    if (!reply.WriteBool(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write SetMaxBrightness return value");
        return E_WRITE_PARCEL_ERROR;
    }
    int32_t error = static_cast<int32_t>(GetError());
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, error, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}

int32_t DisplayPowerMgrStub::NotifyScreenPowerStatusStub(MessageParcel& data, MessageParcel& reply)
{
    uint32_t displayId = 0;
    uint32_t displayPowerStatus = 0;

    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, E_READ_PARCEL_ERROR);
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(data, Uint32, displayPowerStatus, E_READ_PARCEL_ERROR);

    int ret = NotifyScreenPowerStatus(displayId, displayPowerStatus);
    if (!reply.WriteInt32(ret)) {
        DISPLAY_HILOGE(COMP_SVC, "Failed to write NotifyScreenPowerStatus return value");
        return E_WRITE_PARCEL_ERROR;
    }
    int32_t error = static_cast<int32_t>(GetError());
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(reply, Int32, error, E_WRITE_PARCEL_ERROR);
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
