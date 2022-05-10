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

#include "display_event_handler.h"

#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {
DisplayEventHandler::DisplayEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
    const wptr<DisplayPowerMgrService>& parent) : AppExecFwk::EventHandler(runner), parent_(parent)
{
    DISPLAY_HILOGD(COMP_SVC, "Instance created");
}

void DisplayEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    uint32_t eventId = event->GetInnerEventId();
    auto callback = eventCallbackMap_.find(eventId);
    if (callback != eventCallbackMap_.end()) {
        DISPLAY_HILOGD(COMP_SVC, "Perform the display handler event callback, eventId: %{public}d", eventId);
        callback->second();
        return;
    }
    DISPLAY_HILOGI(COMP_SVC, "Start to process, eventId: %{public}d", eventId);
}

void DisplayEventHandler::EmplaceCallBack(DisplayEventHandler::Event event, EventCallback& callback)
{
    uint32_t eventId = static_cast<uint32_t>(event);
    DISPLAY_HILOGI(COMP_SVC, "Register handler events, eventId: %{public}d", eventId);
    if (eventCallbackMap_.find(eventId) != eventCallbackMap_.end()) {
        DISPLAY_HILOGW(COMP_SVC, "Already registered, ignore registration eventId: %{public}d", eventId);
        return;
    }
    eventCallbackMap_.emplace(eventId, callback);
}
} // namespace DisplayPowerMgr
} // namespace OHOS