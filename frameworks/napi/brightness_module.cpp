/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <string>

#include <napi/native_api.h>
#include <napi/native_node_api.h>

#include "brightness.h"
#include "display_common.h"
#include "hilog_wrapper.h"
#include "power_mgr_client.h"
#include "running_lock_info.h"

using namespace OHOS::DisplayPowerMgr;
using namespace OHOS::PowerMgr;
namespace {
std::shared_ptr<RunningLock> runningLock =
    PowerMgrClient::GetInstance().CreateRunningLock(std::string("KeepScreenOn"), RunningLockType::RUNNINGLOCK_SCREEN);
}

static napi_value GetValue(napi_env env, napi_callback_info info)
{
    Brightness brightness(env);
    napi_value options = brightness.GetCallbackInfo(info, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    brightness.GetValue(options);
    return nullptr;
}

static napi_value SetValue(napi_env env, napi_callback_info info)
{
    Brightness brightness(env);
    brightness.SetValue(info);
    return nullptr;
}

static napi_value GetMode(napi_env env, napi_callback_info info)
{
    Brightness brightness(env);
    napi_value options = brightness.GetCallbackInfo(info, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    brightness.GetMode(options);
    return nullptr;
}

static napi_value SetMode(napi_env env, napi_callback_info info)
{
    Brightness brightness(env);
    napi_value options = brightness.GetCallbackInfo(info, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    brightness.SetMode(options);
    return nullptr;
}

static napi_value SetKeepScreenOn(napi_env env, napi_callback_info info)
{
    Brightness brightness(env);
    napi_value options = brightness.GetCallbackInfo(info, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    brightness.SetKeepScreenOn(options, runningLock);
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    DISPLAY_HILOGI(MODULE_JNI, "brightness init");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getValue", GetValue),
        DECLARE_NAPI_FUNCTION("setValue", SetValue),
        DECLARE_NAPI_FUNCTION("getMode", GetMode),
        DECLARE_NAPI_FUNCTION("setMode", SetMode),
        DECLARE_NAPI_FUNCTION("setKeepScreenOn", SetKeepScreenOn)
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    DISPLAY_HILOGI(MODULE_JNI, "brightness init end");
    return exports;
}
EXTERN_C_END

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "brightness",
    .nm_register_func = Init,
    .nm_modname = "brightness",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
