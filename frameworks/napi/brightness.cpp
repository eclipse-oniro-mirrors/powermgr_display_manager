/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "brightness.h"

#include <utility>

#include "napi/native_common.h"
#include "power_state_machine_info.h"

#include "display_common.h"
#include "display_log.h"
#include "display_power_mgr_client.h"

using namespace OHOS::PowerMgr;

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
const uint32_t MAX_ARGC = 1;
const uint32_t ARGV_ONE = 0;
const uint32_t MAX_FAIL_ARGC = 2;
const int32_t MAX_BRIGHTNESS = 255;
const int32_t MIN_BRIGHTNESS = 1;
const int32_t BRIGHTNESS_OFF = 0;

const std::string FUNC_SUCEESS_NAME = "success";
const std::string FUNC_FAIL_NAME = "fail";
const std::string FUNC_COMPLETE_NAME = "complete";

const int32_t COMMON_ERROR_COED = 200;
const int32_t INPUT_ERROR_CODE = 202;

const std::string SET_VALUE_ERROR_MGR = "value is not an available number";
const std::string GET_VALUE_ERROR_MGR = "get system screen brightness fail";
const std::string SET_MODE_ERROR_MGR = "value is not an available number";
const std::string SET_MODE_NOT_SUPPORTED_ERROR_MGR = "Auto adjusting brightness is not supported";
const std::string SET_KEEP_SCREENON_ERROR_MGR = "value is not an available boolean";
} // namespace

std::map<DisplayErrors, std::string> Brightness::Result::errorTable_ = {
    {DisplayErrors::ERR_CONNECTION_FAIL,   "Connecting to the service failed."},
    {DisplayErrors::ERR_PERMISSION_DENIED, "Permission is denied"             },
    {DisplayErrors::ERR_PARAM_INVALID,     "Invalid input parameter."         }
};

Brightness::Brightness(napi_env env, std::shared_ptr<RunningLock> runningLock) : env_(env), runningLock_(runningLock) {}

void Brightness::GetValue()
{
    uint32_t brightness = brightnessInfo_.GetBrightness();
    if (BRIGHTNESS_OFF >= brightness || brightness > MAX_BRIGHTNESS) {
        result_.Error(COMMON_ERROR_COED, GET_VALUE_ERROR_MGR);
    } else {
        result_.SetResult(BRIGHTNESS_VALUE, brightness);
    }
    ExecuteCallback();
}

void Brightness::SetValue(napi_callback_info& info)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness interface");
    napi_value napiBrightness = GetCallbackInfo(info, napi_number);
    if (napiBrightness == nullptr) {
        result_.ThrowError(env_, DisplayErrors::ERR_PARAM_INVALID);
        return;
    }

    int32_t value = MIN_BRIGHTNESS;
    if (napi_ok != napi_get_value_int32(env_, napiBrightness, &value)) {
        DISPLAY_HILOGW(COMP_FWK, "Failed to get the input number");
        result_.ThrowError(env_, DisplayErrors::ERR_PARAM_INVALID);
        return;
    }
    if (!brightnessInfo_.SetBrightness(value)) {
        DisplayErrors error = brightnessInfo_.GetServiceError();
        if (error != DisplayErrors::ERR_OK) {
            result_.ThrowError(env_, error);
        }
    }
}

void Brightness::SystemSetValue()
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "System brightness interface");
    if (napiValRef_ == nullptr) {
        result_.Error(INPUT_ERROR_CODE, SET_VALUE_ERROR_MGR);
    } else {
        int32_t brightness = MIN_BRIGHTNESS;
        napi_value napiVal = nullptr;
        napi_get_reference_value(env_, napiValRef_, &napiVal);
        napi_get_value_int32(env_, napiVal, &brightness);
        brightnessInfo_.SetBrightness(brightness);
        napi_delete_reference(env_, napiValRef_);
    }
    ExecuteCallback();
}

void Brightness::GetMode()
{
    int32_t isAuto = brightnessInfo_.GetAutoMode();
    result_.SetResult(BRIGHTNESS_MODE, isAuto);
    ExecuteCallback();
}

void Brightness::SetMode()
{
    DISPLAY_HILOGD(COMP_FWK, "Set auto brightness");
    if (napiValRef_ == nullptr) {
        result_.Error(INPUT_ERROR_CODE, SET_MODE_ERROR_MGR);
    } else {
        int32_t mode = 0;
        napi_value napiMode = nullptr;
        napi_get_reference_value(env_, napiValRef_, &napiMode);
        napi_get_value_int32(env_, napiMode, &mode);
        if (!brightnessInfo_.SetAutoMode(static_cast<bool>(mode))) {
            result_.Error(COMMON_ERROR_COED, SET_MODE_NOT_SUPPORTED_ERROR_MGR);
        }
        napi_delete_reference(env_, napiValRef_);
    }
    ExecuteCallback();
}

