/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef MOCK_BRIGHTNESS_MANAGER_INTERFACE_H
#define MOCK_BRIGHTNESS_MANAGER_INTERFACE_H

#include "brightness_manager_ext.h"

namespace OHOS {
namespace DisplayPowerMgr {

void MockSetValue(double value);
void MockClearValues(void);
void MockInitBrightnessManagerExt(OHOS::DisplayPowerMgr::BrightnessManagerExt& ext);

} // namespace DisplayPowerMgr
} // namespace OHOS
#endif