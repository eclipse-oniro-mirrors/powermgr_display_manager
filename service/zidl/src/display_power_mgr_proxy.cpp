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

#include "display_power_mgr_proxy.h"

#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
bool DisplayPowerMgrProxy::SetDisplayState(uint32_t id, DisplayState state)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(state), false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::SET_DISPLAY_STATE),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s SendRequest is failed, error code: %d",
            __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
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
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrClient::%{public}s write descriptor failed!", __func__);
        return DisplayState::DISPLAY_UNKNOWN;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, DisplayState::DISPLAY_UNKNOWN);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::GET_DISPLAY_STATE),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s SendRequest is failed,%d", __func__, ret);
        return DisplayState::DISPLAY_UNKNOWN;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
        return DisplayState::DISPLAY_UNKNOWN;
    }

    return static_cast<DisplayState>(result);
}

bool DisplayPowerMgrProxy::SetBrightness(uint32_t id, int32_t value)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Int32, value, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::SET_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %d", __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
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
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Int32, value, false);
    WRITE_PARCEL_WITH_RET(data, Int32, duration, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s SendRequest is failed: %d", __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
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
        DISPLAY_HILOGE(MODULE_INNERKIT, "DisplayPowerMgrProxy::%{public}s write descriptor failed!", __func__);
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, id, false);
    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(state), false);
    WRITE_PARCEL_WITH_RET(data, Int32, value, false);

    int ret = remote->SendRequest(static_cast<int>(IDisplayPowerMgr::SET_STATE_CONFIG),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "PowerMgrProxy::%{public}s SendRequest is failed, error code: %d",
            __func__, ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(MODULE_INNERKIT, "Readback fail!");
        return result;
    }

    return result;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
