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

#include "display_power_mgr_service.h"

#include <file_ex.h>
#include <string_ex.h>

namespace OHOS {
namespace DisplayPowerMgr {
DisplayPowerMgrService::DisplayPowerMgrService()
{
    std::shared_ptr<ScreenAction> screenAction = std::make_shared<ScreenAction>();
    std::vector<uint32_t> devIds = screenAction->GetDisplayIds();
    int count = devIds.size();
    for (int i = 0; i < count; i++) {
        controllerMap_.emplace(devIds[i], std::make_shared<ScreenController>(devIds[i], screenAction));
    }
}

bool DisplayPowerMgrService::SetDisplayState(uint32_t id, DisplayState state)
{
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->UpdateState(state);
}

DisplayState DisplayPowerMgrService::GetDisplayState(uint32_t id)
{
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return DisplayState::DISPLAY_UNKNOWN;
    }
    return iterater->second->GetState();
}

bool DisplayPowerMgrService::SetBrightness(uint32_t id, int32_t value)
{
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->UpdateBrightness(value);
}

bool DisplayPowerMgrService::AdjustBrightness(uint32_t id, int32_t value, uint32_t duration)
{
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->UpdateBrightness(value, duration);
}

bool DisplayPowerMgrService::SetStateConfig(uint32_t id, DisplayState state, int32_t value)
{
    auto iterater = controllerMap_.find(id);
    if (iterater == controllerMap_.end()) {
        return false;
    }
    return iterater->second->UpdateStateConfig(state, value);
}

int32_t DisplayPowerMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::string result("Empty dump info");
    if (!SaveStringToFd(fd, result)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to save dump info to fd");
    }
    return ERR_OK;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
