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

#include "multi_screen_display_state_callback_manager.h"

#include <cinttypes>

#include <datetime_ex.h>
#include <ipc_skeleton.h>

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_publish_info.h"
#include "display_common.h"
#include "display_log.h"
#include "want.h"

namespace OHOS {
namespace DisplayPowerMgr {
namespace {
constexpr const char* PARAM_SCREEN_ID = "screenId";
constexpr const char* PARAM_SCREEN_NAME = "screenName";
constexpr const char* PARAM_REASON = "reason";
constexpr const char* MULTI_SCREEN_ON_ACTION = "usual.event.display.MULTI_SCREEN_ON";
constexpr const char* MULTI_SCREEN_OFF_ACTION = "usual.event.display.MULTI_SCREEN_OFF";
constexpr const char* MULTI_SCREEN_PERMISSION = "ohos.permission.MULTI_SCREEN_MANAGER";
}

bool MultiScreenDisplayStateCallbackManager::Register(const sptr<IRemoteObject>& callback, uint64_t screenId)
{
    if (callback == nullptr) {
        DISPLAY_HILOGE(COMP_SVC, "Register callback is nullptr");
        return false;
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto range = callbacks_.equal_range(callback);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second == screenId) {
            DISPLAY_HILOGI(COMP_SVC, "already registered for screenId=%{public}" PRIu64, screenId);
            return true;
        }
    }
    if (range.first == range.second) {
        if (!callback->IsProxyObject()) {
            DISPLAY_HILOGE(COMP_SVC, "callback is not proxy");
            return false;
        }
        sptr<CallbackDeathRecipient> deathRecipient(new CallbackDeathRecipient(*this));
        callback->AddDeathRecipient(deathRecipient);
        deathRecipients_[callback] = deathRecipient;
        callbackPidUidMap_.emplace(callback,
            std::make_pair(IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid()));
    }
    callbacks_.emplace(callback, screenId);
    DISPLAY_HILOGI(COMP_SVC, "callback for screenId=%{public}" PRIu64 ", total=%{public}zu",
        screenId, callbacks_.size());
    return true;
}

bool MultiScreenDisplayStateCallbackManager::Unregister(const sptr<IRemoteObject>& callback, uint64_t screenId)
{
    if (callback == nullptr) {
        DISPLAY_HILOGE(COMP_SVC, "Unregister callback is nullptr");
        return false;
    }
    if (screenId == SCREEN_ID_ALL) {
        return RemoveAll(callback);
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    auto range = callbacks_.equal_range(callback);
    for (auto it = range.first; it != range.second; ++it) {
        if (it->second != screenId) {
            continue;
        }
        callbacks_.erase(it);
        auto remaining = callbacks_.equal_range(callback);
        if (remaining.first == remaining.second) {
            auto drIt = deathRecipients_.find(callback);
            if (drIt != deathRecipients_.end()) {
                callback->RemoveDeathRecipient(drIt->second);
                deathRecipients_.erase(drIt);
            }
            callbackPidUidMap_.erase(callback);
        }
        return true;
    }
    DISPLAY_HILOGW(COMP_SVC, "callback not found for screenId=%{public}" PRIu64, screenId);
    return true;
}

bool MultiScreenDisplayStateCallbackManager::RemoveAll(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        return false;
    }
    std::lock_guard<ffrt::mutex> lock(mutex_);
    size_t count = callbacks_.erase(callback);
    if (count > 0) {
        auto drIt = deathRecipients_.find(callback);
        if (drIt != deathRecipients_.end()) {
            callback->RemoveDeathRecipient(drIt->second);
            deathRecipients_.erase(drIt);
        }
        callbackPidUidMap_.erase(callback);
        DISPLAY_HILOGI(COMP_SVC, "RemoveAll removed %{public}zu callbacks", count);
    } else {
        DISPLAY_HILOGI(COMP_SVC, "callback not registered");
    }
    return true;
}

void MultiScreenDisplayStateCallbackManager::Notify(uint64_t screenId, const std::string& screenName,
    DisplayState state, uint32_t reason)
{
    DISPLAY_HILOGI(COMP_SVC,
        "Notify screenId=%{public}" PRIu64 ", screenName=%{public}s, state=%{public}u, reason=%{public}u",
        screenId, screenName.c_str(), static_cast<uint32_t>(state), reason);
    std::vector<std::pair<sptr<IRemoteObject>, uint64_t>> callbacks;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        callbacks.assign(callbacks_.begin(), callbacks_.end());
    }
    for (const auto& entry : callbacks) {
        uint64_t registeredId = entry.second;
        if (registeredId == screenId || registeredId == SCREEN_ID_ALL) {
            auto proxy = iface_cast<IMultiScreenDisplayStateCallback>(entry.first);
            if (proxy != nullptr) {
                auto pidUidIt = callbackPidUidMap_.find(entry.first);
                auto pidUid = (pidUidIt != callbackPidUidMap_.end()) ? pidUidIt->second : std::make_pair(0, 0);
                DISPLAY_HILOGI(COMP_SVC,
                    "MultiScreenCallback begin Pid=%{public}d Uid=%{public}d screenId=%{public}" PRIu64,
                    pidUid.first, pidUid.second, screenId);
                int64_t start = GetTickCount();
                proxy->OnMultiScreenDisplayStateChanged(screenId, screenName, state,
                    static_cast<MultiScreenStateChangeReason>(reason));
                int64_t cost = GetTickCount() - start;
                DISPLAY_HILOGI(COMP_SVC,
                    "MultiScreenCallback end Pid=%{public}d Uid=%{public}d costTime=%{public}" PRId64,
                    pidUid.first, pidUid.second, cost);
            }
        }
    }
}

void MultiScreenDisplayStateCallbackManager::PublishCommonEvent(uint64_t screenId, const std::string& screenName,
    DisplayState state, uint32_t reason)
{
    const char* action = (state == DisplayState::DISPLAY_ON) ? MULTI_SCREEN_ON_ACTION : MULTI_SCREEN_OFF_ACTION;
    AAFwk::Want want;
    want.SetAction(action);
    want.SetParam(PARAM_SCREEN_ID, static_cast<long long>(screenId));
    want.SetParam(PARAM_SCREEN_NAME, screenName);
    want.SetParam(PARAM_REASON, GetReasonString(reason));
    EventFwk::CommonEventData data;
    data.SetWant(want);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(false);
    const std::vector<std::string> permissionVec { MULTI_SCREEN_PERMISSION };
    publishInfo.SetSubscriberPermissions(permissionVec);
    bool ret = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo);
    DISPLAY_HILOGI(COMP_SVC,
        "PublishCommonEvent for %{public}s, screenId=%{public}" PRIu64 ", screenName=%{public}s,"
        " reason=%{public}u, ret=%{public}d",
        action, screenId, screenName.c_str(), reason, ret);
}
} // namespace DisplayPowerMgr
} // namespace OHOS
