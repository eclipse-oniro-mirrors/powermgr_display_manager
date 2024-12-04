/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef DISPLAY_SYSTEM_ABILITY_H
#define DISPLAY_SYSTEM_ABILITY_H

#include <cstdint>
#include <iosfwd>
#include <string>
#include <system_ability.h>
#include "refbase.h"
#include "display_common_event_mgr.h"
#include "display_power_mgr_service.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplaySystemAbility : public SystemAbility {
    DECLARE_SYSTEM_ABILITY(DisplaySystemAbility);

public:
    DisplaySystemAbility(int32_t id, bool runOnCreate) : SystemAbility(id, runOnCreate) {}
    ~DisplaySystemAbility() override = default;

    void OnStart() override;
    void OnStop() override;
    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    sptr<DisplayPowerMgrService> service_;
    std::shared_ptr<DisplayCommonEventManager> commonEventManager_ {nullptr};
    bool isReady{false};
};
}
}
#endif
