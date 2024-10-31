/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DISPLAY_COMMON_EVENT_MGR_H
#define DISPLAY_COMMON_EVENT_MGR_H

#include "common_event_manager.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"
#include "datashare_helper.h"
#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayCommonEventManager : public OHOS::EventFwk::CommonEventSubscriber {
public:
    explicit DisplayCommonEventManager(const OHOS::EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    virtual ~DisplayCommonEventManager() = default;
    DisplayCommonEventManager(const DisplayCommonEventManager&) = delete;
    DisplayCommonEventManager& operator=(const DisplayCommonEventManager&) = delete;
    DisplayCommonEventManager(DisplayCommonEventManager&&) = delete;
    DisplayCommonEventManager& operator=(DisplayCommonEventManager&&) = delete;

    void OnReceiveEvent(const OHOS::EventFwk::CommonEventData &data) override;
    bool CheckIfSettingsDataReady();
    bool SetKvDataReady();
    void RegisterSettingObservers();

private:
    bool isDataShareReady_ {false};
    bool isKvDataReady_ {false};
    sptr<IRemoteObject> remoteObj_;
    std::mutex lock_ {};
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif