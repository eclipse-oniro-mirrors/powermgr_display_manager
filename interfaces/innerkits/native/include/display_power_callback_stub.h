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

#ifndef DISPLAYMGR_DISPLAY_POWER_CALLBACK_STUB_H
#define DISPLAYMGR_DISPLAY_POWER_CALLBACK_STUB_H

#include <iremote_stub.h>

#include "idisplay_power_mgr.h"

namespace OHOS {
namespace DisplayPowerMgr {
class DisplayPowerCallbackStub : public IRemoteStub<IDisplayPowerCallback> {
public:
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnDisplayStateChangedStub(MessageParcel& data, MessageParcel& reply);
};
} // namespace DisplayPowerMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_POWER_CALLBACK_STUB_H
