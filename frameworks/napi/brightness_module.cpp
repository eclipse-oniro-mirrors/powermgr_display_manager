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

static napi_value SyncWork(napi_env env, const std::string resName, const std::string valName,
    napi_callback_info& info, napi_async_complete_callback complete)
{
    std::unique_ptr<Brightness> asyncBrightness = std::make_unique<Brightness>(env);
    RETURN_IF_WITH_RET(asyncBrightness == nullptr, nullptr);
    napi_value options = asyncBrightness->GetCallbackInfo(info, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    RETURN_IF_WITH_RET(!asyncBrightness->CreateCallbackRef(options), nullptr);
    if (!valName.empty()) {
        asyncBrightness->CreateValueRef(options, valName, napi_number);
    }

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, resName.c_str(), NAPI_AUTO_LENGTH, &resource));
    napi_create_async_work(
        env,
        nullptr,
        resource,
        [](napi_env env, void *data) {},
        complete,
        (void*)asyncBrightness.get(),
        &asyncBrightness->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncBrightness->asyncWork));
    asyncBrightness.release();

    return nullptr;
}

static napi_value GetValue(napi_env env, napi_callback_info info)
{
    return SyncWork(
        env,
        "GetValue",
        "",
        info,
        [](napi_env env, napi_status status, void *data) {
            Brightness *asyncBrightness = (Brightness*)data;
            if (asyncBrightness != nullptr) {
                asyncBrightness->GetValue();
                napi_delete_async_work(env, asyncBrightness->asyncWork);
                delete asyncBrightness;
            }
        }
    );
}

static napi_value SetValue(napi_env env, napi_callback_info info)
{
    SyncWork(
        env,
        "SetValue",
        Brightness::BRIGHTNESS_VALUE,
        info,
        [](napi_env env, napi_status status, void *data) {
            Brightness *asyncBrightness = (Brightness*)data;
            if (asyncBrightness != nullptr) {
                asyncBrightness->SystemSetValue();
                napi_delete_async_work(env, asyncBrightness->asyncWork);
                delete asyncBrightness;
            }
        }
    );

    std::unique_ptr<Brightness> asyncBrightness = std::make_unique<Brightness>(env);
    napi_value number = asyncBrightness->GetCallbackInfo(info, napi_number);
    if (number != nullptr) {
        Brightness brightness(env);
        brightness.SetValue(number);
    }
    return nullptr;
}

static napi_value GetMode(napi_env env, napi_callback_info info)
{
    return SyncWork(
        env,
        "GetMode",
        "",
        info,
        [](napi_env env, napi_status status, void *data) {
            Brightness *asyncBrightness = (Brightness*)data;
            if (asyncBrightness != nullptr) {
                asyncBrightness->GetMode();
                napi_delete_async_work(env, asyncBrightness->asyncWork);
                delete asyncBrightness;
            }
        }
    );
}

static napi_value SetMode(napi_env env, napi_callback_info info)
{
    return SyncWork(
        env,
        "SetMode",
        Brightness::BRIGHTNESS_MODE,
        info,
        [](napi_env env, napi_status status, void *data) {
            Brightness *asyncBrightness = (Brightness*)data;
            if (asyncBrightness != nullptr) {
                asyncBrightness->SetMode();
                napi_delete_async_work(env, asyncBrightness->asyncWork);
                delete asyncBrightness;
            }
        }
    );
}

static napi_value SetKeepScreenOn(napi_env env, napi_callback_info info)
{
    std::unique_ptr<Brightness> asyncBrightness = std::make_unique<Brightness>(env, runningLock);
    RETURN_IF_WITH_RET(asyncBrightness == nullptr, nullptr);
    napi_value options = asyncBrightness->GetCallbackInfo(info, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    RETURN_IF_WITH_RET(!asyncBrightness->CreateCallbackRef(options), nullptr);
    asyncBrightness->CreateValueRef(options, Brightness::KEEP_SCREENON, napi_boolean);

    napi_value resource = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, "SetKeepScreenOn", NAPI_AUTO_LENGTH, &resource));
    napi_create_async_work(
        env,
        nullptr,
        resource,
        [](napi_env env, void *data) {},
        [](napi_env env, napi_status status, void *data) {
            Brightness *asyncBrightness = (Brightness*)data;
            if (asyncBrightness != nullptr) {
                asyncBrightness->SetKeepScreenOn();
                napi_delete_async_work(env, asyncBrightness->asyncWork);
                delete asyncBrightness;
            }
        },
        (void*)asyncBrightness.get(),
        &asyncBrightness->asyncWork);
    NAPI_CALL(env, napi_queue_async_work(env, asyncBrightness->asyncWork));
    asyncBrightness.release();

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
