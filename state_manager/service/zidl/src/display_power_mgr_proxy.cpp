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

#include "display_power_mgr_proxy.h"

#include "errors.h"
#include "message_option.h"
#include "message_parcel.h"
#include "display_log.h"
#include "display_common.h"
#include "display_power_mgr_ipc_interface_code.h"

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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, id, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, static_cast<uint32_t>(state), false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, reason, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_DISPLAY_STATE),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return DisplayState::DISPLAY_UNKNOWN;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, id, DisplayState::DISPLAY_UNKNOWN);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DISPLAY_STATE),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DISPLAY_IDS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_MAIN_DISPLAY_ID),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::SetBrightness(uint32_t value, uint32_t displayId, bool continuous)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, value, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, continuous, false);

    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, result);
    lastError_ = static_cast<DisplayErrors>(error);

    return result;
}

bool DisplayPowerMgrProxy::SetMaxBrightness(double value, uint32_t enterTestMode)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Double, value, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, enterTestMode, false);

    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_MAX_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, result);
    lastError_ = static_cast<DisplayErrors>(error);
    return result;
}

bool DisplayPowerMgrProxy::SetMaxBrightnessNit(uint32_t maxNit, uint32_t enterTestMode)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, maxNit, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, enterTestMode, false);

    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_MAX_BRIGHTNESS_NIT),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, result);
    lastError_ = static_cast<DisplayErrors>(error);

    return result;
}

bool DisplayPowerMgrProxy::DiscountBrightness(double discount, uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Double, discount, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::DISCOUNT_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return false;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return false;
    }

    return result;
}

bool DisplayPowerMgrProxy::OverrideBrightness(uint32_t value, uint32_t displayId, uint32_t duration)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, value, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, duration, false);

    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::OVERRIDE_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::OverrideDisplayOffDelay(uint32_t delayMs)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, delayMs, false);

    int ret = remote->SendRequest(
        static_cast<int32_t>(PowerMgr::DisplayPowerMgrInterfaceCode::OVERRIDE_DISPLAY_OFF_DELAY),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::RestoreBrightness(uint32_t displayId, uint32_t duration)
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, duration, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::RESTORE_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_BRIGHTNESS),
                                  data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DEFAULT_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_MAX_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_MIN_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, id, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, value, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, duration, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, enable, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::AUTO_ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::IS_AUTO_ADJUST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
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
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::REGISTER_CALLBACK),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::BoostBrightness(int32_t timeoutMs, uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32, timeoutMs, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::BOOST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::CancelBoostBrightness(uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::CANCEL_BOOST_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::GetDeviceBrightness(uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;

    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::GET_DEVICE_BRIGHTNESS),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

bool DisplayPowerMgrProxy::SetCoordinated(bool coordinated, uint32_t displayId)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Bool, coordinated, false);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, false);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_COORDINATED),
        data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

uint32_t DisplayPowerMgrProxy::SetLightBrightnessThreshold(
    std::vector<int32_t> threshold, sptr<IDisplayBrightnessCallback> callback)
{
    sptr<IRemoteObject> remote = Remote();
    uint32_t result = 0;
    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return result;
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Int32Vector, threshold, result);
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, RemoteObject, callback->AsObject(), result);

    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::SET_APS_LIGHT_AND_BRIGHTNESS_THRESOLD), data, reply,
        option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return result;
    }

    if (!reply.ReadUint32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }

    return result;
}

DisplayErrors DisplayPowerMgrProxy::GetError()
{
    return lastError_;
}

int DisplayPowerMgrProxy::NotifyScreenPowerStatus(uint32_t displayId, uint32_t status)
{
    sptr<IRemoteObject> remote = Remote();
    int result = -1;
    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(DisplayPowerMgrProxy::GetDescriptor())) {
        DISPLAY_HILOGE(COMP_FWK, "write descriptor failed!");
        return -1; // -1 means failed
    }

    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, displayId, -1); // -1 means failed
    RETURN_IF_WRITE_PARCEL_FAILED_WITH_RET(data, Uint32, status, -1); // -1 means failed
    int ret = remote->SendRequest(
        static_cast<int>(PowerMgr::DisplayPowerMgrInterfaceCode::NOTIFY_DISPLAY_POWER_STATUS), data, reply, option);
    if (ret != ERR_OK) {
        DISPLAY_HILOGE(COMP_FWK, "SendRequest is failed, error code: %d", ret);
        return ret;
    }

    if (!reply.ReadInt32(result)) {
        DISPLAY_HILOGE(COMP_FWK, "Readback fail!");
        return result;
    }
    int32_t error;
    RETURN_IF_READ_PARCEL_FAILED_WITH_RET(reply, Int32, error, result);
    lastError_ = static_cast<DisplayErrors>(error);
    return result;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
