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

#ifndef DISPLAYMGR_SCREEN_CONTROLLER_H
#define DISPLAYMGR_SCREEN_CONTROLLER_H

#include <map>
#include <mutex>

#include "display_power_info.h"
#include "gradual_animator.h"
#include "screen_action.h"

namespace OHOS {
namespace DisplayPowerMgr {
class ScreenController :
    public AnimateCallback,
    public std::enable_shared_from_this<ScreenController> {
public:
    ScreenController(uint64_t displayId, std::shared_ptr<ScreenAction> action);
    virtual ~ScreenController() = default;

    DisplayState GetState()
    {
        return state_;
    };
    bool UpdateState(DisplayState state, uint32_t reason);
    bool UpdateStateConfig(DisplayState state, uint32_t value);
    bool UpdateBrightness(uint32_t value, uint32_t gradualDuration = 0);
    bool IsScreenOn();
    uint32_t GetBrightness();
    virtual void OnStart() override;
    virtual void OnChanged(int32_t currentValue) override;
    virtual void OnEnd() override;
private:
    static const uint32_t SCREEN_BRIGHTNESS_UPDATE_DURATION = 200;
    void OnStateChanged(DisplayState state);
    void BeforeUpdateState(DisplayState state);
    void AfterUpdateState(DisplayState state);
    std::mutex mutex_;
    const uint64_t displayId_;
    DisplayState state_;
    std::map<DisplayState, uint32_t> stateValues_;

    uint32_t brightness_ {0};
    uint32_t beforeOffBrightness_ {0};
    uint32_t stateChangeReason_ {0};
    std::shared_ptr<ScreenAction> action_;
    std::shared_ptr<GradualAnimator> animator_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_SCREEN_CONTROLLER_H