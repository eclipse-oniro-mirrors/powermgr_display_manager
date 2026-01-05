/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef DISPLAYMGR_SCREEN_POWER_OFF_STRATEGY_H
#define DISPLAYMGR_SCREEN_POWER_OFF_STRATEGY_H

#include "refbase.h"
#include "display_power_info.h"
#include "power_state_machine_info.h"
#include <singleton.h>

namespace OHOS {
namespace DisplayPowerMgr {
class ScreenPowerOffStrategy : public DelayedRefSingleton<ScreenPowerOffStrategy> {
    DECLARE_DELAYED_REF_SINGLETON(ScreenPowerOffStrategy);

public:
    void SetStrategy(PowerOffStrategy strategy, PowerMgr::StateChangeReason reason);
    bool IsSpecificStrategy();
    PowerMgr::StateChangeReason GetReason();

private:
    PowerOffStrategy strategy_ = PowerOffStrategy::STRATEGY_DEFAULT;
    PowerMgr::StateChangeReason reason_ = PowerMgr::StateChangeReason::STATE_CHANGE_REASON_UNKNOWN;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_SCREEN_POWER_OFF_STRATEGY_H