void Brightness::SetKeepScreenOn()
{
    DISPLAY_HILOGD(COMP_FWK, "Set keep screen on");
    if (napiValRef_ == nullptr) {
        result_.Error(INPUT_ERROR_CODE, SET_KEEP_SCREENON_ERROR_MGR);
    } else {
        napi_value napiKeep = nullptr;
        napi_get_reference_value(env_, napiValRef_, &napiKeep);
        bool screenOn = false;
        napi_get_value_bool(env_, napiKeep, &screenOn);
        brightnessInfo_.ScreenOn(screenOn, runningLock_);
        napi_delete_reference(env_, napiValRef_);
    }
    ExecuteCallback();
}

napi_value Brightness::GetCallbackInfo(napi_callback_info& info, napi_valuetype checkType)
{
    size_t argc = MAX_ARGC;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    if (napi_ok != napi_get_cb_info(env_, info, &argc, argv, &thisVar, &data)) {
        DISPLAY_HILOGW(COMP_FWK, "Failed to get the input parameter");
        return nullptr;
    }

    if (argc != MAX_ARGC) {
        DISPLAY_HILOGW(COMP_FWK, "Lack of parameter");
        return nullptr;
    }

    napi_value options = argv[ARGV_ONE];
    RETURN_IF_WITH_RET(!CheckValueType(options, checkType), nullptr);
    return options;
}

bool Brightness::CreateCallbackRef(napi_value& options)
{
    RETURN_IF_WITH_RET(!CheckValueType(options, napi_object), false);

    napi_value succCallBack = GetOptions(options, FUNC_SUCEESS_NAME, napi_function);
    if (succCallBack != nullptr) {
        napi_create_reference(env_, succCallBack, 1, &successRef_);
    }

    napi_value failCallBack = GetOptions(options, FUNC_FAIL_NAME, napi_function);
    if (failCallBack != nullptr) {
        napi_create_reference(env_, failCallBack, 1, &failRef_);
    }

    napi_value completeCallBack = GetOptions(options, FUNC_COMPLETE_NAME, napi_function);
    if (completeCallBack != nullptr) {
        napi_create_reference(env_, completeCallBack, 1, &completeRef_);
    }
    return true;
}

void Brightness::CreateValueRef(napi_value& options, const std::string& valName, napi_valuetype checkType)
{
    napi_value value = GetOptions(options, valName, checkType);
    if (value != nullptr) {
        napi_create_reference(env_, value, 1, &napiValRef_);
    }
}

void Brightness::Result::Error(int32_t code, const std::string& msg)
{
    code_ = code;
    msg_ = msg;
    DISPLAY_HILOGW(COMP_FWK, "Error message, code: %{public}d, msg: %{public}s", code_, msg_.c_str());
}

void Brightness::Result::GetError(napi_env env, napi_value* error, size_t& size) const
{
    if (!error) {
        DISPLAY_HILOGW(COMP_FWK, "error is null");
        return;
    }
    napi_value data = nullptr;
    napi_value code = nullptr;
    napi_create_string_utf8(env, msg_.c_str(), msg_.size(), &data);
    napi_create_int32(env, code_, &code);
    size = MAX_FAIL_ARGC;
    error[ARGV_ONE] = data;
    error[MAX_ARGC] = code;
}

napi_value Brightness::Result::GetError(napi_env& env)
{
    napi_value napiError = nullptr;
    if (!IsError()) {
        napi_get_undefined(env, &napiError);
        return napiError;
    }

    std::string msg;
    auto item = errorTable_.find(static_cast<DisplayErrors>(code_));
    if (item != errorTable_.end()) {
        msg = item->second;
    }
    napi_value napiMsg;
    NAPI_CALL(env, napi_create_string_utf8(env, msg.c_str(), msg.size(), &napiMsg));
    NAPI_CALL(env, napi_create_error(env, nullptr, napiMsg, &napiError));

    napi_value napiCode;
    NAPI_CALL(env, napi_create_int32(env, code_, &napiCode));

    napi_set_named_property(env, napiError, "code", napiCode);
    napi_set_named_property(env, napiError, "message", napiMsg);

    DISPLAY_HILOGW(COMP_FWK, "throw error code: %{public}d, msg: %{public}s,", code_, msg.c_str());
    return napiError;
}

napi_value Brightness::Result::ThrowError(napi_env& env, DisplayErrors code)
{
    Error(static_cast<int32_t>(code));
    napi_value error = GetError(env);
    RETURN_IF_WITH_RET(error == nullptr, nullptr);
    napi_throw(env, error);
    return nullptr;
}

