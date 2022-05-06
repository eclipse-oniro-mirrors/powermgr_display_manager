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

#include "display_power_mgr_proxy.h"

#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
bool DisplayPowerMgrProxy::SetDisplayState(uint32_t id, DisplayState state, uint32_t reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(state), false);
    WRITE_PARCEL_WITH_RET(data, Uint32, reason, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::SET_DISPLAY_STATE),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed, error code: %d",
            __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

DisplayState DisplayPowerMgrProxy::GetDisplayState(uint32_t id)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, DisplayState::DISPLAY_UNKNOWN);

    uint32_t result = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return DisplayState::DISPLAY_UNKNOWN;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, DisplayState::DISPLAY_UNKNOWN);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_DISPLAY_STATE),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return DisplayState::DISPLAY_UNKNOWN;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return DisplayState::DISPLAY_UNKNOWN;
    }

    return static_cast<DisplayState>(result);
}

std::vector<uint32_t> DisplayPowerMgrProxy::GetDisplayIds()
{
    sptr<IRemoteObject> remote = Remote();
    std::vector<uint32_t> result;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    uint32_t count = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_DISPLAY_IDS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(count)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    for (uint32_t i = 0; i < count; i++) {
        uint32_t value;
        if (reply.ReadUint32(value)) {
            result.push_back(value);
        } else {
            DISPLAY_HILOGE(COMP_FWK, "read value fail: %{public}d", i);
        }
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::GetMainDisplayId()
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_MAIN_DISPLAY_ID),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::SetBrightness(uint32_t value, uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, value, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(static_cast<int32_t>(IDisplayPowerMgr::SET_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %{public}d", __func__,
                       ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::OverrideBrightness(uint32_t value, uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, value, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(static_cast<int32_t>(IDisplayPowerMgr::OVERRIDE_BRIGHTNESS), data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %{public}d", __func__,
                       ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::RestoreBrightness(uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::RESTORE_BRIGHTNESS),
                                  data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::GetBrightness(uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_BRIGHTNESS),
                                  data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::GetDefaultBrightness()
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_DEFAULT_BRIGHTNESS),
                                  data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::GetMaxBrightness()
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_MAX_BRIGHTNESS),
                                  data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::GetMinBrightness()
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_MIN_BRIGHTNESS),
                                  data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Int32, value, false);
    WRITE_PARCEL_WITH_RET(data, Int32, duration, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %d", __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::AutoAdjustBrightness(bool enable)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Bool, enable, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::AUTO_ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %d", __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::IsAutoAdjustBrightness()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::IS_AUTO_ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %{public}d", __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::SetStateConfig(uint32_t id, DisplayState state, int32_t value)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(state), false);
    WRITE_PARCEL_WITH_RET(data, Int32, value, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::SET_STATE_CONFIG),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d",
            __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::RegisterCallback(sptr<IDisplayPowerCallback> callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::REGISTER_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %d", __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
