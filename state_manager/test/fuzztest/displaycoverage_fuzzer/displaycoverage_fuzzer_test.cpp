/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#define FUZZ_PROJECT_NAME "displaycoverage_fuzzer"

#include "display_power_mgr_service.h"
#include "displaycoverage_fuzzer_test.h"

using namespace OHOS::PowerMgr;
using namespace OHOS::DisplayPowerMgr;

namespace {
const uint32_t DISPLAY_ID = 0;
const uint32_t BRIGHTNESS_MAX = 300;
const uint32_t BRIGHTNESS_MIN = 0;
const double DISCOUNT_MAX = 1.05;
const double DISCOUNT_MIN = 0.01;
const uint32_t brightness = 1;
} //namespace


extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::sptr<OHOS::DisplayPowerMgr::DisplayPowerMgrService> service =
        DelayedSpSingleton<DisplayPowerMgrService>::GetInstance();
    service->Init();
    DisplayState state = DisplayState::DISPLAY_OFF;
    uint32_t reason = 1;
    service->NotifyStateChangeCallback(DISPLAY_ID, state, reason);
    service->HandleBootBrightness();
    service->SetScreenOnBrightness();
    service->GetSafeBrightness(BRIGHTNESS_MAX);
    service->GetSafeBrightness(BRIGHTNESS_MIN);
    service->GetSafeDiscount(DISCOUNT_MAX, brightness);
    service->GetSafeDiscount(DISCOUNT_MIN, brightness);
    return 0;
}
