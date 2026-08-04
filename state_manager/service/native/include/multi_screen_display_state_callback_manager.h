/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DISPLAYMGR_MULTI_SCREEN_DISPLAY_STATE_CALLBACK_MANAGER_H
#define DISPLAYMGR_MULTI_SCREEN_DISPLAY_STATE_CALLBACK_MANAGER_H

#include <cstdint>
#include <map>
#include <mutex>
#include <utility>
#include <vector>

#include <iremote_object.h>

#include "display_log.h"
#include "display_power_info.h"
#include "ffrt_utils.h"
#include "imulti_screen_display_state_callback.h"

namespace OHOS {
namespace DisplayPowerMgr {

class MultiScreenDisplayStateCallbackManager {
public:
    MultiScreenDisplayStateCallbackManager() = default;
    ~MultiScreenDisplayStateCallbackManager() = default;

    bool Register(const sptr<IRemoteObject>& callback, uint64_t screenId);
    bool Unregister(const sptr<IRemoteObject>& callback, uint64_t screenId);
    bool RemoveAll(const sptr<IRemoteObject>& callback);
    void Notify(uint64_t screenId, const std::string& screenName, DisplayState state, uint32_t reason);
    void PublishCommonEvent(uint64_t screenId, const std::string& screenName, DisplayState state, uint32_t reason);

private:
    class CallbackDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit CallbackDeathRecipient(MultiScreenDisplayStateCallbackManager& manager) : manager_(manager) {}
        ~CallbackDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override
        {
            DISPLAY_HILOGW(COMP_SVC, "CallbackDeathRecipient OnRemoteDied");
            auto object = remote.promote();
            if (object != nullptr) {
                manager_.RemoveAll(object);
            }
        }

    private:
        MultiScreenDisplayStateCallbackManager& manager_;
    };

    std::multimap<sptr<IRemoteObject>, uint64_t> callbacks_;
    std::map<sptr<IRemoteObject>, sptr<CallbackDeathRecipient>> deathRecipients_;
    std::map<sptr<IRemoteObject>, std::pair<int32_t, int32_t>> callbackPidUidMap_;
    ffrt::mutex mutex_;  // Protects callbacks_, deathRecipients_ and callbackPidUidMap_
};

} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_MULTI_SCREEN_DISPLAY_STATE_CALLBACK_MANAGER_H
