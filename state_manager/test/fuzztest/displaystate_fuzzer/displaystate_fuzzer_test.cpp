/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

/* This files contains faultlog fuzzer test modules. */

#define FUZZ_PROJECT_NAME "displaystate_fuzzer"

#include "display_fuzzer.h"
#include "display_power_mgr_ipc_interface_code.h"

using namespace OHOS::PowerMgr;

namespace {
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    DisplayFuzzerTest g_serviceTest;
    g_serviceTest.TestDisplayServiceStub(
        static_cast<uint32_t>(DisplayPowerMgrInterfaceCode::SET_DISPLAY_STATE), data, size);
    g_serviceTest.TestDisplayServiceStub(
        static_cast<uint32_t>(DisplayPowerMgrInterfaceCode::GET_DISPLAY_STATE), data, size);
    return 0;
}
