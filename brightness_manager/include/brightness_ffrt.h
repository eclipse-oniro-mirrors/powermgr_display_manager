/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef BRIGHTNESS_FFRT_H
#define BRIGHTNESS_FFRT_H

#include "display_log.h"
#include "ffrt_utils.h"

namespace OHOS {
namespace DisplayPowerMgr {

inline void FFRT_CANCEL(PowerMgr::FFRTHandle& handle, std::shared_ptr<PowerMgr::FFRTQueue>& queue)
{
    if (!(handle) || !(queue)) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "FFRT_CANCEL, queue or handle is null");
        return;
    }
    int ret = PowerMgr::FFRTUtils::CancelTask(handle, queue);
    if (ret != 0) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "FFRT_CANCEL, cancel fail: %{public}d", ret);
    } else {
        handle = nullptr;
    }
}
} // DisplayPowerMgr
} // OHOS

#endif