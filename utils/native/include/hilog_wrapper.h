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

#ifndef DISPLAY_POWER_MGR_HILOG_WRAPPER_H
#define DISPLAY_POWER_MGR_HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace DisplayPowerMgr {
#define __FILENAME__            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__

#ifdef DISPLAY_HILOGF
#undef DISPLAY_HILOGF
#endif

#ifdef DISPLAY_HILOGE
#undef DISPLAY_HILOGE
#endif

#ifdef DISPLAY_HILOGW
#undef DISPLAY_HILOGW
#endif

#ifdef DISPLAY_HILOGI
#undef DISPLAY_HILOGI
#endif

#ifdef DISPLAY_HILOGD
#undef DISPLAY_HILOGD
#endif

// param of log interface, such as DISPLAY_HILOGF.
enum DisplayPowerMgrSubModule {
    MODULE_INNERKIT = 0,
    MODULE_SERVICE,
    MODULE_JAVAKIT, // java kit, defined to avoid repeated use of domain.
    MODULE_JNI,
    MODULE_COMMON,
    MODULE_JS_NAPI,
    DISPLAY_POWER_MGR_MODULE_BUTT,
};

// 0xD002900: subsystem:PowerMgr module:DisplayPowerMgr, 8 bits reserved.
static constexpr unsigned int BASE_DisplayPowerMgr_DOMAIN_ID = 0xD002910;

enum DisplayPowerMgrDomainId {
    DISPLAY_POWER_MGR_INNERKIT_DOMAIN = BASE_DisplayPowerMgr_DOMAIN_ID + MODULE_INNERKIT,
    DISPLAY_POWER_MGR_SERVICE_DOMAIN,
    DISPLAY_POWER_MGR_JAVAKIT_DOMAIN,
    COMMON_DOMAIN,
    DISPLAY_POWER_MGR_JS_NAPI,
    DISPLAY_POWER_MGR_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel DISPLAY_MGR_LABEL[DISPLAY_POWER_MGR_MODULE_BUTT] = {
    {LOG_CORE, DISPLAY_POWER_MGR_INNERKIT_DOMAIN, "DisplayPowerMgrClient"},
    {LOG_CORE, DISPLAY_POWER_MGR_SERVICE_DOMAIN, "DisplayPowerMgrService"},
    {LOG_CORE, DISPLAY_POWER_MGR_JAVAKIT_DOMAIN, "DisplayPowerMgrJavaService"},
    {LOG_CORE, DISPLAY_POWER_MGR_INNERKIT_DOMAIN, "DisplayPowerMgrJni"},
    {LOG_CORE, COMMON_DOMAIN, "DisplayPowerMgrCommon"},
    {LOG_CORE, DISPLAY_POWER_MGR_JS_NAPI, "DisplayPowerMgrJSNAPI"},
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than DisplayPowerMgr_MODULE_BUTT.
#define DISPLAY_HILOGF(m, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(DISPLAY_MGR_LABEL[m], __FORMATED(__VA_ARGS__))
#define DISPLAY_HILOGE(m, ...) (void)OHOS::HiviewDFX::HiLog::Error(DISPLAY_MGR_LABEL[m], __FORMATED(__VA_ARGS__))
#define DISPLAY_HILOGW(m, ...) (void)OHOS::HiviewDFX::HiLog::Warn(DISPLAY_MGR_LABEL[m], __FORMATED(__VA_ARGS__))
#define DISPLAY_HILOGI(m, ...) (void)OHOS::HiviewDFX::HiLog::Info(DISPLAY_MGR_LABEL[m], __FORMATED(__VA_ARGS__))
#define DISPLAY_HILOGD(m, ...) (void)OHOS::HiviewDFX::HiLog::Debug(DISPLAY_MGR_LABEL[m], __FORMATED(__VA_ARGS__))
} // namespace DisplayPowerMgr
} // namespace OHOS

#else

#define DISPLAY_HILOGF(...)
#define DISPLAY_HILOGE(...)
#define DISPLAY_HILOGW(...)
#define DISPLAY_HILOGI(...)
#define DISPLAY_HILOGD(...)

#endif // CONFIG_HILOG

#endif // DISPLAY_POWER_MGR_HILOG_WRAPPER_H
