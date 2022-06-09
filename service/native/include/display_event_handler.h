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

#ifndef DISPLAYMGR_DISPLAY_EVENT_HANDLER_H
#define DISPLAYMGR_DISPLAY_EVENT_HANDLER_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <event_handler.h>
#include <event_runner.h>
#include <refbase.h>
#include "inner_event.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrService;
class DisplayEventHandler : public AppExecFwk::EventHandler {
public:
    typedef std::function<void()> EventCallback;
    enum Event {
        EVENT_CANCEL_BOOST_BRIGHTNESS,
        EVENT_SET_SETTING_BRIGHTNESS,
    };
    DisplayEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner, 
        const wptr<DisplayPowerMgrService>& parent);
    ~DisplayEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    void EmplaceCallBack(DisplayEventHandler::Event event, EventCallback& callback);

private:
    wptr<DisplayPowerMgrService> parent_;
    std::map<uint32_t, EventCallback> eventCallbackMap_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_EVENT_HANDLER_H