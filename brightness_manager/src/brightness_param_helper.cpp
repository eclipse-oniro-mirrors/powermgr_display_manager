/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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

#include "brightness_param_helper.h"

#include "sysparam.h"

namespace OHOS {
namespace DisplayPowerMgr {
using namespace OHOS::PowerMgr;
uint32_t BrightnessParamHelper::GetDefaultBrightness()
{
    int32_t value = SysParam::GetIntValue(KEY_DEFAULT_BRIGHTNESS, BRIGHTNESS_DEFAULT);
    return static_cast<uint32_t>(value);
}

uint32_t BrightnessParamHelper::GetMaxBrightness()
{
    int32_t value = SysParam::GetIntValue(KEY_MAX_BRIGHTNESS, BRIGHTNESS_MAX);
    return static_cast<uint32_t>(value);
}

uint32_t BrightnessParamHelper::GetMinBrightness()
{
    int32_t value = SysParam::GetIntValue(KEY_MIN_BRIGHTNESS, BRIGHTNESS_MIN);
    return static_cast<uint32_t>(value);
}

void BrightnessParamHelper::RegisterBootCompletedCallback(BootCompletedCallback& callback)
{
    SysParam::RegisterBootCompletedCallback(callback);
}
} // namespace DisplayPowerMgr
} // namespace OHOS