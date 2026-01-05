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
#include "miscellaneous_display_power_strategy.h"

namespace OHOS {
namespace DisplayPowerMgr {

MiscellaneousDisplayPowerStrategy::MiscellaneousDisplayPowerStrategy()= default;
MiscellaneousDisplayPowerStrategy::~MiscellaneousDisplayPowerStrategy() = default;

void MiscellaneousDisplayPowerStrategy::SetStrategy(PowerOffStrategy strategy,
    PowerMgr::StateChangeReason reason)
{
    strategy_ = strategy;
    reason_ = reason;
}

bool MiscellaneousDisplayPowerStrategy::IsSpecificStrategy()
{
    return strategy_ == PowerOffStrategy::STRATEGY_SPECIFIC;
}

PowerMgr::MiscellaneousDisplayPowerStrategy ScreenPowerOffStrategy::GetReason()
{
    return reason_;
}
} // namespace DisplayPowerMgr
} // namespace OHOS
