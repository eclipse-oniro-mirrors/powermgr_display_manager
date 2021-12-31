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

#ifndef DISPLAYMGR_GRADUAL_ANIMATOR_H
#define DISPLAYMGR_GRADUAL_ANIMATOR_H

#include <memory>
#include <string>
#include <event_handler.h>
#include <event_runner.h>

namespace OHOS {
namespace DisplayPowerMgr {
class AnimateCallback {
public:
    virtual void OnStart() = 0;
    virtual void OnChanged(int32_t currentValue) = 0;
    virtual void OnEnd() = 0;
};

class GradualAnimator : public std::enable_shared_from_this<GradualAnimator> {
public:
    GradualAnimator(const std::string& name, std::shared_ptr<AnimateCallback> callback);
    ~GradualAnimator() = default;
    void StartAnimation(int32_t from, int32_t to, uint32_t duration);
    void StopAnimation();
    bool IsAnimating();
private:
    static const uint32_t DEFAULT_UPDATE_TIME = 200;
    static const uint32_t EVENT_STEP = 1;
    class AnimatorHandler : public AppExecFwk::EventHandler {
    public:
        AnimatorHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner,
            std::shared_ptr<GradualAnimator> owner);
        ~AnimatorHandler() = default;
        void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
    private:
        std::weak_ptr<GradualAnimator> owner_;
    };
    void NextStep();
    std::string name_;
    std::weak_ptr<AnimateCallback> callback_;
    std::shared_ptr<AppExecFwk::EventRunner> eventRunner_;
    std::shared_ptr<AnimatorHandler> handler_;
    bool animating_ = false;
    int32_t from_;
    int32_t to_;
    uint32_t duration_;
    uint32_t updateTime_;
    uint32_t steps_;
    int32_t stride_;
    int32_t current_;
    uint32_t currentStep_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_GRADUAL_ANIMATOR_H