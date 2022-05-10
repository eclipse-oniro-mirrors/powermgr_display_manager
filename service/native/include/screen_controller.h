/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <atomic>
#include <map>
#include <memory>
#include <mutex>

#include "display_event_handler.h"
#include "display_power_info.h"
#include "gradual_animator.h"
#include "screen_action.h"

namespace OHOS {
namespace DisplayPowerMgr {
class ScreenController {
public:
    ScreenController(uint32_t displayId, const std::shared_ptr<DisplayEventHandler>& handler);
    virtual ~ScreenController() = default;

    class AnimateController  : public AnimateCallback {
    public:
        AnimateController (const std::shared_ptr<ScreenAction>& action);
        virtual ~AnimateController () = default;
        virtual void OnStart() override;
        virtual void OnChanged(uint32_t currentValue) override;
        virtual void OnEnd() override;
    private:
        const std::shared_ptr<ScreenAction>& action_;
    };

    class SharedController {
    public:
        SharedController(const uint32_t displayId);
        bool SetBrightness(uint32_t value, uint32_t gradualDuration = 0);
        uint32_t GetBrightness();
        void AllowAdjustBrightness(bool allow);
        std::shared_ptr<ScreenAction>& GetAction();
        bool UpdateBrightness(uint32_t value, uint32_t gradualDuration);

    private:
        std::mutex mutex_;
        std::atomic<bool> isAllow_ { true };
        std::shared_ptr<ScreenAction> action_ { nullptr };
        std::shared_ptr<GradualAnimator> animator_;
    };

    class DisplayStateController {
    public:
        DisplayStateController(const std::shared_ptr<SharedController>& sharedControl);
        DisplayState GetState()
        {
            return state_;
        }
        bool UpdateState(DisplayState state, uint32_t reason);
        bool UpdateStateConfig(DisplayState state, uint32_t value);
        bool IsScreenOn();
        void UpdateBeforeOffBrightness(uint32_t brightness);

    private:
        void BeforeScreenOff(DisplayState state);
        void AfterScreenOn(DisplayState state);
        void OnStateChanged(DisplayState state);
        uint32_t beforeOffBrightness_ { 0 };
        DisplayState state_;
        uint32_t stateChangeReason_ { 0 };
        std::mutex mutexState_;
        std::map<DisplayState, uint32_t> stateValues_;
        const std::shared_ptr<SharedController>& sharedControl_;
    };

    class OverrideController {
    public:
        OverrideController(const std::shared_ptr<SharedController>& sharedControl,
            const std::shared_ptr<DisplayStateController>& stateControl,
            const std::shared_ptr<DisplayEventHandler>& handler);
        bool OverrideBrightness(uint32_t value, uint32_t gradualDuration = SCREEN_BRIGHTNESS_UPDATE_DURATION);
        bool RestoreBrightness(uint32_t gradualDuration = SCREEN_BRIGHTNESS_UPDATE_DURATION);
        bool IsBrightnessOverride() const;
        uint32_t GetBeforeBrightness() const;
        bool BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration = SCREEN_BRIGHTNESS_UPDATE_DURATION);
        bool CancelBoostBrightness();
        bool IsBoostBrightness() const;
    private:
        void SaveBeforeBrightness(std::atomic<bool>& isOverride);
        bool BrightnessBeforeRestore(std::atomic<bool>& isOverride, uint32_t gradualDuration);
        uint32_t beforeBrightness_ { 0 };
        std::mutex mutexOverride_;
        std::atomic<bool> isBrightnessOverride_ { false };
        std::atomic<bool> isBoostBrightness_ { false };
        const std::shared_ptr<SharedController>& sharedControl_;
        const std::shared_ptr<DisplayStateController>& stateControl_;
        const std::shared_ptr<DisplayEventHandler>& handler_;
    };

    std::shared_ptr<SharedController>& SharedControl()
    {
        return sharedControl_;
    }
    std::unique_ptr<OverrideController>& OverrideControl()
    {
        return overrideControl_;
    }
    std::shared_ptr<DisplayStateController>& DisplayStateControl()
    {
        return stateControl_;
    }

private:
    static const uint32_t SCREEN_BRIGHTNESS_UPDATE_DURATION = 200;
    std::shared_ptr<SharedController> sharedControl_ { nullptr };
    std::unique_ptr<OverrideController> overrideControl_;
    std::shared_ptr<DisplayStateController> stateControl_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_SCREEN_CONTROLLER_H