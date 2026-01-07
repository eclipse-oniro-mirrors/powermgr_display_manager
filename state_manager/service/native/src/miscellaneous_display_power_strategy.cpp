/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "miscellaneous_display_power_strategy.h"
#include "display_log.h"

namespace OHOS {
namespace DisplayPowerMgr {

MiscellaneousDisplayPowerStrategy::MiscellaneousDisplayPowerStrategy()= default;
MiscellaneousDisplayPowerStrategy::~MiscellaneousDisplayPowerStrategy() = default;

void MiscellaneousDisplayPowerStrategy::SetStrategy(PowerOffStrategy strategy,
    PowerMgr::StateChangeReason reason)
{
    std::lock_guard<std::mutex> lock(strategyMutex_);
    strategy_ = strategy;
    reason_ = reason;
}

uint32_t MiscellaneousDisplayPowerStrategy::GetSpecificStrategyReason(DisplayState state, uint32_t originalReason)
{
    std::lock_guard<std::mutex> lock(strategyMutex_);
    if (strategy_ == PowerOffStrategy::STRATEGY_SPECIFIC && state != DisplayState::DISPLAY_ON) {
        DISPLAY_HILOGI(FEAT_STATE, "enable specific screen power strategy");
        return static_cast<uint32_t>(reason_);
    }
    return originalReason;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
