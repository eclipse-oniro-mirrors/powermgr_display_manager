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

#ifndef DISPLAYMGR_DISPLAY_MGR_PROXY_H
#define DISPLAYMGR_DISPLAY_MGR_PROXY_H

#include <iremote_proxy.h>

#include "idisplay_power_mgr.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrProxy : public IRemoteProxy<IDisplayPowerMgr> {
public:
    explicit DisplayPowerMgrProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IDisplayPowerMgr>(impl) {}
    ~DisplayPowerMgrProxy() override = default;

    virtual bool SetDisplayState(uint32_t id, DisplayState state, uint32_t reason) override;
    virtual DisplayState GetDisplayState(uint32_t id) override;
    virtual std::vector<uint32_t> GetDisplayIds() override;
    virtual uint32_t GetMainDisplayId() override;

    virtual bool SetBrightness(uint32_t value, uint32_t displayId) override;
    virtual bool OverrideBrightness(uint32_t value, uint32_t displayId) override;
    virtual bool RestoreBrightness(uint32_t displayId) override;
    virtual uint32_t GetBrightness(uint32_t displayId) override;
    virtual bool AdjustBrightness(uint32_t id, int32_t value, uint32_t duration) override;
    virtual bool AutoAdjustBrightness(bool enable) override;
    virtual bool IsAutoAdjustBrightness() override;
    virtual bool SetStateConfig(uint32_t id, DisplayState state, int32_t value) override;
    virtual bool RegisterCallback(sptr<IDisplayPowerCallback> callback) override;

private:
    static inline BrokerDelegator<DisplayPowerMgrProxy> delegator_;
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_PROXY_H
