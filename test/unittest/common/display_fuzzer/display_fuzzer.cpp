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

#include "display_fuzzer.h"

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <random>
#include <ctime>
#include "securec.h"
#include "display_power_mgr_client.h"

using namespace OHOS::DisplayPowerMgr;
using namespace std;

namespace {
auto& g_displayMgrClient = OHOS::DisplayPowerMgr::DisplayPowerMgrClient::GetInstance();
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 3;
constexpr int32_t DATANUM = 4;
}

static void SetDisplayState(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }
    g_displayMgrClient.SetDisplayState(static_cast<DisplayState>(type[0]));
}

static void GetDisplayState(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.GetDisplayState();
}

static void GetDisplayIds(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.GetDisplayIds();
}

static void GetMainDisplayId(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.GetMainDisplayId();
}

static void SetBrightness(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.SetBrightness(type[0]);
}

static void AdjustBrightness(const uint8_t* data)
{
    int32_t type[1];
    int32_t duration[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }
    if ((memcpy_s(duration, sizeof(duration), (data+DATANUM), idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.AdjustBrightness(type[0], duration[0]);
}

static void SetStateConfig(const uint8_t* data)
{
    int32_t state[1];
    int32_t value[1];
    int32_t idSize = 4;
    if ((memcpy_s(state, sizeof(state), data, idSize)) != EOK) {
        return;
    }
    if ((memcpy_s(value, sizeof(value), (data+DATANUM), idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.SetStateConfig(static_cast<DisplayState>(state[0]), value[0]);
}

static void AutoAdjustBrightness(const uint8_t* data)
{
    int32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.AutoAdjustBrightness(type[0]);
}

static void RegisterCallback(const uint8_t* data)
{
    static OHOS::sptr<OHOS::DisplayPowerMgr::IDisplayPowerCallback> callback;
    int32_t idSize = 4;
    if ((memcpy_s(callback, sizeof(callback), data, idSize)) != EOK) {
        return;
    }

    g_displayMgrClient.RegisterCallback(callback);
}

static void OverrideBrightness(const uint8_t* data)
{
    uint32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }
    g_displayMgrClient.OverrideBrightness(type[0]);
}

static void RestoreBrightness(const uint8_t* data)
{
    uint32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }
    g_displayMgrClient.RestoreBrightness(type[0]);
}

static void BoostBrightness(const uint8_t* data)
{
    uint32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }
    g_displayMgrClient.BoostBrightness(type[0]);
}

static void CancelBoostBrightness(const uint8_t* data)
{
    uint32_t type[1];
    int32_t idSize = 4;
    if ((memcpy_s(type, sizeof(type), data, idSize)) != EOK) {
        return;
    }
    g_displayMgrClient.CancelBoostBrightness(type[0]);
}

namespace OHOS {
bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
{
    int32_t idSize = 8;
    int32_t cond[1];
    if (static_cast<int32_t>(size) > idSize) {
        if ((memcpy_s(cond, sizeof(cond), data, idSize)) != EOK) {
            return false;
        }
        std::random_device rd;
        std::default_random_engine engine(rd());
        std::uniform_int_distribution<int32_t> randomNum(MIN, MAX);
        int32_t number = randomNum(engine);

        switch (static_cast<ApiNumber>(number)) {
            case ApiNumber::NUM_ZERO:
                SetDisplayState(data);
                break;
            case ApiNumber::NUM_ONE:
                GetDisplayState(data);
                break;
            case ApiNumber::NUM_TWO:
                GetDisplayIds(data);
                break;
            case ApiNumber::NUM_THREE:
                GetMainDisplayId(data);
                break;
            case ApiNumber::NUM_FOUR:
                SetBrightness(data);
                break;
            case ApiNumber::NUM_FIVE:
                AdjustBrightness(data);
                break;
            case ApiNumber::NUM_SIX:
                SetStateConfig(data);
                break;
            case ApiNumber::NUM_SEVEN:
                AutoAdjustBrightness(data);
                break;
            case ApiNumber::NUM_EIGHT:
                RegisterCallback(data);
                break;
            case ApiNumber::NUM_NINE:
                OverrideBrightness(data);
                break;
            case ApiNumber::NUM_TEN:
                RestoreBrightness(data);
                break;
            case ApiNumber::NUM_ELEVEN:
                BoostBrightness(data);
                break;
            case ApiNumber::NUM_TWELVE:
                CancelBoostBrightness(data);
                break;
            case ApiNumber::NUM_THIRTEEN:
                g_displayMgrClient.GetDefaultBrightness();
                break;
            case ApiNumber::NUM_FOURTEEN:
                g_displayMgrClient.GetMaxBrightness();
                break;
            case ApiNumber::NUM_FIFTEEN:
                g_displayMgrClient.GetMinBrightness();
                break;
            default:
                break;
        }
    }
    return true;
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
