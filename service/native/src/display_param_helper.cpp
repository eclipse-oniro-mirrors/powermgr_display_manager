/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "display_param_helper.h"

#include <cstring>
#include "syspara/parameter.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
DisplayParamHelper::DisplayParamHelper() = default;
DisplayParamHelper::~DisplayParamHelper() = default;

uint32_t DisplayParamHelper::GetDefaultBrightness()
{
    int32_t value = QueryIntValue(KEY_DEFAULT_BRIGHTNESS, BRIGHTNESS_DEFAULT);
    return static_cast<uint32_t>(value);
}

uint32_t DisplayParamHelper::GetMaxBrightness()
{
    int32_t value = QueryIntValue(KEY_MAX_BRIGHTNESS, BRIGHTNESS_MAX);
    return static_cast<uint32_t>(value);
}

uint32_t DisplayParamHelper::GetMinBrightness()
{
    int32_t value = QueryIntValue(KEY_MIN_BRIGHTNESS, BRIGHTNESS_MIN);
    return static_cast<uint32_t>(value);
}

void DisplayParamHelper::RegisterBootCompletedCallback(BootCompletedCallback callback)
{
    int32_t ret = WatchParameter(KEY_BOOT_COMPLETED.c_str(), [](const char* key, const char* value, void* context) {
        if (strcmp(value, "true") == 0) {
            ((BootCompletedCallback) context)();
        }
    }, (void*) callback);
    if (ret < 0) {
        DISPLAY_HILOGW(COMP_SVC, "RegisterBootCompletedCallback failed, ret=%{public}d", ret);
    }
}

int32_t DisplayParamHelper::QueryIntValue(const std::string& key, int32_t def)
{
    char value[VALUE_MAX_LEN] = {0};
    int32_t ret = GetParameter(key.c_str(), std::to_string(def).c_str(), value, VALUE_MAX_LEN);
    if (ret < 0) {
        DISPLAY_HILOGW(COMP_SVC, "GetParameter failed, return default value, ret=%{public}d, def=%{public}d", ret, def);
        return def;
    }
    return atoi(value);
}
} // namespace DisplayPowerMgr
} // namespace OHOS