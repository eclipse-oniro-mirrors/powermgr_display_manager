/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <map>
#include "ohos.brightness.proj.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include "display_power_mgr_client.h"
#include "display_log.h"
#include "display_mgr_errors.h"
#include "ohos.brightness.impl.hpp"

using namespace taihe;
using namespace OHOS::DisplayPowerMgr;

namespace {
const uint32_t MAX_BRIGHTNESS = DisplayPowerMgrClient::GetInstance().GetMaxBrightness();
const uint32_t MIN_BRIGHTNESS = DisplayPowerMgrClient::GetInstance().GetMinBrightness();

std::map<DisplayErrors, std::string> errorTable = {
    {DisplayErrors::ERR_CONNECTION_FAIL,   "Failed to connect to the service."},
    {DisplayErrors::ERR_PERMISSION_DENIED, "Permission is denied"             },
    {DisplayErrors::ERR_SYSTEM_API_DENIED, "System permission is denied"      },
    {DisplayErrors::ERR_PARAM_INVALID,     "Invalid input parameter."         }
};

void SetValueContinuous(int32_t value, bool continuous)
{
    DISPLAY_HILOGD(FEAT_BRIGHTNESS, "ets brightness interface");
    DISPLAY_HILOGI(FEAT_BRIGHTNESS, "Set brightness: %{public}d, %{public}d", value, continuous);
    value = value > static_cast<int32_t>(MAX_BRIGHTNESS) ? static_cast<int32_t>(MAX_BRIGHTNESS) : value;
    value = value < static_cast<int32_t>(MIN_BRIGHTNESS) ? static_cast<int32_t>(MIN_BRIGHTNESS) : value;
    bool isSucc = DisplayPowerMgrClient::GetInstance().SetBrightness(value, 0, continuous);
    if (!isSucc) {
        DISPLAY_HILOGW(FEAT_BRIGHTNESS, "Failed to set brightness: %{public}d", value);
        DisplayErrors error = DisplayPowerMgrClient::GetInstance().GetError();
        if (error != DisplayErrors::ERR_OK) {
            taihe::set_business_error(static_cast<int32_t>(error), errorTable[error]);
        }
    }
}

void SetValueInt(int32_t value)
{
    bool continuous = false;
    SetValueContinuous(value, continuous);
}
}  // namespace

TH_EXPORT_CPP_API_SetValueContinuous(SetValueContinuous);
TH_EXPORT_CPP_API_SetValueInt(SetValueInt);
