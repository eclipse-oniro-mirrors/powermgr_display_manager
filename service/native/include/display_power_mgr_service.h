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

#ifndef DISPLAYMGR_DISPLAY_MGR_SERVICE_H
#define DISPLAYMGR_DISPLAY_MGR_SERVICE_H

#include <mutex>
#include <system_ability.h>
#include <system_ability_definition.h>

#include "display_power_mgr_stub.h"
#include "screen_controller.h"
#include "display_common.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrService : public DisplayPowerMgrStub {
public:
    DisplayPowerMgrService();
    ~DisplayPowerMgrService() = default;
    virtual bool SetDisplayState(uint32_t id, DisplayState state) override;
    virtual DisplayState GetDisplayState(uint32_t id) override;
    virtual bool SetBrightness(uint32_t id, int32_t value) override;
    virtual bool AdjustBrightness(uint32_t id, int32_t value, uint32_t duration) override;
    virtual bool SetStateConfig(uint32_t id, DisplayState state, int32_t value) override;
    virtual int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

    class DisplaySystemAbility : public SystemAbility {
    DECLARE_SYSTEM_ABILITY(DisplaySystemAbility);

    public:
        DisplaySystemAbility(int32_t id, bool runOnCreate) : SystemAbility(id, runOnCreate) {}
        ~DisplaySystemAbility() override = default;

        void OnStart() override
        {
            DISPLAY_HILOGI(MODULE_SERVICE, "Start service");
            service_ = new DisplayPowerMgrService();
            if (!Publish(service_)) {
                DISPLAY_HILOGE(MODULE_SERVICE, "Failed to publish service");
            }
        }

        void OnStop() override
        {
            DISPLAY_HILOGI(MODULE_SERVICE, "Stop service");
        }

    private:
        sptr<DisplayPowerMgrService> service_;
    };
    REGISTER_SYSTEM_ABILITY_BY_ID(DisplaySystemAbility, DISPLAY_MANAGER_SERVICE_ID, true);

private:
    std::map<uint32_t, std::shared_ptr<ScreenController>> controllerMap_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_SERVICE_H
