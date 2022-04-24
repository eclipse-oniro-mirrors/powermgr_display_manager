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

const std::string BRIGHTNESS_VALUE = "value";
const std::string BRIGHTNESS_MODE = "mode";
const std::string KEEP_SCREENON = "keepScreenOn";

const std::string FUNC_SUCEESS_NAME = "success";
const std::string FUNC_FAIL_NAME = "fail";
const std::string FUNC_COMPLETE_NAME = "complete";

const int32_t COMMON_ERROR_COED = 200;
const int32_t INPUT_ERROR_CODE = 202;

const std::string SET_VALUE_ERROR_MGR = "setValue: value is not an available number";
const std::string GET_VALUE_ERROR_MGR = "get system screen brightness fail";
const std::string SET_MODE_ERROR_MGR = "setMode: value is not an available number";
const std::string SET_MODE_NOT_SUPPORTED_ERROR_MGR = "setMode: Auto adjusting brightness is not supported";
const std::string SET_KEEP_SCREENON_ERROR_MGR = "value is not an available boolean";
}

Brightness::Brightness(napi_env env) : env_(env)
{
}

void Brightness::GetValue(napi_value options)
{
    uint32_t brightness = brightnessInfo_.GetBrightness();
    if (BRIGHTNESS_OFF >= brightness || brightness > MAX_BRIGHTNESS) {
        result_.Error(COMMON_ERROR_COED, GET_VALUE_ERROR_MGR);
    } else {
        result_.SetResult(BRIGHTNESS_VALUE, brightness);
    }
    ExecuteCallback(options);
}

void Brightness::SetValue(napi_callback_info info)
{
    napi_value options = GetCallbackInfo(info, napi_object);
    if (options != nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "System brightness interface");
        napi_value value = GetOptions(options, BRIGHTNESS_VALUE, napi_number);
        if (value == nullptr) {
            result_.Error(INPUT_ERROR_CODE, SET_VALUE_ERROR_MGR);
        } else {
            int32_t brightness = MIN_BRIGHTNESS;
            napi_get_value_int32(env_, value, &brightness);
            brightnessInfo_.SetBrightness(brightness);
        }
        ExecuteCallback(options);
        return;
    }

    napi_value number = GetCallbackInfo(info, napi_number);
    if (number != nullptr) {
        DISPLAY_HILOGD(FEAT_BRIGHTNESS, "Brightness interface");
        int32_t value = MIN_BRIGHTNESS;
        if (napi_ok != napi_get_value_int32(env_, number, &value)) {
            DISPLAY_HILOGW(COMP_FWK, "Failed to get the input number");
            return;
        }
        brightnessInfo_.SetBrightness(value);
        return;
    }
    DISPLAY_HILOGW(FEAT_BRIGHTNESS, "SetValue: No valid parameters");
}

void Brightness::GetMode(napi_value options)
{
    int32_t isAuto = brightnessInfo_.GetAutoMode();
    result_.SetResult(BRIGHTNESS_MODE, isAuto);
    ExecuteCallback(options);
}

void Brightness::SetMode(napi_value options)
{
    DISPLAY_HILOGD(COMP_FWK, "Set auto brightness");
    napi_value napiMode = GetOptions(options, BRIGHTNESS_MODE, napi_number);
    if (napiMode == nullptr) {
        result_.Error(INPUT_ERROR_CODE, SET_MODE_ERROR_MGR);
    } else {
        int32_t mode = 0;
        napi_get_value_int32(env_, napiMode, &mode);
        if (!brightnessInfo_.SetAutoMode(static_cast<bool>(mode))) {
            result_.Error(COMMON_ERROR_COED, SET_MODE_NOT_SUPPORTED_ERROR_MGR);
        }
    }
    ExecuteCallback(options);
}

void Brightness::SetKeepScreenOn(napi_value options, std::shared_ptr<RunningLock>& runningLock)
{
    DISPLAY_HILOGD(COMP_FWK, "Set keep screen on");
    napi_value napiKeep = GetOptions(options, KEEP_SCREENON, napi_boolean);
    if (napiKeep == nullptr) {
        result_.Error(INPUT_ERROR_CODE, SET_KEEP_SCREENON_ERROR_MGR);
    } else {
        bool screenOn = false;
        napi_get_value_bool(env_, napiKeep, &screenOn);
        brightnessInfo_.ScreenOn(screenOn, runningLock);
    }
    ExecuteCallback(options);
}

napi_value Brightness::GetCallbackInfo(napi_callback_info info, napi_valuetype checkType)
{
    size_t argc = MAX_ARGC;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void *data = nullptr;
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

void Brightness::Result::Error(int32_t code, const std::string& msg)
{
    code_ = code;
    msg_ = msg;
    DISPLAY_HILOGW(COMP_FWK, "Error message, code: %{public}d, msg: %{public}s",
        code_, msg_.c_str());
}

void Brightness::Result::GetError(napi_env env, napi_value* error, size_t& size)
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

uint32_t Brightness::BrightnessInfo::GetBrightness()
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

int32_t Brightness::BrightnessInfo::GetAutoMode()
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
    DISPLAY_HILOGD(COMP_FWK, "Keep screen on, keep: %{public}d, isUsed: %{public}d", keep, runningLock->IsUsed());
    keep ? runningLock->Lock() : runningLock->UnLock();
}

void Brightness::ExecuteCallback(napi_value options)
{
    bool error = result_.IsError();
    if (!error) {
        DISPLAY_HILOGI(COMP_FWK, "Call the js success method");
        napi_value result = result_.GetResult(env_);
        size_t argc = result ? MAX_ARGC : 0;
        CallFunction(options, FUNC_SUCEESS_NAME, argc, result ? &result : nullptr);
    }

    if (error) {
        DISPLAY_HILOGI(COMP_FWK, "Call the js fail method");
        size_t argc = MAX_FAIL_ARGC;
        napi_value argv[argc];
        result_.GetError(env_, argv, argc);
        CallFunction(options, FUNC_FAIL_NAME, argc, argv);
    }
    DISPLAY_HILOGI(COMP_FWK, "Call the js complete method");
    CallFunction(options, FUNC_COMPLETE_NAME, 0, nullptr);
}

bool Brightness::CheckValueType(napi_value value, napi_valuetype checkType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env_, value, &valueType);
    if (valueType != checkType) {
        DISPLAY_HILOGW(COMP_FWK, "Check input parameter error");
        return false;
    }
    return true;
}

napi_value Brightness::GetOptions(napi_value options, const std::string& name, napi_valuetype checkType)
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

void Brightness::CallFunction(napi_value options, const std::string& name, size_t argc, napi_value* response)
{
    napi_value optionsFunc = GetOptions(options, name, napi_function);
    RETURN_IF(optionsFunc == nullptr);

    napi_value callResult = 0;
    napi_status status = napi_call_function(env_, nullptr, optionsFunc, argc, response, &callResult);
    if (status != napi_ok) {
        DISPLAY_HILOGW(COMP_FWK, "Failed to call the %{public}s callback function", name.c_str());
    }
}
} // namespace DisplayPowerMgr
} // namespace OHOS
