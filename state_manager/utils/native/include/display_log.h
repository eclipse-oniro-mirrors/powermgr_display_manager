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

#include <cstdint>
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
    COMP_FWK = 0,
    COMP_SVC = 1,
    COMP_UTS = 2,
    FEAT_BRIGHTNESS,
    FEAT_STATE,
    // Test label
    LABEL_TEST,
    // The end of labels, max to the domain id range length 32
    LABEL_END,
};

enum DisplayManagerLogDomain {
    DOMAIN_SERVICE = DISPLAY_DOMAIN_ID_START + COMP_SVC, // 0xD002981
    DOMAIN_FEAT_BRIGHTNESS, // 0xD002982
    DOMAIN_TEST = TEST_DOMAIN_ID, // 0xD000F00
    DOMAIN_END = DISPLAY_DOMAIN_ID_END, // Max to 0xD00299F, keep the sequence and length same as DisplayManagerLogLabel
};

struct DisplayManagerLogLabelTag {
    uint32_t logLabel;
    const char* tag;
};

// Keep the sequence same as DisplayManagerLogDomain
static constexpr DisplayManagerLogLabelTag DISPLAY_LABEL_TAG[LABEL_END] = {
    {COMP_FWK,          "DisplayFwk"},
    {COMP_SVC,          "DisplaySvc"},
    {COMP_UTS,          "DisplayUts"},
    {FEAT_BRIGHTNESS,   "DisplayBrightness"},
    {FEAT_STATE,        "DisplayState"},
    {LABEL_TEST,        "DisplayTest"},
};

struct DisplayManagerLogLabelDomain {
    uint32_t logLabel;
    uint32_t domainId;
};

// Keep the sequence same as DisplayManagerLogDomain
static constexpr DisplayManagerLogLabelDomain DISPLAY_LABEL_DOMAIN[LABEL_END] = {
    {COMP_FWK,          DOMAIN_SERVICE},
    {COMP_SVC,          DOMAIN_SERVICE},
    {COMP_UTS,          DOMAIN_SERVICE},
    {FEAT_BRIGHTNESS,   DOMAIN_FEAT_BRIGHTNESS},
    {FEAT_STATE,        DOMAIN_SERVICE},
    {LABEL_TEST,        DOMAIN_TEST},
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than LABEL_END.
#define DISPLAY_HILOGF(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, DISPLAY_LABEL_DOMAIN[domain].domainId, DISPLAY_LABEL_TAG[domain].tag,   \
    ##__VA_ARGS__))
#define DISPLAY_HILOGE(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, DISPLAY_LABEL_DOMAIN[domain].domainId, DISPLAY_LABEL_TAG[domain].tag,   \
    ##__VA_ARGS__))
#define DISPLAY_HILOGW(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, DISPLAY_LABEL_DOMAIN[domain].domainId, DISPLAY_LABEL_TAG[domain].tag,    \
    ##__VA_ARGS__))
#define DISPLAY_HILOGI(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, DISPLAY_LABEL_DOMAIN[domain].domainId, DISPLAY_LABEL_TAG[domain].tag,    \
    ##__VA_ARGS__))
#define DISPLAY_HILOGD(domain, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, DISPLAY_LABEL_DOMAIN[domain].domainId, DISPLAY_LABEL_TAG[domain].tag,   \
    ##__VA_ARGS__))
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
