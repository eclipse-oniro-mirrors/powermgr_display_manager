/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DISPLAYMGR_SCREEN_DISPLAY_STATE_CALLBACK_PROXY_H
#define DISPLAYMGR_SCREEN_DISPLAY_STATE_CALLBACK_PROXY_H

#include <cstdint>
#include <iremote_proxy.h>
#include "iremote_broker.h"
#include "iremote_object.h"
#include "refbase.h"
#include "iscreen_display_state_callback.h"

namespace OHOS {
namespace DisplayPowerMgr {
class ScreenDisplayStateCallbackProxy : public IRemoteProxy<IScreenDisplayStateCallback> {
public:
    explicit ScreenDisplayStateCallbackProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IScreenDisplayStateCallback>(impl) {}
    ~ScreenDisplayStateCallbackProxy() override = default;
    virtual void OnDisplayStateChanged(uint64_t displayId, uint32_t state, uint32_t reason) override;

private:
    static inline BrokerDelegator<ScreenDisplayStateCallbackProxy> delegator_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS

#endif // DISPLAYMGR_SCREEN_DISPLAY_STATE_CALLBACK_PROXY_H
