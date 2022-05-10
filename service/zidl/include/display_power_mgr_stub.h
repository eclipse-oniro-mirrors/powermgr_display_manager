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

#ifndef DISPLAYMGR_DISPLAY_MGR_STUB_H
#define DISPLAYMGR_DISPLAY_MGR_STUB_H

#include <iremote_stub.h>

#include "idisplay_power_mgr.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerMgrStub : public IRemoteStub<IDisplayPowerMgr> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t SetDisplayStateStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetDisplayStateStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetDisplayIdsStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetMainDisplayIdStub(MessageParcel& data, MessageParcel& reply);
    int32_t SetBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t OverrideBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t RestoreBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetDefaultBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetMaxBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t GetMinBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t AdjustBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t AutoAdjustBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t IsAutoAdjustBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t SetStateConfigStub(MessageParcel& data, MessageParcel& reply);
    int32_t RegisterCallbackStub(MessageParcel& data, MessageParcel& reply);
    int32_t BoostBrightnessStub(MessageParcel& data, MessageParcel& reply);
    int32_t CancelBoostBrightnessStub(MessageParcel& data, MessageParcel& reply);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_STUB_H
