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

#ifndef DISPLAY_LOG_H
#define DISPLAY_LOG_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG

#include "hilog/log.h"

namespace OHOS {
namespace DisplayPowerMgr  {

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

namespace {
// Display manager reserved domain id range
constexpr unsigned int DISPLAY_DOMAIN_ID_START = 0xD002980;
constexpr unsigned int DISPLAY_DOMAIN_ID_END = DISPLAY_DOMAIN_ID_START + 32;
constexpr unsigned int TEST_DOMAIN_ID = 0xD000F00;
}

enum DisplayManagerLogLabel {
    // Component labels, you can add if needed
    COMP_APP = 0,
    COMP_FWK = 1,
    COMP_SVC = 2,
    COMP_HDI = 3,
    COMP_DRV = 4,
    COMP_UTS = 5,
    FEAT_BRIGHTNESS,
    FEAT_STATE,
    // Test label
    LABEL_TEST,
    // The end of labels, max to the domain id range length 32
    LABEL_END,
};

enum DisplayManagerLogDomain {
    DOMAIN_APP = DISPLAY_DOMAIN_ID_START + COMP_APP, // 0xD002980
    DOMAIN_FRAMEWORK, // 0xD002981
    DOMAIN_SERVICE, // 0xD002982
    DOMAIN_HDI, // 0xD002983
    DOMAIN_DRIVER, // 0xD002984
    DOMAIN_UTILS, // 0xD002985
    DOMAIN_FEAT_BRIGHTNESS,
    DOMAIN_FEAT_STATE,
    DOMAIN_TEST = TEST_DOMAIN_ID, // 0xD000F00
    DOMAIN_END = DISPLAY_DOMAIN_ID_END, // Max to 0xD00299F, keep the sequence and length same as DisplayManagerLogLabel
};

struct DisplayManagerLogLabelDomain {
    uint32_t domainId;
    const char* tag;
};

// Keep the sequence and length same as DisplayManagerLogDomain
static const DisplayManagerLogLabelDomain DISPLAY_LABEL[LABEL_END] = {
    {DOMAIN_APP,               "DisplayPowerApp"},
    {DOMAIN_FRAMEWORK,         "DisplayPowerFwk"},
    {DOMAIN_SERVICE,           "DisplayPowerSvc"},
    {DOMAIN_HDI,               "DisplayPowerHdi"},
    {DOMAIN_DRIVER,            "DisplayPowerDrv"},
    {DOMAIN_UTILS,             "DisplayPowerUts"},
    {DOMAIN_FEAT_BRIGHTNESS,   "DisplayPowerBrightness"},
    {DOMAIN_FEAT_STATE,        "DisplayPowerState"},
    {DOMAIN_TEST,              "DisplayPowerTest"},
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than LABEL_END.
#define DISPLAY_HILOGF(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, DISPLAY_LABEL[domain].domainId, DISPLAY_LABEL[domain].tag, ##__VA_ARGS__))
#define DISPLAY_HILOGE(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, DISPLAY_LABEL[domain].domainId, DISPLAY_LABEL[domain].tag, ##__VA_ARGS__))
#define DISPLAY_HILOGW(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, DISPLAY_LABEL[domain].domainId, DISPLAY_LABEL[domain].tag, ##__VA_ARGS__))
#define DISPLAY_HILOGI(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, DISPLAY_LABEL[domain].domainId, DISPLAY_LABEL[domain].tag, ##__VA_ARGS__))
#define DISPLAY_HILOGD(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, DISPLAY_LABEL[domain].domainId, DISPLAY_LABEL[domain].tag, ##__VA_ARGS__))
} // namespace DisplayPowerMgr
} // namespace OHOS

#else

#define DISPLAY_HILOGF(...)
#define DISPLAY_HILOGE(...)
#define DISPLAY_HILOGW(...)
#define DISPLAY_HILOGI(...)
#define DISPLAY_HILOGD(...)

#endif // CONFIG_HILOG

#endif // DISPLAY_LOG_H