napi_value Brightness::Result::GetResult(napi_env env)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_object(env, &result));
    for (const auto& it : mapResult_) {
        napi_value napiValue = 0;
        NAPI_CALL(env, napi_create_int32(env, it.second, &napiValue));
        NAPI_CALL(env, napi_set_named_property(env, result, it.first.c_str(), napiValue));
    }
    return result;
}

uint32_t Brightness::BrightnessInfo::GetBrightness() const
{
    uint32_t brightness = DisplayPowerMgrClient::GetInstance().GetBrightness();
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Get brightness: %{public}d", brightness);
    return brightness;
}

bool Brightness::BrightnessInfo::SetBrightness(int32_t value)
{
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Set brightness: %{public}d", value);
    value = value > MAX_BRIGHTNESS ? MAX_BRIGHTNESS : value;
    value = value < MIN_BRIGHTNESS ? MIN_BRIGHTNESS : value;
    bool isSucc = DisplayPowerMgrClient::GetInstance().SetBrightness(value);
    if (!isSucc) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Failed to set brightness: %{public}d", value);
    }
    return isSucc;
}

int32_t Brightness::BrightnessInfo::GetAutoMode() const
{
    bool isAuto = DisplayPowerMgrClient::GetInstance().IsAutoAdjustBrightness();
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Automatic brightness adjustment: %{public}d", isAuto);
    return static_cast<int32_t>(isAuto);
}

bool Brightness::BrightnessInfo::SetAutoMode(bool mode)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "AutoAdjustBrightness begin");
    bool isSucc = DisplayPowerMgrClient::GetInstance().AutoAdjustBrightness(mode);
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "set auto brightness mode: %{public}d, succ: %{public}d", mode, isSucc);
    return isSucc;
}

void Brightness::BrightnessInfo::ScreenOn(bool keep, std::shared_ptr<RunningLock>& runningLock)
{
    if (runningLock != nullptr) {
        DISPLAY_HILOGD(COMP_FWK, "Keep screen on, keep: %{public}d, isUsed: %{public}d", keep, runningLock->IsUsed());
        keep ? runningLock->Lock() : runningLock->UnLock();
    }
}

DisplayErrors Brightness::BrightnessInfo::GetServiceError() const
{
    return DisplayPowerMgrClient::GetInstance().GetError();
}

void Brightness::ExecuteCallback()
{
    bool error = result_.IsError();
    if (!error) {
        DISPLAY_HILOGI(COMP_FWK, "Call the js success method");
        napi_value result = result_.GetResult(env_);
        size_t argc = result ? MAX_ARGC : 0;
        CallFunction(successRef_, argc, result ? &result : nullptr);
    }

    if (error) {
        DISPLAY_HILOGI(COMP_FWK, "Call the js fail method");
        size_t argc = MAX_FAIL_ARGC;
        napi_value argv[argc];
        result_.GetError(env_, argv, argc);
        CallFunction(failRef_, argc, argv);
    }
    DISPLAY_HILOGI(COMP_FWK, "Call the js complete method");
    CallFunction(completeRef_, 0, nullptr);
}

bool Brightness::CheckValueType(napi_value& value, napi_valuetype checkType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env_, value, &valueType);
    if (valueType != checkType) {
        DISPLAY_HILOGW(COMP_FWK, "Check input parameter error");
        return false;
    }
    return true;
}

napi_value Brightness::GetOptions(napi_value& options, const std::string& name, napi_valuetype checkType)
{
    napi_value property = nullptr;
    napi_status status = napi_get_named_property(env_, options, name.c_str(), &property);
    if (status != napi_ok) {
        DISPLAY_HILOGW(COMP_FWK, "Failed to get the %{public}s Options property", name.c_str());
        return nullptr;
    }
    if (!CheckValueType(property, checkType)) {
        DISPLAY_HILOGW(COMP_FWK, "Get %{public}s Options property type mismatch", name.c_str());
        return nullptr;
    }
    return property;
}

void Brightness::CallFunction(napi_ref& callbackRef, size_t argc, napi_value* response)
{
    RETURN_IF(callbackRef == nullptr);

    napi_value callResult = 0;
    napi_value callback = nullptr;
    napi_get_reference_value(env_, callbackRef, &callback);
    napi_status status = napi_call_function(env_, nullptr, callback, argc, response, &callResult);
    if (status != napi_ok) {
        DISPLAY_HILOGW(COMP_FWK, "Failed to call the callback function");
    }
    napi_delete_reference(env_, callbackRef);
}
} // namespace DisplayPowerMgr
} // namespace OHOS