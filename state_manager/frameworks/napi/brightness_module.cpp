/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <string>
#include <iosfwd>
#include <memory>

#include <napi/native_api.h>
#include <napi/native_common.h>

#include "new"
#include "js_native_api.h"
#include "js_native_api_types.h"

#include "power_state_machine_info.h"
#include "brightness.h"
#include "display_common.h"
#include "display_log.h"
#include "power_mgr_client.h"
#include "xpower_event_js.h"

using namespace OHOS::DisplayPowerMgr;
using namespace OHOS::PowerMgr;
namespace {
std::shared_ptr<RunningLock> runningLock =
    PowerMgrClient::GetInstance().CreateRunningLock(std::string("KeepScreenOn"), RunningLockType::RUNNINGLOCK_SCREEN);
}

typedef void (*brightness_callback)(void* data);

static void SyncWorkSendEvent(napi_env env, Brightness *asyncContext,
    brightness_callback complete, napi_event_priority prio)
{
    auto task = [asyncContext, complete]() {
        DISPLAY_HILOGD(COMP_FWK, "CompleteCallback In");
        if (asyncContext == nullptr) {
            DISPLAY_HILOGD(COMP_FWK, "asyncContext is nullptr");
            return;
        }
        complete(asyncContext);
    };
    if (napi_send_event(env, task, prio) != napi_status::napi_ok) {
        DISPLAY_HILOGE(COMP_FWK, "failed to SendEvent!");
        delete asyncContext;
    }
}

static napi_value SyncWork(napi_env env, const std::string resName, const std::string valName,
    napi_callback_info& info, brightness_callback complete)
{
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    std::unique_ptr<Brightness> asyncBrightness = std::make_unique<Brightness>(env);
    RETURN_IF_WITH_RET(asyncBrightness == nullptr, undefined);
    napi_value options = asyncBrightness->GetCallbackInfo(info, 0, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, undefined);
    RETURN_IF_WITH_RET(!asyncBrightness->CreateCallbackRef(options), undefined);
    if (!valName.empty()) {
        asyncBrightness->CreateValueRef(options, valName, napi_number);
    }
    SyncWorkSendEvent(env, asyncBrightness.get(), complete, napi_eprio_low);
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
        [](void *data) {
            Brightness *asyncBrightness = reinterpret_cast<Brightness*>(data);
            if (asyncBrightness != nullptr) {
                asyncBrightness->GetValue();
                delete asyncBrightness;
            }
        }
    );
}

static napi_value SetValue(napi_env env, napi_callback_info info)
{
    napi_value res = SyncWork(
        env,
        "SetValue",
        Brightness::BRIGHTNESS_VALUE,
        info,
        [](void *data) {
            Brightness *asyncBrightness = reinterpret_cast<Brightness*>(data);
            if (asyncBrightness != nullptr) {
                asyncBrightness->SystemSetValue();
                delete asyncBrightness;
            }
        }
    );
    if (res != nullptr) {
        Brightness brightness(env);
        brightness.SetValue(info);
        OHOS::HiviewDFX::ReportXPowerJsStackSysEvent(env, "Brightness::SetValue");
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
        [](void *data) {
            Brightness *asyncBrightness = reinterpret_cast<Brightness*>(data);
            if (asyncBrightness != nullptr) {
                asyncBrightness->GetMode();
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
        [](void *data) {
            Brightness *asyncBrightness = reinterpret_cast<Brightness*>(data);
            if (asyncBrightness != nullptr) {
                asyncBrightness->SetMode();
                delete asyncBrightness;
            }
        }
    );
}

static void SetKeepScreenOnSendEvent(napi_env env, Brightness *asyncContext, napi_event_priority prio)
{
    auto task = [env, asyncContext]() {
        DISPLAY_HILOGD(COMP_FWK, "CompleteCallback In");
        if (asyncContext == nullptr) {
            DISPLAY_HILOGD(COMP_FWK, "asyncContext is nullptr");
            return;
        }
        asyncContext->SetKeepScreenOn();
        delete asyncContext;
    };
    if (napi_send_event(env, task, prio) != napi_status::napi_ok) {
        DISPLAY_HILOGE(COMP_FWK, "failed to SendEvent!");
        delete asyncContext;
    }
}

static napi_value SetKeepScreenOn(napi_env env, napi_callback_info info)
{
    std::unique_ptr<Brightness> asyncBrightness = std::make_unique<Brightness>(env, runningLock);
    RETURN_IF_WITH_RET(asyncBrightness == nullptr, nullptr);
    napi_value options = asyncBrightness->GetCallbackInfo(info, 0, napi_object);
    RETURN_IF_WITH_RET(options == nullptr, nullptr);
    RETURN_IF_WITH_RET(!asyncBrightness->CreateCallbackRef(options), nullptr);
    asyncBrightness->CreateValueRef(options, Brightness::KEEP_SCREENON, napi_boolean);
    SetKeepScreenOnSendEvent(env, asyncBrightness.get(), napi_eprio_low);
    asyncBrightness.release();
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    DISPLAY_HILOGD(COMP_FWK, "brightness init");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getValue", GetValue),
        DECLARE_NAPI_FUNCTION("setValue", SetValue),
        DECLARE_NAPI_FUNCTION("getMode", GetMode),
        DECLARE_NAPI_FUNCTION("setMode", SetMode),
        DECLARE_NAPI_FUNCTION("setKeepScreenOn", SetKeepScreenOn)
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    DISPLAY_HILOGD(COMP_FWK, "brightness init end");
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
