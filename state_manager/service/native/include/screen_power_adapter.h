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

#ifndef DISPLAYMGR_SCREEN_POWER_ADAPTER_H
#define DISPLAYMGR_SCREEN_POWER_ADAPTER_H

#ifdef ENABLE_PER_SCREEN_POWER

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include "display_manager_lite.h"
#include "display_log.h"
#include "brightness_manager.h"

namespace OHOS {
namespace DisplayPowerMgr {

/**
 * ScreenPowerAdapter encapsulates external per-displayId dependencies
 * for the SetDisplayStateById complete screen on/off flow.
 *
 * Current status:
 * - WakeUpBegin/WakeUpEnd/SuspendBegin/SuspendEnd: MOCK implementation
 *   (window_manager per-displayId APIs not yet available)
 * - SetScreenPowerById: REAL (Rosen already supports per-displayId)
 * - SetScreenOnBrightness: MOCK implementation
 *   (BrightnessManager per-displayId API not yet available)
 *
 * When real per-displayId APIs become available, replace mock methods
 * with real delegation to Rosen::DisplayManagerLite and BrightnessManager.
 */
class ScreenPowerAdapter {
public:
    static ScreenPowerAdapter& GetInstance();

    // ===== window_manager per-displayId APIs (MOCK for now) =====

    /**
     * Notify window_manager that a wakeup sequence is starting for a specific display.
     * MOCK: logs the call and returns true.
     * Real: will call Rosen::DisplayManagerLite::WakeUpBegin(displayId, reason)
     */
    bool WakeUpBegin(uint64_t displayId, Rosen::PowerStateChangeReason reason);

    /**
     * Notify window_manager that a wakeup sequence has completed for a specific display.
     * MOCK: logs the call and returns true.
     * Real: will call Rosen::DisplayManagerLite::WakeUpEnd(displayId)
     */
    bool WakeUpEnd(uint64_t displayId);

    /**
     * Notify window_manager that a suspend sequence is starting for a specific display.
     * MOCK: logs the call and returns true.
     * Real: will call Rosen::DisplayManagerLite::SuspendBegin(displayId, reason)
     */
    bool SuspendBegin(uint64_t displayId, Rosen::PowerStateChangeReason reason);

    /**
     * Notify window_manager that a suspend sequence has completed for a specific display.
     * MOCK: logs the call and returns true.
     * Real: will call Rosen::DisplayManagerLite::SuspendEnd(displayId)
     */
    bool SuspendEnd(uint64_t displayId);

    // ===== Rosen per-displayId (REAL — already available) =====

    /**
     * Set screen power state for a specific display.
     * REAL: delegates to Rosen::DisplayManagerLite::SetScreenPowerById.
     */
    bool SetScreenPowerById(uint64_t displayId,
        Rosen::ScreenPowerState powerState, Rosen::PowerStateChangeReason reason);

    // ===== BrightnessManager per-displayId (MOCK for now) =====

    /**
     * Restore screen brightness for a specific display after screen-on.
     * MOCK: logs the call and returns true.
     * Real: will call BrightnessManager::SetScreenOnBrightness(displayId)
     */
    bool SetScreenOnBrightness(uint64_t displayId);

    // ===== Test verification helpers =====

    struct CallRecord {
        std::string methodName;
        uint64_t displayId {0};
        int32_t reasonOrState {0};
    };

    std::vector<CallRecord> GetCallRecords() const;
    void ClearCallRecords();
    bool HasCall(const std::string& methodName, uint64_t displayId) const;
    size_t GetCallCount(const std::string& methodName) const;

private:
    ScreenPowerAdapter() = default;
    ~ScreenPowerAdapter() = default;

    void RecordCall(const std::string& methodName, uint64_t displayId, int32_t param);

    mutable std::mutex recordsMutex_;
    std::vector<CallRecord> callRecords_;
};

} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // ENABLE_PER_SCREEN_POWER
#endif // DISPLAYMGR_SCREEN_POWER_ADAPTER_H