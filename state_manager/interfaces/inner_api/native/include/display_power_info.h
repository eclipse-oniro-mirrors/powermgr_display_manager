/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef DISPLAYMGR_DISPLAY_POWER_INFO_H
#define DISPLAYMGR_DISPLAY_POWER_INFO_H

#include <string>

namespace OHOS {
namespace DisplayPowerMgr {
/**
 * Display State
 */
enum class DisplayState : uint32_t {
    DISPLAY_OFF = 0,
    DISPLAY_DIM = 1,
    DISPLAY_ON = 2,
    DISPLAY_SUSPEND = 3,
    DISPLAY_DELAY_OFF = 4,
    DISPLAY_DOZE,
    DISPLAY_DOZE_SUSPEND,
    DISPLAY_UNKNOWN
};

/**
 * Screen Power Off Strategy
 */
enum class PowerOffStrategy : uint32_t {
    STRATEGY_ALL = 0, // default strategy
    STRATEGY_SPECIFIC = 1,
    STRATEGY_UNKNOWN
};

/**
 * Multi Screen State Change Reason
 */
enum class MultiScreenStateChangeReason : uint32_t {
    STATE_CHANGE_REASON_DEFAULT = 0,
    STATE_CHANGE_REASON_UNKNOWN = 1000,
};

#ifdef DISPLAY_MANAGER_ENABLE_MULTI_SCREEN_STATE
inline std::string GetReasonString(uint32_t reason)
{
    auto type = static_cast<MultiScreenStateChangeReason>(reason);
    switch (type) {
        case MultiScreenStateChangeReason::STATE_CHANGE_REASON_DEFAULT:
            return std::string("DEFAULT");
        default:
            return std::string("UNKNOWN");
    }
}
#endif
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_POWER_INFO_H
