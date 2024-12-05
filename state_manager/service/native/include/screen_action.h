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

#ifndef DISPLAYMGR_SCREEN_ACTION_H
#define DISPLAYMGR_SCREEN_ACTION_H

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "dm_common.h"
#include "display_manager_lite.h"
#include "display_power_info.h"

namespace OHOS {
namespace DisplayPowerMgr {
class ScreenAction {
public:
    ScreenAction(uint32_t displayId);
    ~ScreenAction() = default;

    static uint32_t GetDefaultDisplayId();
    static std::vector<uint32_t> GetAllDisplayId();

    uint32_t GetDisplayId();
    DisplayState GetDisplayState();
    bool SetDisplayState(DisplayState state, const std::function<void(DisplayState)>& callback);
    bool SetDisplayPower(DisplayState state, uint32_t reason);
    uint32_t GetBrightness();
    bool SetBrightness(uint32_t value);
    void SetCoordinated(bool coordinated);
    bool EnableSkipSetDisplayState(uint32_t reason);

private:
    static constexpr uint32_t DEFAULT_DISPLAY_ID = 0;
    std::mutex mutexBrightness_;
    uint32_t brightness_ {102};
    uint32_t displayId_ {DEFAULT_DISPLAY_ID};
    bool coordinated_ {false};
    Rosen::PowerStateChangeReason ParseSpecialReason(uint32_t reason);
    Rosen::DisplayState ParseDisplayState(DisplayState state);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_SCREEN_ACTION_H
