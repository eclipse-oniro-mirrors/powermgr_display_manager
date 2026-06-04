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

#ifdef ENABLE_PER_SCREEN_POWER

#include "screen_power_adapter.h"

namespace OHOS {
namespace DisplayPowerMgr {

ScreenPowerAdapter& ScreenPowerAdapter::GetInstance()
{
    static ScreenPowerAdapter instance;
    return instance;
}

// ===== window_manager per-displayId APIs (MOCK) =====

bool ScreenPowerAdapter::WakeUpBegin(uint64_t displayId, Rosen::PowerStateChangeReason reason)
{
    DISPLAY_HILOGI(COMP_SVC,
        "ScreenPowerAdapter::WakeUpBegin MOCK, displayId=%{public}lu, reason=%{public}u",
        displayId, static_cast<uint32_t>(reason));
    RecordCall("WakeUpBegin", displayId, static_cast<int32_t>(reason));
    return true;
}

bool ScreenPowerAdapter::WakeUpEnd(uint64_t displayId)
{
    DISPLAY_HILOGI(COMP_SVC,
        "ScreenPowerAdapter::WakeUpEnd MOCK, displayId=%{public}lu",
        displayId);
    RecordCall("WakeUpEnd", displayId, 0);
    return true;
}

bool ScreenPowerAdapter::SuspendBegin(uint64_t displayId, Rosen::PowerStateChangeReason reason)
{
    DISPLAY_HILOGI(COMP_SVC,
        "ScreenPowerAdapter::SuspendBegin MOCK, displayId=%{public}lu, reason=%{public}u",
        displayId, static_cast<uint32_t>(reason));
    RecordCall("SuspendBegin", displayId, static_cast<int32_t>(reason));
    return true;
}

bool ScreenPowerAdapter::SuspendEnd(uint64_t displayId)
{
    DISPLAY_HILOGI(COMP_SVC,
        "ScreenPowerAdapter::SuspendEnd MOCK, displayId=%{public}lu",
        displayId);
    RecordCall("SuspendEnd", displayId, 0);
    return true;
}

// ===== Rosen per-displayId (REAL — already available) =====

bool ScreenPowerAdapter::SetScreenPowerById(uint64_t displayId,
    Rosen::ScreenPowerState powerState, Rosen::PowerStateChangeReason reason)
{
    DISPLAY_HILOGI(COMP_SVC,
        "ScreenPowerAdapter::SetScreenPowerById REAL, displayId=%{public}lu, powerState=%{public}u, reason=%{public}u",
        displayId, static_cast<uint32_t>(powerState), static_cast<uint32_t>(reason));
    RecordCall("SetScreenPowerById", displayId, static_cast<int32_t>(powerState));
    bool ret = Rosen::DisplayManagerLite::GetInstance().SetScreenPowerById(
        static_cast<Rosen::ScreenId>(displayId), powerState, reason);
    if (!ret) {
        DISPLAY_HILOGE(COMP_SVC,
            "SetScreenPowerById REAL failed, displayId=%{public}lu", displayId);
    }
    return ret;
}

// ===== BrightnessManager per-displayId (MOCK) =====

bool ScreenPowerAdapter::SetScreenOnBrightness(uint64_t displayId)
{
    DISPLAY_HILOGI(COMP_SVC,
        "ScreenPowerAdapter::SetScreenOnBrightness MOCK, displayId=%{public}lu",
        displayId);
    RecordCall("SetScreenOnBrightness", displayId, 0);
    // MOCK: In production mock, also call the existing global version for basic brightness
    // When real per-displayId API is available, replace with BrightnessManager::SetScreenOnBrightness(displayId)
    BrightnessManager::Get().SetScreenOnBrightness();
    return true;
}

// ===== Test verification helpers =====

void ScreenPowerAdapter::RecordCall(const std::string& methodName, uint64_t displayId, int32_t param)
{
    std::lock_guard lock(recordsMutex_);
    CallRecord record;
    record.methodName = methodName;
    record.displayId = displayId;
    record.reasonOrState = param;
    callRecords_.push_back(record);
}

std::vector<ScreenPowerAdapter::CallRecord> ScreenPowerAdapter::GetCallRecords() const
{
    std::lock_guard lock(recordsMutex_);
    return callRecords_;
}

void ScreenPowerAdapter::ClearCallRecords()
{
    std::lock_guard lock(recordsMutex_);
    callRecords_.clear();
}

bool ScreenPowerAdapter::HasCall(const std::string& methodName, uint64_t displayId) const
{
    std::lock_guard lock(recordsMutex_);
    for (const auto& record : callRecords_) {
        if (record.methodName == methodName && record.displayId == displayId) {
            return true;
        }
    }
    return false;
}

size_t ScreenPowerAdapter::GetCallCount(const std::string& methodName) const
{
    std::lock_guard lock(recordsMutex_);
    size_t count = 0;
    for (const auto& record : callRecords_) {
        if (record.methodName == methodName) {
            count++;
        }
    }
    return count;
}

} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // ENABLE_PER_SCREEN_POWER