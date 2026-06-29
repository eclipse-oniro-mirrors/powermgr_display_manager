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

#define FUZZ_PROJECT_NAME "updatescreenpowerstate_fuzzer"

#include "display_fuzzer.h"
#include "idisplay_power_mgr.h"

using namespace OHOS::DisplayPowerMgr;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    DisplayFuzzerTest g_serviceTest;
    g_serviceTest.TestDisplayServiceStub(
        static_cast<uint32_t>(IDisplayPowerMgrIpcCode::COMMAND_UPDATE_SCREEN_POWER_STATE), data, size);
    return 0;
}
