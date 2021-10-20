/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <system_ability.h>
#include <system_ability_definition.h>
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

private:
    sptr<DisplayPowerMgrService> service_;
};
}
}
#endif