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
#include <memory>
#include <mutex>
#include <cstdint>

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

    class AnimateCallbackImpl : public AnimateCallback {
    public:
        explicit AnimateCallbackImpl(const std::shared_ptr<ScreenAction>& action,
            const std::shared_ptr<DisplayEventHandler>& handler);
        ~AnimateCallbackImpl() override = default;
        void OnStart() override;
        void OnChanged(uint32_t currentValue) override;
        void OnEnd() override;
        void DiscountBrightness(double discount) override;
    private:
        const std::shared_ptr<ScreenAction>& action_;
        const std::shared_ptr<DisplayEventHandler>& handler_;
        double discount_ {1.0};
    };

    DisplayState GetState();
    bool UpdateState(DisplayState state, uint32_t reason);
    bool IsScreenOn();

    bool SetBrightness(uint32_t value, uint32_t gradualDuration = 0);
    uint32_t GetBrightness();
    uint32_t GetDeviceBrightness();

    bool DiscountBrightness(double discount, uint32_t gradualDuration = 0);
    bool OverrideBrightness(uint32_t value, uint32_t gradualDuration = 0);
    bool RestoreBrightness(uint32_t gradualDuration = 0);
    bool IsBrightnessOverridden() const;

    bool BoostBrightness(uint32_t timeoutMs, uint32_t gradualDuration = 0);
    bool CancelBoostBrightness(uint32_t gradualDuration = 0);
    bool IsBrightnessBoosted() const;

    uint32_t GetScreenOnBrightness() const;

    void RegisterSettingBrightnessObserver();
    void UnregisterSettingBrightnessObserver();
    double GetDiscount() const;

    uint32_t GetAnimationUpdateTime() const;
private:
    void OnStateChanged(DisplayState state);

    bool CanSetBrightness();
    bool CanDiscountBrightness();
    bool CanOverrideBrightness();
    bool CanBoostBrightness();
    bool UpdateBrightness(uint32_t value, uint32_t gradualDuration = 0, bool updateSetting = false);
    void SetSettingBrightness(uint32_t value);
    uint32_t GetSettingBrightness(const std::string& key = SETTING_BRIGHTNESS_KEY) const;
    void BrightnessSettingUpdateFunc(const std::string& key);

    static const constexpr char* SETTING_BRIGHTNESS_KEY {"settings.display.screen_brightness_status"};
    DisplayState state_;
    std::mutex mutexState_;
    uint32_t stateChangeReason_ {0};
    double discount_ {1.0};

    std::atomic<bool> isBrightnessOverridden_ {false};
    std::atomic<bool> isBrightnessBoosted_ {false};
    uint32_t cachedSettingBrightness_ {102};
    uint32_t overriddenBrightness_ {102};
    std::shared_ptr<ScreenAction> action_ {nullptr};
    std::shared_ptr<AnimateCallback> animateCallback_ {nullptr};
    std::shared_ptr<GradualAnimator> animator_;
    const std::shared_ptr<DisplayEventHandler>& handler_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_SCREEN_CONTROLLER_H